#pragma once

#include <array>
#include <SFML/Network/SocketSelector.hpp>
#include <SFML/Network/TcpListener.hpp>
#include <SFML/Network/TcpSocket.hpp>
#include <morpion_settings.h>

#include "morpion_packet.h"

namespace morpion
{

class MorpionServer
{
public:
    int Run();
private:
    std::array<sf::TcpSocket, maxClientNmb> sockets_;
    std::array<Move, 9> moves_{};
    sf::SocketSelector selector_;
    sf::TcpListener listener_;
    MorpionPhase phase_ = MorpionPhase::CONNECTION;
    unsigned char currentMoveIndex_ = 0;

    void StartNewGame();
    void UpdateConnectionPhase();
    void ReceivePacket();
    PlayerNumber CheckWinner() const;
    void ManageMovePacket(const MovePacket& movePacket);

    int GetNextSocket();
};
}
