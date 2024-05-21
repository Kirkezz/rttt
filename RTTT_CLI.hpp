#ifndef RTTT_CLI_HPP
#define RTTT_CLI_HPP

#include "RTTT.hpp"
#include <iostream>

class RTTT_CLI {
public:
    RTTT_CLI(RTTT& rttt);
    void move();
    void printBoard();

private:
    RTTT& rttt;
};

#endif // RTTT_CLI_HPP
