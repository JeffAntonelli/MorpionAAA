#pragma once
#include <SFML/System/Vector2.hpp>

namespace morpion
{
constexpr int maxClientNmb = 2;
constexpr unsigned short serverPortNumber = 12345;
enum class MorpionPhase
{
    CONNECTION,
    GAME,
    END
};

using PlayerNumber = unsigned char;
struct Move
{
    sf::Vector2i position;
    PlayerNumber playerNumber;
};
}
