// main.cpp
#include <GameApp/Init.hpp>
#include <string>
#include <iostream>
\
int main(int argc, char *argv[])
{
    float longtitude = 50.86501694f;
    float latitude = 16.70881694f;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--start") {
            if (i + 2 < argc) {
                try {
                    longtitude = std::stod(argv[i + 1]);
                    latitude = std::stod(argv[i + 2]);
                    i += 2;
                } catch (const std::exception& e) {
                    std::cerr << "Błąd: Parametry --start muszą być liczbami (double)." << std::endl;
                    return 1;
                }
            } else {
                std::cerr << "Błąd: Flaga --start wymaga podania dwóch wartości: {longitude} {latitude}." << std::endl;
                return 1;
            }
        }
    }

    GameApp::init(longtitude, latitude);
    return 0;
}
