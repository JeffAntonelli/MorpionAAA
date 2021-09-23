cmake_minimum_required (VERSION 3.18)
project(GPR5100_TurnBased)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (MSVC)
    # warning level 4 and all warnings as errors
    add_compile_options(/W4 /w14640)
    add_compile_definitions(_USE_MATH_DEFINES)
else()
    # lots of warnings and all warnings as errors
    add_compile_options(-Wall -Wextra -Wshadow -Wnon-virtual-dtor)
endif()

find_package(SFML COMPONENTS system window graphics network audio CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(ImGui-SFML CONFIG REQUIRED)

file(GLOB_RECURSE gpr5100_src include/*.h src/*.cpp)
add_library(GPR5100_Lib STATIC ${gpr5100_src})
target_include_directories(GPR5100_Lib PUBLIC include/)
target_link_libraries(GPR5100_Lib PUBLIC sfml-system sfml-network sfml-graphics sfml-window
	sfml-network sfml-audio ImGui-SFML::ImGui-SFML)
set_target_properties(GPR5100_Lib PROPERTIES UNITY_BUILD OFF)

add_executable(echo_client main/main_echo_client.cpp)
target_link_libraries(echo_client PRIVATE GPR5100_Lib)

add_executable(echo_server main/main_echo_server.cpp)
target_link_libraries(echo_server PRIVATE GPR5100_Lib)

add_executable(morpion_server main/main_morpion_server.cpp)
target_link_libraries(morpion_server PRIVATE GPR5100_Lib)

add_executable(morpion_client main/main_morpion_client.cpp)
target_link_libraries(morpion_client PRIVATE GPR5100_Lib)