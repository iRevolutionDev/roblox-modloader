message(STATUS "Configuring Discord RPC mod with Game SDK dependencies...")

set(DISCORD_SDK_VERSION "3.2.1")
set(DISCORD_SDK_URL "https://dl-game-sdk.discordapp.net/${DISCORD_SDK_VERSION}/discord_game_sdk.zip")

include(FetchContent)
FetchContent_Declare(
        discord_game_sdk_${CURRENT_MOD_NAME}
        URL ${DISCORD_SDK_URL}
)

FetchContent_GetProperties(discord_game_sdk_${CURRENT_MOD_NAME})
if (NOT discord_game_sdk_${CURRENT_MOD_NAME}_POPULATED)
    message(STATUS "Downloading Discord Game SDK for ${CURRENT_MOD_NAME}...")
    FetchContent_Populate(discord_game_sdk_${CURRENT_MOD_NAME})
endif ()

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(DISCORD_SDK_LIB_DIR "${discord_game_sdk_${CURRENT_MOD_NAME}_SOURCE_DIR}/lib/x86_64")
else ()
    set(DISCORD_SDK_LIB_DIR "${discord_game_sdk_${CURRENT_MOD_NAME}_SOURCE_DIR}/lib/x86")
endif ()

set(DISCORD_SDK_INCLUDE_DIR "${discord_game_sdk_${CURRENT_MOD_NAME}_SOURCE_DIR}/cpp")
set(DISCORD_SDK_DLL_PATH "${DISCORD_SDK_LIB_DIR}/discord_game_sdk.dll")
set(DISCORD_SDK_LIB_PATH "${DISCORD_SDK_LIB_DIR}/discord_game_sdk.dll.lib")

file(GLOB DISCORD_SDK_SOURCES "${DISCORD_SDK_INCLUDE_DIR}/*.cpp")

if (NOT EXISTS ${DISCORD_SDK_INCLUDE_DIR})
    message(FATAL_ERROR "Discord Game SDK include directory not found: ${DISCORD_SDK_INCLUDE_DIR}")
endif ()

if (NOT EXISTS ${DISCORD_SDK_LIB_PATH})
    message(FATAL_ERROR "Discord Game SDK library not found: ${DISCORD_SDK_LIB_PATH}")
endif ()

if (NOT EXISTS ${DISCORD_SDK_DLL_PATH})
    message(FATAL_ERROR "Discord Game SDK DLL not found: ${DISCORD_SDK_DLL_PATH}")
endif ()

if (NOT DISCORD_SDK_SOURCES)
    message(FATAL_ERROR "No Discord Game SDK source files found in: ${DISCORD_SDK_INCLUDE_DIR}")
endif ()

add_library(discord_sdk_${CURRENT_MOD_NAME} STATIC ${DISCORD_SDK_SOURCES})
target_include_directories(discord_sdk_${CURRENT_MOD_NAME} PUBLIC ${DISCORD_SDK_INCLUDE_DIR})
target_link_libraries(discord_sdk_${CURRENT_MOD_NAME} PRIVATE ${DISCORD_SDK_LIB_PATH})

set_target_properties(discord_sdk_${CURRENT_MOD_NAME} PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        FOLDER "Examples/Discord"
)

create_custom_roblox_mod(
        DEPENDENCIES
        discord_sdk_${CURRENT_MOD_NAME}
        ${DISCORD_SDK_LIB_PATH}

        INCLUDE_DIRS
        ${DISCORD_SDK_INCLUDE_DIR}

        COMPILE_DEFINITIONS
        DISCORD_SDK_AVAILABLE=1
        DISCORD_CLIENT_ID=1396335710755098757LL
)

message(STATUS "Discord Game SDK configured successfully for ${CURRENT_MOD_NAME}")
message(STATUS "  - Include Dir: ${DISCORD_SDK_INCLUDE_DIR}")
message(STATUS "  - Library: ${DISCORD_SDK_LIB_PATH}")
message(STATUS "  - DLL: ${DISCORD_SDK_DLL_PATH}")
