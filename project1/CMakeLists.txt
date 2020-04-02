file(GLOB test_src "*.cc")
file(GLOB kernel_src "./kernels/*.cc")
file(GLOB solution_src "./solution/*.cc")

include_directories("../include")

foreach(src IN LISTS solution_src)
    get_filename_component(exe_name ${src} NAME_WE)
    add_executable(${exe_name} ${src})
    target_link_libraries(${exe_name} ${LIB_NAME})
endforeach(src)

execute_process(
  COMMAND rm -rf ${kernel_src}
  COMMAND touch ${kernel_src}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(test ${test_src} ${kernel_src})
add_executable(solution ${solution_src})

add_custom_target(solution_run
  COMMAND solution
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "run generated solution in ${CMAKE_CURRENT_SOURCE_DIR}"
  SOURCES ${solution_src}
)


add_dependencies(test solution_run)