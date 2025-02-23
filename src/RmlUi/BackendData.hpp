#ifndef BACKENDDATA_HPP
#define BACKENDDATA_HPP

#include "RmlUi_Platform_SDL.h"
#include "RmlUi_Renderer_SDL.h"

/**
    Global data used by this backend.

    Lifetime governed by the calls to Backend::Initialize() and Backend::Shutdown().
 */
struct BackendData {
    BackendData(SDL_Renderer* renderer) : render_interface(renderer) {}

    SystemInterface_SDL system_interface;
    RenderInterface_SDL render_interface;

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    static Rml::UniquePtr<BackendData>& getInstance() {
        static Rml::UniquePtr<BackendData> data;
        return data;
    }
    static Rml::UniquePtr<BackendData>& getData();

    bool running = true;
};

#endif //BACKENDDATA_HPP
