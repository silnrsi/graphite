#include <fstream>
#include <iterator>
#include <vector>
#include <iostream>

#include "graphite-fuzzer.hpp"

int main(int argc, char * * argv)
{
  if (argc < 2)
  {
    std::cerr << argv[0] << " FUZZ-FILE [FUZZ-FILE ...]" << std::endl
              << argv[0] << ": need at least one file to test" << std::endl;
    return 1;
  }

  for (auto arg = 1; arg != argc; ++arg)
  {
    std::ifstream file(argv[arg]);
    if (!file)
    {
        std::cerr << argv[0] << ": " << argv[arg] << ": No such file or directory." << std::endl;
        return 1;
    }
    std::string test_data((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());

    std::cout << "Test case " << arg << ": " << argv[arg] << std::endl;
    auto ret = LLVMFuzzerTestOneInput(reinterpret_cast<const uint8_t *>(test_data.data()), test_data.size());
    if (ret != 0)
    {
      std::cerr << argv[0] << ": unsuccesful return code (" << ret << "): " << argv[arg] << std::endl;
      return ret;
    }
  }
}
