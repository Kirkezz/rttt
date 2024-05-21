#include "RTTT_CLI.hpp"
#include "RTTT_SDL.hpp"
#include <chrono>
#include <thread>

using namespace std::chrono_literals;

extern "C" int main(int argc, char *argv[]) {
    RTTT rttt(3, 3, 3, 2);
    RTTT_SDL rttt_sdl(rttt);
    SDL_Event e;
    bool quit = false;
    while (!quit) {
        auto t1 = std::chrono::system_clock::now();
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
            case SDL_QUIT:
                quit = true;
                break;
            case SDL_MOUSEBUTTONUP:
                if (e.button.button == SDL_BUTTON_LEFT) {
                    rttt_sdl.click(e.button.x, e.button.y);
                }
                break;
            }
        }
        rttt_sdl.render();
        auto t2 = std::chrono::system_clock::now();
        if (auto t = std::chrono::duration_cast<std::chrono::milliseconds>(0.033s - (t2 - t1)).count(); t > 0)
            SDL_Delay(t);
    }
}
