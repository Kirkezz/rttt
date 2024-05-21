#ifndef RTTT_SDL_HPP
#define RTTT_SDL_HPP

#include "RTTT.hpp"
#include <SDL2/SDL.h>
#include <SDL_ttf.h>
#include <functional>

class RTTT_SDL {
public:
    int new_width = 3, new_height = 3, new_win_condition = 3, new_players_n = 2;
    RTTT_SDL(RTTT& rttt) : rttt(rttt) {
        if (SDL_Init(SDL_INIT_VIDEO) < 0)
            throw std::runtime_error(SDL_GetError());
        if (SDL_GetDesktopDisplayMode(0, &dm) != 0) {
            SDL_Log("%s", SDL_GetError());
            dm.w = 1024;
            dm.h = 1024;
        }

        window = SDL_CreateWindow("Recursive Tic-Tac-Toe", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, dm.h * 0.75, dm.h * 0.75, SDL_WINDOW_SHOWN);

        for (unsigned i = 1; i <= rttt.players_n_max; ++i) {
            player_surface.push_back(SDL_LoadBMP(std::string("player_" + std::to_string(i) + ".bmp").c_str()));
            SDL_SetColorKey(player_surface.back(), SDL_TRUE, SDL_MapRGB(player_surface.back()->format, 255, 255, 255));
        }
        reset_button = SDL_LoadBMP("reset.bmp");
        SDL_SetColorKey(reset_button, SDL_TRUE, SDL_MapRGB(reset_button->format, 255, 255, 255));

        TTF_Init();
        font = TTF_OpenFont("yoster.ttf", 36);
        // clang-format off
        buttons.push_back(Button{font, SDL_Rect{0, int(dm.h * 0.75 - 128), int(dm.h * 0.75), 128}, SDL_Color{100, 255, 200, 255}, SDL_Color{0, 0, 0, 255}, "Play", [&](){ rttt = RTTT(new_width, new_height, new_win_condition, new_players_n); scene = GAME_SCENE; }});
        buttons.push_back(Button{font, SDL_Rect{int(dm.h * 0.75 - 80), 128 + 8, 48, 48}, SDL_Color{35, 226, 0, 255}, SDL_Color{0, 0, 0, 255}, "+", [&]() { if (new_width != 16) ++new_width; }});
        buttons.push_back(Button{font, SDL_Rect{int(dm.h * 0.75 - 160), 128 + 8, 48, 48}, SDL_Color{255, 8, 0, 255}, SDL_Color{0, 0, 0, 255}, "-", [&]() { if (new_width != 1) --new_width; }});
        buttons.push_back(Button{font, SDL_Rect{int(dm.h * 0.75 - 80), 192 + 8, 48, 48}, SDL_Color{35, 226, 0, 255}, SDL_Color{0, 0, 0, 255}, "+", [&]() { if (new_height != 16) ++new_height; }});
        buttons.push_back(Button{font, SDL_Rect{int(dm.h * 0.75 - 160), 192 + 8, 48, 48}, SDL_Color{255, 8, 0, 255}, SDL_Color{0, 0, 0, 255}, "-", [&]() { if (new_height != 1) --new_height; }});
        buttons.push_back(Button{font, SDL_Rect{int(dm.h * 0.75 - 80), 256 + 8, 48, 48}, SDL_Color{35, 226, 0, 255}, SDL_Color{0, 0, 0, 255}, "+", [&]() { if (new_win_condition != 16) ++new_win_condition; }});
        buttons.push_back(Button{font, SDL_Rect{int(dm.h * 0.75 - 160), 256 + 8, 48, 48}, SDL_Color{255, 8, 0, 255}, SDL_Color{0, 0, 0, 255}, "-", [&]() { if (new_win_condition != 1) --new_win_condition; }});
        buttons.push_back(Button{font, SDL_Rect{int(dm.h * 0.75 - 80), 320 + 8, 48, 48}, SDL_Color{35, 226, 0, 255}, SDL_Color{0, 0, 0, 255}, "+", [&]() { if (new_players_n != 4) ++new_players_n; }});
        buttons.push_back(Button{font, SDL_Rect{int(dm.h * 0.75 - 160), 320 + 8, 48, 48}, SDL_Color{255, 8, 0, 255}, SDL_Color{0, 0, 0, 255}, "-", [&]() { if (new_players_n != 1) --new_players_n; }});
        // clang-format on

        render();
    }

    enum Scene { MENU_SCENE, GAME_SCENE } scene = GAME_SCENE;

    int prev_w = -1, prev_h = -1;
    bool redraw = true;

