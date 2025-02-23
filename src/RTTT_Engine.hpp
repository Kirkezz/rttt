#ifndef RTTT_ENGINE_HPP
#define RTTT_ENGINE_HPP

#include "RTTT.hpp"

#include "RmlUi/BackendData.hpp"
#include "RmlUi/EventListener.hpp"
#include "RmlUi/FileInterface.hpp"
#include "RmlUi/RmlUi_Backend.h"
#include "RmlUi/RmlUi_Platform_SDL.h"
#include "RmlUi/RmlUi_Renderer_SDL.h"

#include <RmlUi/Core.h>
#include <RmlUi/Debugger.h>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <array>
#include <cmath>

struct RTTT_Engine {
    friend class ElementsEventListener;
    bool running = true;

    SDL_Renderer *renderer = nullptr;
    SDL_Window *window = nullptr;
    const SDL_DisplayMode *dm;

    Rml::Context *context;
    Rml::ElementDocument *menu_doc, *new_game_doc, *settings_doc, *active_game;
    EventListener event_listener;

    RTTT_Engine() : rttt(3, 3, 3, 2) {}

    enum Scene { MENU, ACTIVE_GAME };
    SDL_AppResult init() {
        if (SDL_DisplayID *displays = SDL_GetDisplays(nullptr); displays && (dm = SDL_GetDesktopDisplayMode(*displays)) == nullptr) {
            SDL_Log("%s", SDL_GetError());
            dm = new SDL_DisplayMode{.w = 1024, .h = 1024};
        }

        SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");

        SDL_CreateWindowAndRenderer("RTTT", dm->w, dm->h, 0, &window, &renderer);
        SDL_SetRenderVSync(renderer, 1);

        for (int i = 1; i <= RTTT::players_n_max; ++i) {
            t_player[i - 1] = loadTexture(std::string("player_" + std::to_string(i) + ".bmp").c_str());
        }

        // text_engine = TTF_CreateRendererTextEngine(renderer);
        // font = TTF_OpenFont("yoster.ttf", 24);

        Rml::UniquePtr<BackendData> &data = BackendData::getData();
        data = Rml::MakeUnique<BackendData>(renderer);
        data->window = window;
        data->system_interface.SetWindow(window);
        data->renderer = renderer;

        Backend::Initialize("", 0, 0, true);

        Rml::SetSystemInterface(Backend::GetSystemInterface());
        Rml::SetRenderInterface(Backend::GetRenderInterface());
        Rml::SetFileInterface(new FileInterface);

        event_listener.engine = this;
        Rml::Factory::RegisterEventListenerInstancer(&event_listener);

        Rml::Initialise();

        context = Rml::CreateContext("main", Rml::Vector2i(dm->w, dm->h));
        const Rml::String directory = "RmlAssets/";

        struct FontFace {
            const char *filename;
            bool fallback_face;
        };
        FontFace font_faces[] = {
            {"LatoLatin-Regular.ttf", false},    {"LatoLatin-Italic.ttf", false}, {"LatoLatin-Bold.ttf", false},
            {"LatoLatin-BoldItalic.ttf", false}, {"NotoEmoji-Regular.ttf", true},
        };
        for (const FontFace &face : font_faces)
            if (!Rml::LoadFontFace(directory + face.filename, face.fallback_face)) { return SDL_APP_FAILURE; }

        menu_doc = context->LoadDocument("RmlAssets/menu.rml");
        menu_doc->Show();

        new_game_doc = context->LoadDocument("RmlAssets/new_game.rml");
        settings_doc = context->LoadDocument("RmlAssets/settings.rml");
        {
            Rml::Event event;
            ElementsEventListener(this, "saveSettings", nullptr).ProcessEvent(event);
        }
        active_game = context->LoadDocument("RmlAssets/active_game.rml");

        Rml::ElementList elements;
        new_game_doc->GetElementsByClassName(elements, "form_field");
        for (auto i : elements) {
            i->AddEventListener("change", event_listener.InstanceEventListener(i->GetId(), i));
        }

        return SDL_APP_CONTINUE;
    }
    Scene current_scene = Scene::ACTIVE_GAME;

