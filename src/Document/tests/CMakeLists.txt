add_executable(FieldTests FieldTests.cpp)
target_link_libraries(FieldTests DoochiCore gtest pthread)

add_executable(DocumentTests DocumentTests.cpp)
target_link_libraries(DocumentTests DoochiCore gtest pthread)
