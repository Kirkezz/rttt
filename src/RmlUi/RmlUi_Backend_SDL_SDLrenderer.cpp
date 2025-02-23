/*
 * This source file is part of RmlUi, the HTML/CSS Interface Middleware
 *
 * For the latest information, see http://github.com/mikke89/RmlUi
 *
 * Copyright (c) 2008-2010 CodePoint Ltd, Shift Technology Ltd
 * Copyright (c) 2019-2023 The RmlUi Team, and contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "BackendData.hpp"
#include "RmlUi_Backend.h"
#include "RmlUi_Platform_SDL.h"
#include "RmlUi_Renderer_SDL.h"
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Log.h>

#include <iostream>

Rml::UniquePtr<BackendData> data;
Rml::UniquePtr<BackendData>& BackendData::getData() { return data; }

bool Backend::Initialize(const char* window_name, int width, int height, bool allow_resize) {
    Rml::String available_renderers;

    for (int i = 0; i < SDL_GetNumRenderDrivers(); i++) {
        if (i > 0) available_renderers += ", ";
        available_renderers += SDL_GetRenderDriver(i);
    }

    if (!available_renderers.empty())
        data->system_interface.LogMessage(Rml::Log::LT_INFO, Rml::CreateString("Available SDL renderers: %s", available_renderers.c_str()));

    return true;
}

void Backend::Shutdown() {
    RMLUI_ASSERT(data);

    SDL_DestroyRenderer(data->renderer);
    SDL_DestroyWindow(data->window);

    data.reset();

    SDL_Quit();
}

Rml::SystemInterface* Backend::GetSystemInterface() {
    RMLUI_ASSERT(data);
    return &data->system_interface;
}

Rml::RenderInterface* Backend::GetRenderInterface() {
    RMLUI_ASSERT(data);
    return &data->render_interface;
}

bool Backend::ProcessEvents(Rml::Context* context, KeyDownCallback key_down_callback, bool power_save) {
    RMLUI_ASSERT(data && context);

    auto GetKey = [](const SDL_Event& event) { return event.key.key; };
    auto GetDisplayScale = []() { return SDL_GetWindowDisplayScale(data->window); };
    constexpr auto event_quit = SDL_EVENT_QUIT;
    constexpr auto event_key_down = SDL_EVENT_KEY_DOWN;
    bool has_event = false;

    bool result = data->running;
    data->running = true;

    SDL_Event ev;
    if (power_save)
        has_event = SDL_WaitEventTimeout(&ev, static_cast<int>(Rml::Math::Min(context->GetNextUpdateDelay(), 10.0) * 1000));
    else
        has_event = SDL_PollEvent(&ev);

    while (has_event) {
        bool propagate_event = true;
        switch (ev.type) {
        case event_quit: {
            propagate_event = false;
            result = false;
        } break;
        case event_key_down: {
            propagate_event = false;
            const Rml::Input::KeyIdentifier key = RmlSDL::ConvertKey(GetKey(ev));
            const int key_modifier = RmlSDL::GetKeyModifierState();
            const float native_dp_ratio = GetDisplayScale();

            // See if we have any global shortcuts that take priority over the context.
            if (key_down_callback && !key_down_callback(context, key, key_modifier, native_dp_ratio, true)) break;
            // Otherwise, hand the event over to the context by calling the input handler as normal.
            if (!RmlSDL::InputEventHandler(context, data->window, ev)) break;
            // The key was not consumed by the context either, try keyboard shortcuts of lower priority.
            if (key_down_callback && !key_down_callback(context, key, key_modifier, native_dp_ratio, false)) break;
        } break;
        default: break;
        }

        if (propagate_event) RmlSDL::InputEventHandler(context, data->window, ev);

        has_event = SDL_PollEvent(&ev);
    }

    return result;
}

void Backend::RequestExit() {
    RMLUI_ASSERT(data);
    data->running = false;
}

void Backend::BeginFrame() {
    RMLUI_ASSERT(data);
    data->render_interface.BeginFrame();
}

void Backend::PresentFrame() {
    RMLUI_ASSERT(data);
    data->render_interface.EndFrame();
    SDL_RenderPresent(data->renderer);
}
