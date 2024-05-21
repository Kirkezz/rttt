cmake_minimum_required(VERSION 3.6)

project(RecursiveTicTacToe)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(SDL
        URL https://github.com/libsdl-org/SDL/releases/download/release-2.30.3/SDL2-2.30.3.tar.gz
)
FetchContent_Declare(SDL_ttf
        GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
        GIT_TAG SDL2
)

FetchContent_MakeAvailable(SDL SDL_ttf)

add_library(main SHARED)

target_sources(main PRIVATE
        main.cpp
        RTTT.hpp RTTT.cpp
        RTTT_CLI.hpp RTTT_CLI.cpp
        RTTT_SDL.hpp RTTT_SDL.cpp
)

target_link_libraries(main PRIVATE
        SDL2::SDL2
        SDL2::SDL2main
        SDL2_ttf::SDL2_ttf
)
