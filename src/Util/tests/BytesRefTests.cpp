#include <assert.h>
#include <iostream>
#include <Util/Bytes.h>

using namespace lucene::core::util;

// TODO Change this naive tests to Google test

int main() {
  std::cout << "Start basic BytesRefTests" << std::endl;

  {
    // Basic test
    std::string name = "doochi";
    BytesRef bytes_ref(name);

    assert(bytes_ref.offset == 0);
    assert(bytes_ref.length == 6);

    std::string got = bytes_ref.UTF8ToString();
    assert(got == "doochi");
  }

  {
    // Shallow copy, Deep copy
    std::string str = "doochi";
    BytesRef bytes_ref1(str);
    BytesRef shared_bytes_ref2(bytes_ref1); // Shallow copy

    // Shallow case, Change single character at 0 index
    bytes_ref1.bytes.get()[0] = 'x';
    assert(bytes_ref1.bytes.get()[0] == 'x');

    // Deep copy case, Chage single character at 0 index
    BytesRef bytes_ref2;
    bytes_ref2.DeepCopyOf(bytes_ref1);
    bytes_ref1.bytes.get()[0] = 'd';
    assert(bytes_ref2.bytes.get()[0] != bytes_ref1.bytes.get()[0]);
    assert(bytes_ref2.bytes.get()[0] == 'x');

    // operator =, assign
    BytesRef bytes_ref3(bytes_ref1);
    bytes_ref3 = bytes_ref2;
    assert(bytes_ref3 == bytes_ref2);

    BytesRef shared_bytes_ref3(bytes_ref3);
    shared_bytes_ref3 = bytes_ref2;
    assert(shared_bytes_ref3 == bytes_ref2);
  }

  {
    // Compare tests
    std::string str1 = "doochi stupid";
    std::string str2 = str1 + " suffix";
    std::string str3 = "ugly doochi";

    // 1. ==
    {
      BytesRef bytes_ref1(str1);
      BytesRef shared_bytes_ref1(bytes_ref1);
      BytesRef bytes_ref2(str1);
      BytesRef shared_bytes_ref2(bytes_ref2);

      assert(bytes_ref1 == bytes_ref2);
      assert(shared_bytes_ref1 == bytes_ref2);
      assert(bytes_ref1 == shared_bytes_ref2);
      assert(shared_bytes_ref1 == shared_bytes_ref2);
    }

    // 2. !=
    {
      BytesRef bytes_ref1(str1);
      BytesRef shared_bytes_ref1(bytes_ref1);
      BytesRef bytes_ref2(str2);
      BytesRef shared_bytes_ref2(bytes_ref2);

      assert(bytes_ref1 != bytes_ref2);
      assert(bytes_ref1 != shared_bytes_ref2);
      assert(shared_bytes_ref1 != bytes_ref2);
      assert(shared_bytes_ref1 != shared_bytes_ref2);
    }

    // 3. <
    {
      BytesRef bytes_ref1(str1);
      BytesRef shared_bytes_ref1(bytes_ref1);
      BytesRef bytes_ref2(str2);
      BytesRef shared_bytes_ref2(bytes_ref2);

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
      BytesRef bytes_ref1(str1);
      BytesRef shared_bytes_ref1(bytes_ref1);
      BytesRef bytes_ref2(str1);
      BytesRef shared_bytes_ref2(bytes_ref2);

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
      BytesRef bytes_ref1(str1);
      BytesRef shared_bytes_ref1(bytes_ref1);
      BytesRef bytes_ref2(str2);
      BytesRef shared_bytes_ref2(bytes_ref2);

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
      BytesRef bytes_ref1(str1);
      BytesRef shared_bytes_ref1(bytes_ref1);
      BytesRef bytes_ref2(str1);
      BytesRef shared_bytes_ref2(bytes_ref2);

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
