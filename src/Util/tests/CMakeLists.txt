add_executable(BytesRefTests BytesRefTests.cpp)
target_link_libraries(BytesRefTests DoochiCore gtest pthread)

add_executable(ArrayUtilTests ArrayUtilTests.cpp)
target_link_libraries(ArrayUtilTests DoochiCore gtest pthread)

add_executable(AttributeTests AttributeTests.cpp)
target_link_libraries(AttributeTests DoochiCore gtest pthread)