    SDL_Rect TTTRect{0, 0, 0, 0};
    void render() {
        int w, h;
        SDL_GetWindowSizeInPixels(window, &w, &h);
        if (w != prev_w || h != prev_h)
            redraw = true;
        if (!redraw) {
            SDL_UpdateWindowSurface(window);
            return;
        }
        screen_surface = SDL_GetWindowSurface(window);
        SDL_FillRect(screen_surface, NULL, SDL_MapRGB(screen_surface->format, 255, 255, 255));
        if (scene == MENU_SCENE) {
            SDL_Color black{0, 0, 0, 255};
            SDL_Rect above{0, 0, screen_surface->w, 128};
            renderText(screen_surface, font, "Recursive Tic-Tac-Toe", black, above);
            SDL_Rect width_pos{0, 128, screen_surface->w, 64};
            renderText(screen_surface, font, ("Width - " + std::to_string(new_width)).c_str(), black, width_pos, ALIGN_LEFT);
            SDL_Rect height_pos{0, 192, screen_surface->w, 64};
            renderText(screen_surface, font, ("Height - " + std::to_string(new_height)).c_str(), black, height_pos, ALIGN_LEFT);
            SDL_Rect win_cond_pos{0, 256, screen_surface->w, 64};
            renderText(screen_surface, font, ("Win cond. - " + std::to_string(new_win_condition)).c_str(), black, win_cond_pos, ALIGN_LEFT);
            SDL_Rect players_n_pos{0, 320, screen_surface->w, 64};
            renderText(screen_surface, font, ("Players - " + std::to_string(new_players_n)).c_str(), black, players_n_pos, ALIGN_LEFT);
            buttons[0].pos = SDL_Rect{0, screen_surface->h - 128, screen_surface->w, 128};
            for (int i = 1; i < buttons.size(); i += 2)
                buttons[i].pos.x = w - 80;
            for (int i = 2; i < buttons.size(); i += 2)
                buttons[i].pos.x = w - 160;
            for (auto& i : buttons) {
                i.render(screen_surface);
            }
        } else if (scene == GAME_SCENE) {
            TTTRect = SDL_Rect{0, 0, 0, 0};
            TTTRect.w = TTTRect.h = std::min(w, h);
            ((w > h) ? TTTRect.x : TTTRect.y) = std::abs(w - h) / 2;

            TTT_surface = SDL_CreateRGBSurface(0, TTTRect.w, TTTRect.h, 32, 0, 0, 0, 0);
            SDL_FillRect(TTT_surface, NULL, SDL_MapRGBA(screen_surface->format, 255, 255, 255, 0));

            transparency_surface = SDL_CreateRGBSurface(0, TTTRect.w, TTTRect.h, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

            transparency_renderer = SDL_CreateSoftwareRenderer(transparency_surface);

            std::vector<std::vector<SDL_Rect>> current_area{{{0, 0, TTTRect.w, TTTRect.h}}};
            if (rttt.firstGame()) {
                current_area = drawTTT(rttt.current_top[0][0], current_area[0][0]);
                for (auto& i : current_area)
                    for (auto& j : i)
                        drawBoundaries(j);
            } else {
                current_area = drawTTT(RTTT::TTT(rttt.current_top, rttt.last_pos), current_area[0][0]);
                int layers_index = rttt.layers.size() - 1;
                auto* top = &rttt.current_top;
                do {
                    for (unsigned i = 0; i < rttt.width; ++i) {
                        for (unsigned j = 0; j < rttt.height; ++j) {
                            if (layers_index < 0 || rttt.layers[layers_index].pos != Pos2D(i, j))
                                drawTTT((*top)[i][j], current_area[i][j], top == &rttt.current_top, Pos2D(i, j) != rttt.current_pos && !rttt.isCurrentPosInvalid());
                        }
                    }
                    if (layers_index >= 0) {
                        current_area = drawTTT((*top)[rttt.layers[layers_index].pos.x][rttt.layers[layers_index].pos.y],
                                               current_area[rttt.layers[layers_index].pos.x][rttt.layers[layers_index].pos.y], top == &rttt.current_top);
                        top = &rttt.layers[layers_index].layer;
                    }
                } while ((--layers_index) + 1 >= 0);
            }

            SDL_BlitScaled(TTT_surface, nullptr, screen_surface, &TTTRect);
            SDL_BlitScaled(transparency_surface, nullptr, screen_surface, &TTTRect);
            SDL_DestroyRenderer(transparency_renderer);
            SDL_FreeSurface(TTT_surface);
            SDL_FreeSurface(transparency_surface);
        }
        SDL_Rect reset_rect{0, 0, 48, 48};
        SDL_BlitScaled(reset_button, nullptr, screen_surface, &reset_rect);
        SDL_UpdateWindowSurface(window);
        prev_w = w;
        prev_h = h;
        redraw = false;
    }

    TTF_Font* font;
    enum Alignment { ALIGN_LEFT, ALIGN_CENTER, ALIGN_RIGHT };
    static void renderText(SDL_Surface* dst_surface, TTF_Font* font, const char* text_cstr, SDL_Color text_color, SDL_Rect dst_rect,
                           Alignment alignment = ALIGN_CENTER) {
        SDL_Surface* text = TTF_RenderText_Blended(font, text_cstr, text_color);
        SDL_Rect rect;
        if (alignment == ALIGN_LEFT) {
            rect = SDL_Rect{dst_rect.x, dst_rect.y + (dst_rect.h - text->h) / 2, dst_rect.x + text->w, dst_rect.y + text->h};
        } else if (alignment == ALIGN_CENTER) {
            rect = SDL_Rect{(dst_rect.w + dst_rect.x - text->w) / 2, (dst_rect.h + dst_rect.y - text->h) / 2, text->w, text->h};
        } else if (alignment == ALIGN_RIGHT) {
            rect = SDL_Rect{dst_rect.w + dst_rect.x - text->w, dst_rect.y, text->w, text->h};
        }
        SDL_BlitSurface(text, nullptr, dst_surface, &rect);
        SDL_FreeSurface(text);
    }
    struct Button {
        TTF_Font* font;
        SDL_Rect pos;
        SDL_Color color, text_color;
        std::string text;
        void render(SDL_Surface* surface) {
            SDL_Surface* button = SDL_CreateRGBSurface(0, pos.w, pos.h, 32, 0, 0, 0, 0);
            SDL_FillRect(button, NULL, SDL_MapRGB(button->format, color.r, color.g, color.b));
            SDL_Surface* text = TTF_RenderText_Blended(font, this->text.c_str(), text_color);
            SDL_Rect dst_rect{(pos.w - text->w) / 2, (pos.h - text->h) / 2, text->w, text->h};
            SDL_BlitSurface(text, nullptr, button, &dst_rect);
            SDL_BlitSurface(button, nullptr, surface, &pos);
        }
        std::function<void(void)> on_click;
    };
    std::vector<Button> buttons;

    void click(int64_t x, int64_t y) {
        redraw = true;
        static bool menu = false;
        if (x >= 0 && x < 48 && y >= 0 && y < 48) {
            if (menu) {
                scene = Scene(!scene);
                return;
            }
            menu = true;
            goto after_menu;
        }
        menu = false;
        after_menu:
        if (scene == MENU_SCENE) {
            for (auto& i : buttons) {
                if (x >= i.pos.x && x < i.pos.x + i.pos.w && y >= i.pos.y && y < i.pos.y + i.pos.w) {
                    i.on_click();
                }
            }
            return;
        }
        x -= TTTRect.x;
        y -= TTTRect.y;
        if (x >= TTTRect.w || y >= TTTRect.h || x < 0 || y < 0)
            return;
        if (rttt.firstGame()) {
            rttt.move(Pos2D(x / (TTTRect.w / rttt.width), y / (TTTRect.h / rttt.height)));
        } else {
            Pos2D current_pos(x / (TTTRect.w / rttt.width), y / (TTTRect.h / rttt.height));
            Pos2D move_pos((x - current_pos.x * (TTTRect.w / rttt.width)) / ((TTTRect.w / rttt.width) / rttt.width),
                           (y - current_pos.y * (TTTRect.h / rttt.height)) / ((TTTRect.h / rttt.height) / rttt.height));
            if (current_pos == rttt.current_pos || rttt.isCurrentPosInvalid()) {
                rttt.current_pos = current_pos;
                rttt.move(move_pos);
            }
        }
    }

    ~RTTT_SDL() {
        for (auto i : player_surface)
            SDL_FreeSurface(i);
        TTF_CloseFont(font);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

private:
    RTTT& rttt;
    SDL_Window* window = nullptr;
    SDL_Surface *screen_surface = nullptr, *TTT_surface = nullptr, *transparency_surface = nullptr;
    SDL_Renderer* transparency_renderer;
    SDL_DisplayMode dm;
    std::vector<SDL_Surface*> player_surface;
    SDL_Surface* reset_button;

    void drawBoundaries(SDL_Rect area, int opacity = 224) {
        SDL_SetRenderDrawColor(transparency_renderer, 0, 0, 0, opacity);
        SDL_RenderDrawRect(transparency_renderer, &area);
        --area.x;
        --area.y;
        SDL_RenderDrawRect(transparency_renderer, &area);
        area.x += 2;
        area.y += 2;
        SDL_RenderDrawRect(transparency_renderer, &area);
    }
    std::vector<std::vector<SDL_Rect>> drawTTT(const RTTT::TTT& ttt, const SDL_Rect& area, bool unavailable = false, bool not_current = false) {
        bool gray = unavailable && (not_current || ttt.gameEnded());
        if (gray) {
            SDL_FillRect(transparency_surface, &area, SDL_MapRGBA(transparency_surface->format, 0, 0, 0, 128));
        }
        std::vector<std::vector<SDL_Rect>> result(rttt.width, std::vector<SDL_Rect>(rttt.height));
        for (int i = 0; i < rttt.width; ++i) {
            for (int j = 0; j < rttt.height; ++j) {
                result[i][j] = SDL_Rect{int(area.x + area.w * i / rttt.width), int(area.y + area.h * j / rttt.height), int(area.w / rttt.width),
                                        int(area.h / rttt.height)};
                if (ttt.field[i][j])
                    SDL_BlitScaled(player_surface[ttt.field[i][j] - 1], nullptr, TTT_surface, &result[i][j]);
                if (unavailable)
                    drawBoundaries(result[i][j], 64 + gray * 128);
            }
        }
        if (unavailable) {
            drawBoundaries(area);
        }
        return result;
    }
};

#endif // RTTT_SDL_HPP
