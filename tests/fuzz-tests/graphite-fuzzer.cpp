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

struct segment_parameters
{
  uint32_t  script_tag;
  uint8_t   encoding;
  int       direction;
  uint8_t   test_text[128];
};

struct slot_attr_parameters
{
  uint8_t     index;
  uint8_t     subindex;
};

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
  memory_face test_face(data, size);
  auto face = gr_make_face_with_ops(&test_face, &test_face, gr_face_preloadAll);
  auto font = gr_make_font(12, face);

  if (size > sizeof(segment_parameters))
  {
    segment_parameters params;
    std::memcpy(&params, data + size - sizeof(segment_parameters), sizeof(segment_parameters));
    const size_t n_chars = gr_count_unicode_characters(gr_encform(params.encoding),
                              params.test_text,
                              params.test_text + sizeof params.test_text,
                              nullptr);
    auto seg = gr_make_seg(font, face, params.script_tag, nullptr,
                  gr_encform(params.encoding),
                  params.test_text, n_chars,
                  params.direction);
    if (seg)
    {
      const auto n_cinfos = gr_seg_n_cinfo(seg);
      for (auto i = 0u; i < n_cinfos; ++i)
      {
        const auto ci = gr_seg_cinfo(seg, i);
        gr_cinfo_unicode_char(ci);
        gr_cinfo_base(ci);
        gr_cinfo_break_weight(ci);
        gr_cinfo_after(ci);
        gr_cinfo_before(ci);
      }

      const auto n_slots = gr_seg_n_slots(seg);
      for (auto s = gr_seg_first_slot(seg); s; s = gr_slot_next_in_segment(s))
      {
         gr_slot_attached_to(s);
         gr_slot_first_attachment(s);
         gr_slot_next_sibling_attachment(s);
         gr_slot_before(s);
         gr_slot_after(s);
         gr_slot_index(s);
         gr_slot_gid(s);
         gr_slot_origin_X(s);
         gr_slot_origin_Y(s);
         gr_slot_advance_X(s, face, font);
         gr_slot_advance_Y(s, face, font);
         gr_slot_can_insert_before(s);
         gr_slot_original(s);
      }

      if (size > sizeof(slot_attr_parameters)*n_slots)
      {
        std::unique_ptr<slot_attr_parameters[]> params(new slot_attr_parameters[n_slots]);
        std::memcpy(params.get(), data + size - sizeof(slot_attr_parameters)*n_slots, sizeof(slot_attr_parameters)*n_slots);
        auto n_params = n_slots;
        for (auto s = gr_seg_first_slot(seg); s && n_params; s = gr_slot_next_in_segment(s))
        {
          auto && p = params[--n_params];
          gr_slot_attr(s, seg, gr_attrCode(p.index), p.subindex);
        }
      }
    }
    gr_seg_destroy(seg);
  }

  gr_font_destroy(font);
  gr_face_destroy(face);

  return 0;
}
