#include <cstring>
#include <cstdint>
#include <memory>

#include <graphite2/Font.h>
#include <graphite2/Segment.h>

#include "graphite-fuzzer.hpp"
#include "inc/TtfUtil.h"

namespace TtfUtil = graphite2::TtfUtil;

class memory_face : public gr_face_ops
{
  const uint8_t * _data;
  size_t          _len;
  const void *    _table_dir;
  const void *    _header_tbl;

public:
  memory_face(const uint8_t *data, size_t size)
  : gr_face_ops({sizeof(gr_face_ops), memory_face::get_table_fn, nullptr}),
    _data(data),
    _len(size),
    _table_dir(nullptr),
    _header_tbl(nullptr)
  {
    size_t tbl_offset, tbl_len;

    // Get the header
    if (!TtfUtil::GetHeaderInfo(tbl_offset, tbl_len)
        || tbl_len > size
        || tbl_offset > size - tbl_len) return;
    _header_tbl = data + tbl_offset;
    if (!TtfUtil::CheckHeader(_header_tbl)) return;

    // Get the table directory
    if (!TtfUtil::GetTableDirInfo(_header_tbl, tbl_offset, tbl_len)
        || tbl_len > size
        || tbl_offset > size - tbl_len) return;
    _table_dir = data + tbl_offset;
    return;
  }

  operator bool () const noexcept {
    return _table_dir && _header_tbl;
  }

private:
  static const void * get_table_fn(const void* app_fh, unsigned int name, size_t *len)
  {
    if (app_fh == nullptr)
      return nullptr;

    const auto & mf = *static_cast<const memory_face *>(app_fh);
    if (!mf)
      return nullptr;

    size_t tbl_offset, tbl_len;
    if (!TtfUtil::GetTableInfo(name, mf._header_tbl, mf._table_dir, tbl_offset, tbl_len))
        return nullptr;

    if (tbl_len > mf._len
        || tbl_offset > mf._len - tbl_len)
        return nullptr;

    if (len) *len = tbl_len;
    return mf._data + tbl_offset;
  }
};

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
  memory_face test_face(data, size);
  auto face = gr_make_face_with_ops(&test_face, &test_face, gr_face_preloadAll);
  auto font = gr_make_font(12, face);

  gr_font_destroy(font);
  gr_face_destroy(face);

  return 0;
}
