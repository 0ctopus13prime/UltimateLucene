#include <future>
#include <string>
#include <mutex>
#include <thread>
#include <Util/Concurrency.h>
#include <gtest/gtest.h>

using namespace lucene::core::util;

TEST(CONCURRENCY__TESTS, CloseableThreadLocal__BASIC) {
  CloseableThreadLocal ctlocal;
  int num = 13;
  ctlocal.Set<int>(&num);
  int* got = ctlocal.Get<int>();
  EXPECT_EQ(num, *got);
}

CloseableThreadLocal global_ctlocal;
void IncreaseNumber(std::string thread_name, std::promise<int>&& promise) {
  int* got = global_ctlocal.Get<int>();
  if(got == nullptr) {
    global_ctlocal.Set<int>(new int(0));
    got = global_ctlocal.Get<int>();
  }

  for(int i = 0 ; i < 1000 ; ++i) {
    (*got)++;
  }

  promise.set_value(*got);
}

TEST(CONCURRENCY__TESTS, CloseableThreadLocal__PARALLEL) {
  std::promise<int> th1_promise, th2_promise, main_promise;
  auto th1_future = th1_promise.get_future();
  auto th2_future = th2_promise.get_future();
  auto main_future = main_promise.get_future();

  std::thread t1(IncreaseNumber, "thread1", std::move(th1_promise)), t2(IncreaseNumber, "thread2", std::move(th2_promise));
  IncreaseNumber("main-thread", std::move(main_promise));

  t1.join();
  t2.join();

  EXPECT_EQ(1000, th1_future.get());
  EXPECT_EQ(1000, th2_future.get());
  EXPECT_EQ(1000, main_future.get());
}


int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
