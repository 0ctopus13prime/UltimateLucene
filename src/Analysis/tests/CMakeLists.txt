add_executable(TokenStreamTests TokenStreamTests.cpp)
target_link_libraries(TokenStreamTests DoochiCore gtest pthread)

add_executable(ReaderTests ReaderTests.cpp)
target_link_libraries(ReaderTests DoochiCore gtest pthread)

add_executable(CharacterUtilTests CharacterUtilTests.cpp)
target_link_libraries(CharacterUtilTests DoochiCore gtest pthread)

add_executable(AttributeImplTests AttributeImplTests.cpp)
target_link_libraries(AttributeImplTests DoochiCore gtest pthread)

add_executable(AnalyzerTests AnalyzerTests.cpp)
target_link_libraries(AnalyzerTests DoochiCore gtest pthread)
