#include <fstream>
#include <iterator>
#include <vector>

#include "graphite-fuzzer.hpp"

int main(int argc, char * * argv)
{
  if (argc < 2)
    return 1;

  for (auto arg = 1; arg != argc; ++arg)
  {
    std::ifstream file(argv[arg]);
    std::string test_data((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());

    return LLVMFuzzerTestOneInput(reinterpret_cast<const uint8_t *>(test_data.data()), test_data.size());
  }
}
