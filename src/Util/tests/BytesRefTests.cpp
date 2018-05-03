#include <iostream>
#include <Util/Bytes.h>

using namespace lucene::core::util;

int main() {
  {
    std::string name = "doochi"; 
    BytesRef bytes_ref(name);

    std::cout << "'doochi' Offset -> " << bytes_ref.offset << ", Length -> " << bytes_ref.length << std::endl;

    std::string got = bytes_ref.UTF8ToString();
    std::cout << "Got -> " << got << std::endl;
  }
}