    SDL_FRect TTTRect;

    void renderCurrentScene() {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        if (current_scene == Scene::MENU) {
            //text = TTF_CreateText(text_engine, font, "waffle", 0);
            //TTF_SetTextColor(text, 0x00, 0x00, 0x00, SDL_ALPHA_OPAQUE);
            //
            //Button test SDL_Rect caption_rect;
            //TTF_GetTextSize(text, &caption_rect.w, &caption_rect.h);
            //std::cout << caption_rect.w << " " << caption_rect.h << std::endl;
            //TTF_DrawRendererText(text, (float) 0.0f, (float) 0.0f);
        } else if (current_scene == Scene::ACTIVE_GAME) { // rewrite
            TTTRect = SDL_FRect{0, 0, 0, 0};
            TTTRect.w = TTTRect.h = std::min(dm->w, dm->h);
            (dm->w > dm->h ? TTTRect.x : TTTRect.y) = std::abs(dm->w - dm->h) / 2;

            std::vector<std::vector<SDL_FRect>> current_area{{{TTTRect.x, TTTRect.y, TTTRect.w, TTTRect.h}}};
            if (rttt.firstGame()) {
                current_area = drawTTT(rttt.current_top[0][0], current_area[0][0], false, false, true);
                for (auto &i : current_area) {
                    for (auto &j : i) {
                        drawBoundaries(j);
                    }
                }
            } else {
                current_area = drawTTT(RTTT::TTT(rttt.current_top, rttt.previous_pos), current_area[0][0], false, false, false);
                int layers_index = rttt.layers.size() - 1;
                auto *top = &rttt.current_top;
                do {
                    for (unsigned i = 0; i < rttt.width; ++i) {
                        for (unsigned j = 0; j < rttt.height; ++j) {
                            if (layers_index < 0 || rttt.layers[layers_index].pos != Pos2D(i, j))
                                drawTTT((*top)[i][j], current_area[i][j], top == &rttt.current_top, Pos2D(i, j) != rttt.last_pos && !rttt.isLastPosInvalid(),
                                        i == transparent_pos.x / rttt.width && j == transparent_pos.y / rttt.height && layers_index == rttt.layers.size() - 1);
                        }
                    }
                    if (layers_index >= 0) {
                        current_area = drawTTT((*top)[rttt.layers[layers_index].pos.x][rttt.layers[layers_index].pos.y],
                                               current_area[rttt.layers[layers_index].pos.x][rttt.layers[layers_index].pos.y], top == &rttt.current_top, false,
                                               false);
                        top = &rttt.layers[layers_index].layer;
                    }
                } while ((--layers_index) + 1 >= 0);
            }
        }
        Backend::ProcessEvents(context);
        context->Update();
        context->Render();
    }

    Pos2D mouseToPos(float x, float y) {
        x -= TTTRect.x;
        y -= TTTRect.y;

        if (x >= TTTRect.w || y >= TTTRect.h || x < 0 || y < 0) return Pos2D(-1, -1);

        Pos2D last_pos;
        if (rttt.firstGame()) return Pos2D(x / (TTTRect.w / rttt.width), y / (TTTRect.h / rttt.height));
        return Pos2D(x / (TTTRect.w / rttt.width / rttt.width), y / (TTTRect.h / rttt.height / rttt.height));
    }

    Pos2D transparent_pos = Pos2D(-1, -1);

    void mouseMove(float x, float y) {
        context->ProcessMouseMove(x, y, 0);
        if (transparent_pos != Pos2D(-1, -1)) { transparent_pos = mouseToPos(x, y); }
    }

    void buttonDown(float x, float y) {
        context->ProcessMouseButtonDown(0, 0);
        if (active_game->IsVisible()) transparent_pos = mouseToPos(x, y);
    }

