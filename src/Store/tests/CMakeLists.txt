 add_executable(DataInputOutputTests DataInputOutputTests.cpp)
 target_link_libraries(DataInputOutputTests DoochiCore gtest pthread)

 add_executable(DirectoryTests DirectoryTests.cpp)
 target_link_libraries(DirectoryTests DoochiCore gtest pthread)
