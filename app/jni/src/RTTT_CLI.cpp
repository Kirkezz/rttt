#include "RTTT_CLI.hpp"

RTTT_CLI::RTTT_CLI(RTTT& rttt) : rttt(rttt) {}

void RTTT_CLI::move() {
    Pos2D pos;
    std::cout << "Enter the move coordinates (sample: 0 1): ";
    std::cin >> pos.x >> pos.y;
    RTTT::MoveErrorCode result;
    while (result = rttt.move(pos)) {
        switch (result) {
        case RTTT::INVALID_CURRENT_POS:
            std::cout << "Enter the square you want to play in (sample: 0 1): ";
            std::cin >> rttt.current_pos.x >> rttt.current_pos.y;
            break;
        case RTTT::INVALID_POS:
            std::cout << "Enter the move coordinates (sample: 0 1): ";
            std::cin >> pos.x >> pos.y;
            break;
        }
    }
}

void RTTT_CLI::printBoard() {
    for (auto& i : rttt.current_top) {
        for (unsigned j = 0; j < rttt.width; ++j) {
            for (auto& c : i) {
                for (auto w : c.field[j]) {
                    std::cout << (int) w;
                }
                std::cout << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
}