    void buttonUp(float x, float y) {
        bool visible = new_game_doc->IsVisible();
        context->ProcessMouseButtonUp(0, 0);
        x -= TTTRect.x;
        y -= TTTRect.y;

        if (x >= TTTRect.w || y >= TTTRect.h || x < 0 || y < 0 || visible || transparent_pos == Pos2D(-1, -1)) return;

        Pos2D last_pos(x / (TTTRect.w / rttt.width), y / (TTTRect.h / rttt.height));
        if (rttt.firstGame()) {
            rttt.move(last_pos);
        } else {
            Pos2D move_pos((x - last_pos.x * (TTTRect.w / rttt.width)) / ((TTTRect.w / rttt.width) / rttt.width),
                           (y - last_pos.y * (TTTRect.h / rttt.height)) / ((TTTRect.h / rttt.height) / rttt.height));
            if (last_pos == rttt.last_pos || rttt.isLastPosInvalid()) {
                Pos2D temp = rttt.last_pos;
                rttt.last_pos = last_pos;
                if (rttt.move(move_pos) != RTTT::MoveErrorCode::OK) {
                    rttt.last_pos = temp;
                }
            }
        }
        transparent_pos = Pos2D(-1, -1);
    }

    void keyDown(SDL_Keycode key) {
        if (key == SDLK_BACKSPACE)
            context->ProcessKeyDown(RmlSDL::ConvertKey(key), 0);
        else
            context->ProcessTextInput(key);
    }

    void createOfflineGame(unsigned width, unsigned height, unsigned win_condition, unsigned players_n) { rttt = RTTT(width, height, win_condition, players_n); }

  private:
    RTTT rttt;
    std::array<SDL_Texture *, RTTT::players_n_max> t_player;

    // TTF_TextEngine *text_engine = nullptr;
    // TTF_Font *font = nullptr;
    // TTF_Text *text = nullptr;

    SDL_Texture *loadTexture(const char *path) {
        SDL_Surface *surface = SDL_LoadBMP(path);
        if (!surface) { return nullptr; }
        SDL_SetSurfaceColorKey(surface, true, SDL_MapRGB(SDL_GetPixelFormatDetails(surface->format), nullptr, 0, 0, 0));

        SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (!texture) { return nullptr; }

        SDL_DestroySurface(surface);

        return texture;
    }

    void drawBoundaries(SDL_FRect area, int opacity = 224) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, opacity);

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND_PREMULTIPLIED);
        SDL_RenderRect(renderer, &area);
        --area.x;
        --area.y;
        SDL_RenderRect(renderer, &area);

        area.x += 2;
        area.y += 2;
        SDL_RenderRect(renderer, &area);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    }

    std::vector<std::vector<SDL_FRect>> drawTTT(const RTTT::TTT &ttt, const SDL_FRect &area, bool unavailable, bool not_current, bool draw_hover) {
        bool gray = unavailable && (not_current || ttt.gameEnded());
        if (gray) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND_PREMULTIPLIED);
            SDL_RenderFillRect(renderer, &area);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        }

        std::vector<std::vector<SDL_FRect>> result(rttt.width, std::vector<SDL_FRect>(rttt.height));
        for (int i = 0; i < rttt.width; ++i) {
            for (int j = 0; j < rttt.height; ++j) {
                result[i][j] = SDL_FRect{std::floor(area.x + area.w * i / rttt.width), std::floor(area.y + area.h * j / rttt.height), //
                                         std::floor(area.w / rttt.width), std::floor(area.h / rttt.height)};

                if (ttt.field[i][j])
                    SDL_RenderTexture(renderer, t_player[ttt.field[i][j] - 1], nullptr, &result[i][j]);
                else if (transparent_pos.x != -1 && !gray && draw_hover && transparent_pos.x % rttt.width == i && transparent_pos.y % rttt.height == j) {
                    SDL_FRect &a = result[i][j], temp = (SDL_FRect){a.x, a.y, a.w / 1.5f, a.h / 1.5f};
                    SDL_RenderTexture(renderer, t_player[rttt.player_to_move - 1], nullptr, &temp);
                }
                if (unavailable) drawBoundaries(result[i][j], 64 + gray * 128);
            }
        }
        if (unavailable) { drawBoundaries(area); }

        return result;
    }
};

#endif // RTTT_ENGINE_HPP
