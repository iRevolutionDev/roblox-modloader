include(FetchContent)

message("ZLIB")
FetchContent_Declare(
        zlib-cmake
        URL https://github.com/jimmy-park/zlib-cmake/archive/main.tar.gz
)

FetchContent_MakeAvailable(zlib-cmake)