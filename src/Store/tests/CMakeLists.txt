 add_executable(DataInputTests DataInputTests.cpp)
 target_link_libraries(DataInputTests DoochiCore gtest pthread)

 add_executable(DataOutputTests DataOutputTests.cpp)
 target_link_libraries(DataOutputTests DoochiCore gtest pthread)

 add_executable(DirectoryTests DirectoryTests.cpp)
 target_link_libraries(DirectoryTests DoochiCore gtest pthread)
