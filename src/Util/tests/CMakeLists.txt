add_executable(RefTests RefTests.cpp)
target_link_libraries(RefTests DoochiCore gtest pthread)

add_executable(ArrayUtilTests ArrayUtilTests.cpp)
target_link_libraries(ArrayUtilTests DoochiCore gtest pthread)

add_executable(AttributeTests AttributeTests.cpp)
target_link_libraries(AttributeTests DoochiCore gtest pthread)

add_executable(AttributeSourceTests AttributeSourceTests.cpp)
target_link_libraries(AttributeSourceTests DoochiCore gtest pthread)

add_executable(ConcurrencyTests ConcurrencyTests.cpp)
target_link_libraries(ConcurrencyTests DoochiCore gtest pthread)

add_executable(EtcTests EtcTests.cpp)
target_link_libraries(EtcTests DoochiCore gtest pthread)

add_executable(NumericTests NumericTests.cpp)
target_link_libraries(NumericTests DoochiCore gtest pthread)

add_executable(FileUtilTests FileUtilTests.cpp)
target_link_libraries(FileUtilTests DoochiCore gtest pthread)

#add_executable(FstTests FstTests.cpp)
#target_link_libraries(FstTests DoochiCore gtest pthread)
