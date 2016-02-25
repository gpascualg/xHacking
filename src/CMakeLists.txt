file(GLOB_RECURSE sources_API                           ${PROJECT_SOURCE_DIR}/include/API/*.h)
file(GLOB_RECURSE sources_Detour    Detour/*.cpp        ${PROJECT_SOURCE_DIR}/include/Detour/*.h)
file(GLOB_RECURSE sources_Loader    Loader/*.cpp        ${PROJECT_SOURCE_DIR}/include/Loader/*.h)
file(GLOB_RECURSE sources_Memory    Memory/*.cpp        ${PROJECT_SOURCE_DIR}/include/Memory/*.h)
file(GLOB_RECURSE sources_Remote    Remote/*.cpp        ${PROJECT_SOURCE_DIR}/include/Remote/*.h)
file(GLOB_RECURSE sources_Utilities Utilities/*.cpp     ${PROJECT_SOURCE_DIR}/include/Utilities/*.h)

set(xHacking_SOURCES
    ${PROJECT_SOURCE_DIR}/include/xHacking.h ${PROJECT_SOURCE_DIR}/src/xHacking.cpp
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
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/lib)
