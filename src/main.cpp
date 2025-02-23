#define SDL_MAIN_USE_CALLBACKS 1 /* use the callbacks instead of main() */

#include <array>
#include <cmath>
#include <ctime>
#include <iostream>
#include <random>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_video.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "RTTT.hpp"
#include "RTTT_Engine.hpp"

RTTT_Engine engine;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    SDL_SetAppMetadata("RTTT", "1.3", "org.kirkezz.rttt");

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!TTF_Init()) {
        SDL_Log("Couldn't initialize TTF: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    return engine.init();
}
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    if (event->type == SDL_EVENT_QUIT || !engine.running) { return SDL_APP_SUCCESS; }
    if (event->type == SDL_EVENT_MOUSE_MOTION) { engine.mouseMove(event->motion.x, event->button.y); }
    if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) { engine.buttonDown(event->button.x, event->button.y); }
    if (event->type == SDL_EVENT_MOUSE_BUTTON_UP) { engine.buttonUp(event->button.x, event->button.y); }
    if (event->type == SDL_EVENT_KEY_DOWN) { engine.keyDown(event->key.key); }
    return SDL_APP_CONTINUE;
}
SDL_AppResult SDL_AppIterate(void *appstate) {
    engine.renderCurrentScene();
    SDL_RenderPresent(engine.renderer);
    return SDL_APP_CONTINUE;
}
void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    Rml::Shutdown();
    Backend::Shutdown();
}
