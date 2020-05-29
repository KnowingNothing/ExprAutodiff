cmake_minimum_required (VERSION 3.5)
#project (Demo3)

#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")  #设置c++的编译选项
#set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")  #设置c的编译选项

include_directories("../include")

find_package(BISON)
BISON_TARGET(auto_parser auto.bison auto.tab.c)

#add_subdirectory(flex)
#add_executable(auto auto.tab.c)
#target_link_libraries(auto FlexFunction)
#find_library(FLEX_LIB fl)
#target_link_libraries(auto ${FLEX_LIB})
#target_link_libraries(auto ${LIB_NAME})


find_package(FLEX)
FLEX_TARGET(auto_lexer auto.flex lex.yy.c)
# aux_source_directory(. DIR_LIB_SRCS)
file(GLOB PARSER_CXX
    ./*.cc
    )

file(GLOB PARSER_C
    ./*.c
    )
add_library (Parser ${PARSER_C} ${PARSER_CXX})
target_link_libraries(Parser ${LIB_NAME})

add_subdirectory(test)