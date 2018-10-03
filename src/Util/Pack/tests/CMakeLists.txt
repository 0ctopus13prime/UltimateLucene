add_executable(PagedTests PagedTests.cpp)
target_link_libraries(PagedTests DoochiCore gtest pthread)

add_executable(PackedTests PackedTests.cpp)
target_link_libraries(PackedTests DoochiCore gtest pthread)
