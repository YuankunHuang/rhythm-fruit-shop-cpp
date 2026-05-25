cmake_minimum_required(VERSION 3.24)
project(rhythm_fruit_shop LANGUAGES CXX)

include(cmake/Warnings.cmake)

find_package(nlohmann_json CONFIG REQUIRED)

# rfs_core: core library, no external dependencies.
add_library(rfs_core STATIC
	src/rhythm/ChartLoader.cpp
	src/rhythm/SmoothedSongClock.cpp
 )
target_include_directories(rfs_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(rfs_core 
	PUBLIC nlohmann_json::nlohmann_json
	PRIVATE rfs_warnings
)

# rfs_platform_iface: platform interface library, depends on rfs_core.
add_library(rfs_platform_iface INTERFACE)
target_include_directories(rfs_platform_iface INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src/platform)
target_link_libraries(rfs_platform_iface INTERFACE rfs_core)

# rfs_platform_sfml, depends on rfs_platform_iface.
find_package(SFML 2.6 COMPONENTS system window graphics REQUIRED)
add_library(rfs_platform_sfml STATIC
	"src/platform/sfml/SfmlWindow.cpp"
	"src/platform/sfml/SfmlRenderer.cpp"
	"src/platform/sfml/SfmlInputSource.cpp"
 )
target_include_directories(rfs_platform_sfml PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(rfs_platform_sfml
	PUBLIC rfs_platform_iface
	PRIVATE sfml-graphics sfml-window sfml-system rfs_warnings
)

# rfs_platform_miniaudio, depends on rfs_platform_iface.
add_library(rfs_platform_miniaudio STATIC
	"src/platform/miniaudio/MiniaudioAudioPlayer.cpp"
	"src/platform/miniaudio/MiniaudioAudioBackendClock.cpp"
)
target_include_directories(rfs_platform_miniaudio
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/external/miniaudio
)
target_link_libraries(rfs_platform_miniaudio
	PUBLIC rfs_platform_iface
	PRIVATE rfs_warnings
)

# rfs_app: main application target, depends on platform interface
add_library(rfs_app STATIC
	src/rfs_app.cpp
	src/app/Application.cpp
	src/app/StateStack.cpp
	src/game/MainMenuState.cpp
	src/game/LoadingState.cpp
	src/game/GameplayState.cpp
)
target_include_directories(rfs_app
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(rfs_app
	PUBLIC rfs_core rfs_platform_iface
	PRIVATE rfs_warnings
)

# rfs_demo: executable target for the main application.
add_executable(rfs_demo 
	src/main.cpp
)
set_target_properties(rfs_demo PROPERTIES
	VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
target_link_libraries(rfs_demo PRIVATE
    rfs_app
    rfs_platform_miniaudio
	rfs_platform_sfml
    sfml-graphics sfml-window sfml-system
)

# rfs_tests: test target, depends on rfs_core and one platform implementation.
add_executable(rfs_tests
	src/rfs_tests.cpp
)
target_link_libraries(rfs_tests
	PRIVATE rfs_core rfs_platform_iface rfs_warnings
)

# DEPENDENCY GUARDS
include(cmake/DependencyGuards.cmake)
rfs_assert_no_forbidden_deps(rfs_core            "sfml;miniaudio")
rfs_assert_no_forbidden_deps(rfs_platform_iface  "sfml;miniaudio")
rfs_assert_no_forbidden_deps(rfs_app             "sfml;miniaudio")
rfs_assert_no_forbidden_deps(rfs_tests           "sfml;miniaudio")