file(GLOB test_src "*.cc")

include_directories("../include")

foreach(src IN LISTS test_src)
    get_filename_component(exe_name ${src} NAME_WE)
    add_executable(${exe_name} ${src})
    target_link_libraries(${exe_name} ${LIB_NAME})
    target_link_libraries(${exe_name} Parser)
    find_library(FLEX_LIB fl)
    target_link_libraries(${exe_name} ${FLEX_LIB})
endforeach(src)
