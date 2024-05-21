cmake_minimum_required(VERSION 3.5)

project(RecursiveTicTacToe LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)
find_package(SDL2 REQUIRED CONFIG COMPONENTS SDL2main)

include(FetchContent)
FetchContent_Declare(SDL_ttf
  GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
  GIT_TAG        SDL2
)

add_executable(RecursiveTicTacToe
    main.cpp
    RTTT.hpp RTTT.cpp
    RTTT_CLI.hpp RTTT_CLI.cpp
    RTTT_SDL.hpp RTTT_SDL.cpp
)

if(TARGET SDL2::SDL2main)
    target_link_libraries(RecursiveTicTacToe PRIVATE SDL2::SDL2main)
endif()
target_link_libraries(RecursiveTicTacToe PRIVATE SDL2::SDL2)

FetchContent_MakeAvailable(SDL_ttf)
target_link_libraries(RecursiveTicTacToe PRIVATE SDL2_ttf::SDL2_ttf)

include(GNUInstallDirs)
install(TARGETS RecursiveTicTacToe
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
