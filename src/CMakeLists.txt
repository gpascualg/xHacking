file(GLOB_RECURSE sources_API                           ${PROJECT_SOURCE_DIR}/include/xhacking/API/*.h)
file(GLOB_RECURSE sources_Detour    Detour/*.cpp        ${PROJECT_SOURCE_DIR}/include/xhacking/Detour/*.h)
file(GLOB_RECURSE sources_Loader    Loader/*.cpp        ${PROJECT_SOURCE_DIR}/include/xhacking/Loader/*.h)
file(GLOB_RECURSE sources_Memory    Memory/*.cpp        ${PROJECT_SOURCE_DIR}/include/xhacking/Memory/*.h)
file(GLOB_RECURSE sources_Remote    Remote/*.cpp        ${PROJECT_SOURCE_DIR}/include/xhacking/Remote/*.h)
file(GLOB_RECURSE sources_Utilities Utilities/*.cpp     ${PROJECT_SOURCE_DIR}/include/xhacking/Utilities/*.h)

set(xHacking_SOURCES
    ${PROJECT_SOURCE_DIR}/include/xhacking/xHacking.h ${PROJECT_SOURCE_DIR}/src/xHacking.cpp
    ${sources_API}
    ${sources_Detour}
    ${sources_Loader}
    ${sources_Memory}
    ${sources_Remote}
    ${sources_Utilities}
)

add_library(xHacking STATIC ${xHacking_SOURCES})
target_link_libraries(xHacking ws2_32)

set_property(TARGET xHacking PROPERTY CXX_STANDARD 11)
target_compile_features(xHacking PRIVATE cxx_decltype cxx_nullptr cxx_variadic_templates)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set_target_properties(xHacking PROPERTIES RELEASE_POSTFIX 64 DEBUG_POSTFIX 64_d)
else()
    set_target_properties(xHacking PROPERTIES DEBUG_POSTFIX _d)
endif()

set_target_properties(xHacking PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/lib
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/lib

    INTERFACE_INCLUDE_DIRECTORIES "$<INSTALL_INTERFACE:include>")

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/xhacking DESTINATION include/)
install(TARGETS xHacking DESTINATION lib)

include(CMakePackageConfigHelpers)
set(config_install_dir lib/cmake/xHacking)
set(version_config ${PROJECT_BINARY_DIR}/xHacking-config-version.cmake)
set(project_config ${PROJECT_BINARY_DIR}/xHacking-config.cmake)
set(targets_export_name xHacking-targets)

# Generate the version, config and target files into the build directory.
write_basic_package_version_file(
  ${version_config}
  VERSION 0.0.1
  COMPATIBILITY AnyNewerVersion)
configure_package_config_file(
  ${PROJECT_SOURCE_DIR}/config.cmake.in
  ${project_config}
  INSTALL_DESTINATION ${config_install_dir})
export(TARGETS xHacking FILE ${PROJECT_BINARY_DIR}/${targets_export_name}.cmake)

install(TARGETS xHacking
	EXPORT ${targets_export_name}
	RUNTIME DESTINATION bin/
  	LIBRARY DESTINATION lib/
  	ARCHIVE DESTINATION lib/)

install(FILES ${project_config} ${version_config} DESTINATION ${config_install_dir})
install(EXPORT ${targets_export_name} DESTINATION ${config_install_dir})
