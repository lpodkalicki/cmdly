# Helper macro adding example
macro(add_example EXAMPLE_SOURCE)
    get_filename_component(FILE_NAME ${EXAMPLE_SOURCE} NAME_WE)
    set(EXAMPLE_NAME "${FILE_NAME}")
    add_executable(${EXAMPLE_NAME} ${EXAMPLE_SOURCE})
    target_link_libraries(${EXAMPLE_NAME} cmdly)
    target_compile_options(${EXAMPLE_NAME} PUBLIC -Wall -Wextra -pedantic -Werror)
    install(TARGETS ${EXAMPLE_NAME} DESTINATION bin COMPONENT runtime)
endmacro()
# Collect examples
file(GLOB_RECURSE EXAMPLE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
foreach (example_file ${EXAMPLE_SOURCES})
    add_example(${example_file})
endforeach ()
