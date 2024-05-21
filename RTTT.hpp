#ifndef RTTT_HPP
#define RTTT_HPP

#include "inttypes.h"
#include <algorithm>
#include <iostream>
#include <list>
#include <vector>

struct Pos2D {
    Pos2D(unsigned x = 0, unsigned y = 0) : x(x), y(y) {}
    unsigned x, y;
    bool operator==(const Pos2D &rhs) const { return x == rhs.x && y == rhs.y; }
};

struct RTTT {
    bool game_over = false;
    RTTT(unsigned width, unsigned height, unsigned win_condition, unsigned players_n)
        : width(width), height(height), win_condition(win_condition), players_n(players_n), current_top{{TTT(width, height, win_condition)}} {}

    unsigned width, height, win_condition, players_n;
    static constexpr unsigned players_n_max = 4;
    uint8_t player_to_move = 1;
    struct TTT {
        TTT() = default;
        TTT(unsigned width, unsigned height, unsigned win_condition) : width(width), height(height), win_condition(win_condition) {
            field = std::vector<std::vector<uint8_t>>(width, std::vector<uint8_t>(height, 0));
            empty_cells = width * height;
        }
        TTT(std::vector<std::vector<RTTT::TTT>> &top, Pos2D pos) : TTT(top.size(), top[0].size(), top[0][0].win_condition) {
            for (unsigned i = 0; i < top.size(); ++i) {
                for (unsigned j = 0; j < top[i].size(); ++j) {
                    field[i][j] = top[i][j].game_state;
                    if (top[i][j].gameEnded())
                        --empty_cells;
                }
            }
            game_state = updateGameState(pos);
        }

        uint8_t game_state{};
        unsigned empty_cells, width, height, win_condition;
        std::vector<std::vector<uint8_t>> field;

        void move(Pos2D pos, uint8_t player) {
            field[pos.x][pos.y] = player;
            --empty_cells;
            game_state = updateGameState(pos);
        }
        [[nodiscard]] uint8_t updateGameState(Pos2D pos) {
            uint8_t team{};
            unsigned times = 0u;
            // horizontally
            for (unsigned x = 0; x < width; ++x) {
                if (team != field[x][pos.y]) {
                    times = 0;
                    team = field[x][pos.y];
                }
                if (team != 0 && times >= win_condition - 1) {
                    return team;
                }
                ++times;
            }
            // vertically
            team = times = 0;
            for (unsigned y = 0; y < height; ++y) {
                if (team != field[pos.x][y]) {
                    times = 0;
                    team = field[pos.x][y];
                }
                if (team != 0 && times >= win_condition - 1) {
                    return team;
                }
                ++times;
            }
            // diagonally 1
            int fromX = 0, fromY = 0;
            (pos.x > pos.y ? fromX : fromY) = (pos.x > pos.y ? (pos.x - pos.y) : (pos.y - pos.x));
            team = times = 0;
            for (int xy = 0; fromX + xy < width && fromY + xy < height; ++xy) {
                if (team != field[fromX + xy][fromY + xy]) {
                    times = 0;
                    team = field[fromX + xy][fromY + xy];
                }
                if (team != 0 && times >= win_condition - 1) {
                    return team;
                }
                ++times;
            }
            // diagonally 2
            int N = std::min(width - pos.x - 1, pos.y);
            fromX = pos.x + N;
            fromY = pos.y - N;
            team = times = 0;
            for (int xy = 0; fromX >= xy && fromY + xy < height; ++xy) {
                if (team != field[fromX - xy][fromY + xy]) {
                    times = 0;
                    team = field[fromX - xy][fromY + xy];
                }
                if (team != 0 && times >= win_condition - 1) {
                    return team;
                }
                ++times;
            }
            return 0;
        }
        bool gameEnded() const { return (game_state == 0 && empty_cells == 0) || game_state != 0; }
    };
    Pos2D last_pos, current_pos = Pos2D(0, 0);
    struct Layer {
        std::vector<std::vector<TTT>> layer;
        Pos2D pos;
    };
    std::vector<Layer> layers;
    std::vector<std::vector<TTT>> current_top;

    bool firstGame() { return current_top.size() == 1 && current_top[0].size() == 1; }

    enum MoveErrorCode : uint8_t { OK = 0, INVALID_POS, INVALID_CURRENT_POS };
    MoveErrorCode move(Pos2D pos) {
        if (isCurrentPosInvalid()) {
            return INVALID_CURRENT_POS;
        }
        if (pos.x >= width || pos.y >= height || current_top[current_pos.x][current_pos.y].field[pos.x][pos.y] != 0) {
            return INVALID_POS;
        }
        auto &top = current_top[current_pos.x][current_pos.y];
        top.move(pos, player_to_move);
        player_to_move = (player_to_move + 1) % (players_n + 1);
        player_to_move += (player_to_move == 0);
        if (firstGame() ? top.gameEnded() : TTT(current_top, current_pos).gameEnded()) {
            std::vector<std::vector<RTTT::TTT>> new_top(width, std::vector<RTTT::TTT>(height, TTT(width, height, win_condition)));
            if (firstGame()) {
                new_top[pos.x][pos.y] = std::move(top);
            } else {
                new_top[pos.x][pos.y] = TTT(current_top, current_pos);
                layers.push_back({std::move(current_top), pos});
            }
            current_top = std::move(new_top);
        }
        if (!firstGame()) {
            last_pos = current_pos;
            current_pos = pos;
        }
        return OK;
    }
    bool isCurrentPosInvalid() {
        return current_pos.x >= current_top.size() || current_pos.y >= current_top[0].size() || current_top[current_pos.x][current_pos.y].gameEnded();
    }
};

#endif // RTTT_HPP
