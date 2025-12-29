#include "Game.hpp"
#include <iostream>
#include <exception>

int main() {
    try {
        Game game;
        game.run();
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
