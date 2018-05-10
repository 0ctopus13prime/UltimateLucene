#include <assert.h>
#include <iostream>
#include <Util/Bytes.h>

using namespace lucene::core::util;

// TODO Change this naive tests to Google test

int main() {
  std::cout << "Start basic BytesRefTests" << std::endl;

  {
    std::cout << "  - Basic test" << std::endl;
    // Basic test
    std::string name = "doochi";
    BytesRef bytes_ref(name);

    assert(bytes_ref.offset == 0);
    assert(bytes_ref.length == 6);

    std::string got = bytes_ref.UTF8ToString();
    assert(got == "doochi");
  }

  {
    std::cout << "  - Shallow copy, Deep copy" << std::endl;
    // Shallow copy, Deep copy
    std::string str = "doochi";
    BytesRef bytes_ref1(str);
    BytesRef shared_bytes_ref2;
    bytes_ref1.ShallowCopyTo(shared_bytes_ref2); // Shallow copy

    // Shallow case, Change single character at 0 index
    bytes_ref1.bytes.get()[0] = 'x';
    assert(shared_bytes_ref2.bytes.get()[0] == 'x');

    // Deep copy case, Chage single character at 0 index
    BytesRef bytes_ref2 = bytes_ref1;
    bytes_ref1.bytes.get()[0] = 'd';
    assert(bytes_ref2.bytes.get()[0] != bytes_ref1.bytes.get()[0]);
    assert(bytes_ref2.bytes.get()[0] == 'x');

    // operator =, assign
    BytesRef bytes_ref3 = bytes_ref2; // Deep copy
    assert(bytes_ref3 == bytes_ref2);

    BytesRef shared_bytes_ref3;
    bytes_ref3.ShallowCopyTo(shared_bytes_ref3);

    assert(shared_bytes_ref3 == bytes_ref2);
  }

  {
    std::cout << "  - Compare tests" << std::endl;
    // Compare tests
    std::string str1 = "doochi stupid";
    std::string str2 = str1 + " suffix";
    std::string str3 = "ugly doochi";

    // 1. ==
    {
      std::cout << "    - == test" << std::endl;
      BytesRef bytes_ref1(str1);
      BytesRef shared_bytes_ref1;
      bytes_ref1.ShallowCopyTo(shared_bytes_ref1);
      BytesRef bytes_ref2(str1);
      BytesRef shared_bytes_ref2(bytes_ref2);

      assert(bytes_ref1 == bytes_ref2);
      assert(shared_bytes_ref1 == bytes_ref2);
      assert(bytes_ref1 == shared_bytes_ref2);
      assert(shared_bytes_ref1 == shared_bytes_ref2);
    }

    // 2. !=
    {
      std::cout << "    - != test" << std::endl;
      BytesRef bytes_ref1(str1);
      BytesRef shared_bytes_ref1;
      bytes_ref1.ShallowCopyTo(shared_bytes_ref1);

      BytesRef bytes_ref2(str2);
      BytesRef shared_bytes_ref2;
      bytes_ref2.ShallowCopyTo(shared_bytes_ref2);

      assert(bytes_ref1 != bytes_ref2);
      assert(bytes_ref1 != shared_bytes_ref2);
      assert(shared_bytes_ref1 != bytes_ref2);
      assert(shared_bytes_ref1 != shared_bytes_ref2);
    }

    // 3. <
    {
      std::cout << "    - < test" << std::endl;
      BytesRef bytes_ref1(str1);
      BytesRef shared_bytes_ref1;
      bytes_ref1.ShallowCopyTo(shared_bytes_ref1);

      BytesRef bytes_ref2(str2);
      BytesRef shared_bytes_ref2;
      bytes_ref2.ShallowCopyTo(shared_bytes_ref2);

      assert(bytes_ref1 < bytes_ref2);
      assert(bytes_ref1 < shared_bytes_ref2);
      assert(shared_bytes_ref1 < bytes_ref2);
      assert(shared_bytes_ref1 < shared_bytes_ref2);

      BytesRef bytes_ref3(str3);
      BytesRef shared_bytes_ref3(bytes_ref3);
      assert(bytes_ref1 < bytes_ref3);
      assert(bytes_ref1 < shared_bytes_ref3);
      assert(shared_bytes_ref1 < bytes_ref3);
      assert(shared_bytes_ref1 < shared_bytes_ref3);
    }

    // 4. <=
    {
      std::cout << "    - <= test" << std::endl;
      BytesRef bytes_ref1(str1);
      BytesRef shared_bytes_ref1;
      bytes_ref1.ShallowCopyTo(shared_bytes_ref1);

      BytesRef bytes_ref2(str2);
      BytesRef shared_bytes_ref2;
      bytes_ref2.ShallowCopyTo(shared_bytes_ref2);

      assert(bytes_ref1 <= bytes_ref2);
      assert(bytes_ref1 <= shared_bytes_ref2);
      assert(shared_bytes_ref1 <= bytes_ref2);
      assert(shared_bytes_ref1 <= shared_bytes_ref2);

      BytesRef bytes_ref3(str3);
      BytesRef shared_bytes_ref3(bytes_ref3);
      assert(bytes_ref1 <= bytes_ref3);
      assert(bytes_ref1 <= shared_bytes_ref3);
      assert(shared_bytes_ref1 <= bytes_ref3);
      assert(shared_bytes_ref1 <= shared_bytes_ref3);
    }

    // 5. >
    {
      std::cout << "    - > test" << std::endl;
      BytesRef bytes_ref1(str1);
      BytesRef shared_bytes_ref1;
      bytes_ref1.ShallowCopyTo(shared_bytes_ref1);

      BytesRef bytes_ref2(str2);
      BytesRef shared_bytes_ref2;
      bytes_ref2.ShallowCopyTo(shared_bytes_ref2);

      assert(bytes_ref2 > bytes_ref1);
      assert(bytes_ref2 > shared_bytes_ref1);
      assert(shared_bytes_ref2 > bytes_ref1);
      assert(shared_bytes_ref2 > shared_bytes_ref1);

      BytesRef bytes_ref3(str3);
      BytesRef shared_bytes_ref3(bytes_ref3);
      assert(bytes_ref3 > bytes_ref1);
      assert(bytes_ref3 > shared_bytes_ref1);
      assert(shared_bytes_ref3 > bytes_ref1);
      assert(shared_bytes_ref3 > shared_bytes_ref1);
    }

    // 6. >=
    {
      std::cout << "    - >= test" << std::endl;
      BytesRef bytes_ref1(str1);
      BytesRef shared_bytes_ref1;
      bytes_ref1.ShallowCopyTo(shared_bytes_ref1);

      BytesRef bytes_ref2(str2);
      BytesRef shared_bytes_ref2;
      bytes_ref2.ShallowCopyTo(shared_bytes_ref2);

      assert(bytes_ref2 >= bytes_ref1);
      assert(bytes_ref2 >= shared_bytes_ref1);
      assert(shared_bytes_ref2 >= bytes_ref1);
      assert(shared_bytes_ref2 >= shared_bytes_ref1);

      BytesRef bytes_ref3(str3);
      BytesRef shared_bytes_ref3(bytes_ref3);
      assert(bytes_ref3 >= bytes_ref1);
      assert(bytes_ref3 >= shared_bytes_ref1);
      assert(shared_bytes_ref3 >= bytes_ref1);
      assert(shared_bytes_ref3 >= shared_bytes_ref1);
    }
  }

  {
    std::cout << "  - Validation test" << std::endl;
    // Validation test
    BytesRef bytes_ref;
    bytes_ref.offset = 1; // nullptr && offset > length
    try {
      bytes_ref.IsValid();
      assert(false); // Fail here
    } catch(...) {
      // Ignore, Exception must be caught in here.
    }
  }

  std::cout << "End basic BytesRefTests" << std::endl;
}
