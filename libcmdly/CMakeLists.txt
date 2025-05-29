set(TARGET_NAME "cmdly")
file(GLOB_RECURSE TARGET_SOURCES "src/*.cpp")
add_library(${TARGET_NAME}_object OBJECT ${TARGET_SOURCES})
set_property(TARGET ${TARGET_NAME}_object PROPERTY POSITION_INDEPENDENT_CODE 1)
target_compile_options(${TARGET_NAME}_object PUBLIC -Wall -Wextra -pedantic -O3)
target_include_directories(${TARGET_NAME}_object
        PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        PUBLIC "$<INSTALL_INTERFACE:include>")

add_library(${TARGET_NAME} SHARED $<TARGET_OBJECTS:${TARGET_NAME}_object>)
target_link_libraries(${TARGET_NAME} "")
target_include_directories(${TARGET_NAME}
        PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        PUBLIC "$<INSTALL_INTERFACE:include>")


add_library(${TARGET_NAME}_static STATIC $<TARGET_OBJECTS:${TARGET_NAME}_object>)
target_link_libraries(${TARGET_NAME}_static "")
target_include_directories(${TARGET_NAME}_static
        PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        PUBLIC "$<INSTALL_INTERFACE:include>")

file(GLOB TARGET_PUBLIC_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/cmdly/*.h")
install(FILES ${TARGET_PUBLIC_HEADERS} DESTINATION include/cmdly COMPONENT development)

install(TARGETS ${TARGET_NAME} ${TARGET_NAME}_static
        EXPORT CmdlyConfig
        LIBRARY DESTINATION lib COMPONENT runtime
        ARCHIVE DESTINATION lib COMPONENT runtime
        RUNTIME DESTINATION bin COMPONENT runtime
        PUBLIC_HEADER DESTINATION include/cmdly COMPONENT development)

install(EXPORT CmdlyConfig FILE CmdlyConfig.cmake DESTINATION export NAMESPACE Cmdly:: COMPONENT development)

if (BUILD_TESTING)
    add_subdirectory(tests)
endif()
