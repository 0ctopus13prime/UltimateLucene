add_executable(TokenStreamTests TokenStreamTests.cpp)
target_link_libraries(TokenStreamTests DoochiCore gtest pthread)

add_executable(ReaderTests ReaderTests.cpp)
target_link_libraries(ReaderTests DoochiCore gtest pthread)
