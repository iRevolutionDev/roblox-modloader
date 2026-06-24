add_library(rml_thirdparty INTERFACE)
add_library(rml::thirdparty ALIAS rml_thirdparty)

target_include_directories(rml_thirdparty INTERFACE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/g3d/include>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/bullet/include>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/rbxg3d/include>"
)

set(RML_THIRDPARTY_G3D_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/g3d/src" CACHE INTERNAL "Vendored g3d source dir")
