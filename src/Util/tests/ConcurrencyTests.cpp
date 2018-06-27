#include <memory>
#include <future>
#include <string>
#include <mutex>
#include <thread>
#include <Util/Concurrency.h>
#include <gtest/gtest.h>

using namespace lucene::core::util;

class DummyClass {};

TEST(CONCURRENCY__TESTS, CloseableThreadLocal__BASIC) {
  // DummyClass's thread local associate variable table
  CloseableThreadLocal<DummyClass, int> ctlocal;
  ctlocal.Set(13);
  int got = ctlocal.Get();
  EXPECT_EQ(13, got);
}

CloseableThreadLocal<DummyClass, int> global_ctlocal;
void IncreaseNumber(std::string thread_name, std::promise<int>&& promise) {
  try {
    int& got = global_ctlocal.Get();
  } catch(EmptyThreadLocalException&) {
    global_ctlocal.Set(0);
  }

  int& got = global_ctlocal.Get();
  for(int i = 0 ; i < 1000 ; ++i) {
    got++;
  }

  promise.set_value(got);
}

TEST(CONCURRENCY__TESTS, CloseableThreadLocal__PARALLEL) {
  std::promise<int> th1_promise, th2_promise, main_promise;
  auto th1_future = th1_promise.get_future();
  auto th2_future = th2_promise.get_future();
  auto main_future = main_promise.get_future();

  std::thread t1(IncreaseNumber, "thread1", std::move(th1_promise)),
              t2(IncreaseNumber, "thread2", std::move(th2_promise));
  IncreaseNumber("main-thread", std::move(main_promise));

  t1.join();
  t2.join();

  EXPECT_EQ(1000, th1_future.get());
  EXPECT_EQ(1000, th2_future.get());
  EXPECT_EQ(1000, main_future.get());
}

TEST(CONCURRENCY__TESTS, CloseableThreadLocal__EACH) {
  CloseableThreadLocal<DummyClass, int> ctlocal1;
  ctlocal1.Set(13);

  CloseableThreadLocal<DummyClass, int> ctlocal2;
  ctlocal2.Set(1313);

  EXPECT_EQ(13, ctlocal1.Get());
  EXPECT_EQ(1313, ctlocal2.Get());

  ctlocal1.Close();
  ctlocal2.Close();
}

TEST(CONCURRENCY__TESTS, CloseableThreadLocal__MOVE) {
  std::unique_ptr<int> ptr = std::make_unique<int>(13);
  CloseableThreadLocal<DummyClass, std::unique_ptr<int>> ctlocal;
  ctlocal.Set(std::move(ptr));

  std::unique_ptr<int>& got = ctlocal.Get();
  EXPECT_EQ(13, *got);
}

TEST(CONCURRENCY__TESTS, CloseableTheadLocal__CLEAN) {
  {
    CloseableThreadLocal<DummyClass, int> ctlocal;
    ctlocal.Set(13);
  }

  CloseableThreadLocal<DummyClass, int> ctlocal;
  try {
    ctlocal.Get();
    FAIL();
  } catch(EmptyThreadLocalException&) {
  }
}


int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
