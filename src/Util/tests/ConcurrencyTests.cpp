/*
 *
 * Copyright (c) 2018-2019 Doo Yong Kim. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <gtest/gtest.h>
#include <Util/Concurrency.h>
#include <memory>
#include <future>
#include <string>
#include <mutex>
#include <thread>

using lucene::core::util::CloseableThreadLocal;
using lucene::core::util::EmptyThreadLocalException;

class DummyClass {};

TEST(CONCURRENCY__TESTS, CloseableThreadLocal__BASIC) {
  // DummyClass's thread local associate variable table
  CloseableThreadLocal<DummyClass, int> ctlocal;
  ctlocal.Set(13);
  int got = ctlocal.Get();
  EXPECT_EQ(13, got);
}

void IncreaseNumber(std::string thread_name, std::promise<int>&& promise) {
  CloseableThreadLocal<DummyClass, int> ctlocal;
  try {
    int& got = ctlocal.Get();
  } catch(EmptyThreadLocalException&) {
    ctlocal.Set(0);
  }

  int& got = ctlocal.Get();
  for (int i = 0 ; i < 1000 ; ++i) {
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
