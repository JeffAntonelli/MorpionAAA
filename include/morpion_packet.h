#pragma once

#include <morpion_settings.h>
#include <SFML/Network/Packet.hpp>
#include <SFML/System/Vector2.hpp>

namespace morpion
{
    enum class PacketType : unsigned char
    {
        GAME_INIT,
        MOVE,
        END
    };

    struct Packet
    {
        PacketType packetType;
    };

    inline sf::Packet& operator <<(sf::Packet& packet, const Packet& morpionPacket)
    {
        const auto packetType = static_cast<unsigned char>(morpionPacket.packetType);
        return packet << packetType;
    }

    inline sf::Packet& operator >>(sf::Packet& packet, Packet& morpionPacket)
    {
        unsigned char packetType;
        packet >> packetType;
        morpionPacket.packetType = static_cast<PacketType>(packetType);
        return packet  ;
    }

    struct GameInitPacket : Packet
    {
        PlayerNumber playerNumber;
    };

    inline sf::Packet& operator <<(sf::Packet& packet, const GameInitPacket& gameInitPacket)
    {
        return packet << static_cast<unsigned char>(gameInitPacket.packetType)
        << gameInitPacket.playerNumber;
    }

    inline sf::Packet& operator >>(sf::Packet& packet, GameInitPacket& gameInitPacket)
    {
        return packet >> gameInitPacket.playerNumber;
    }

    struct MovePacket : Packet
    {
        sf::Vector2i position;
        PlayerNumber playerNumber;
    };

    inline sf::Packet& operator <<(sf::Packet& packet, const MovePacket& movePacket)
    {
        return packet << static_cast<unsigned char>(movePacket.packetType)
            << movePacket.position.x
            << movePacket.position.y
            << movePacket.playerNumber;
    }

    inline sf::Packet& operator >>(sf::Packet& packet, MovePacket& movePacket)
    {
        return packet
            >> movePacket.position.x
            >> movePacket.position.y
            >> movePacket.playerNumber;
    }

    enum class EndType : unsigned char
    {
        NONE,
        STALEMATE,
        WIN_P1,
        WIN_P2,
        ERROR
    };

    struct EndPacket : Packet
    {
        EndType endType;
    };

    inline sf::Packet& operator <<(sf::Packet& packet, const EndPacket& endPacket)
    {
        const auto endType = static_cast<unsigned char>(endPacket.endType);
        return packet << static_cast<unsigned char>(endPacket.packetType)
            << endType;
    }

    inline sf::Packet& operator >>(sf::Packet& packet, EndPacket& endPacket)
    {
        unsigned char endType = 0;
        packet >> endType;
        endPacket.endType = static_cast<EndType>(endType);
        return packet;
    }
}
