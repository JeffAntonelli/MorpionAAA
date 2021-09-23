#include "morpion_server.h"
#include <SFML/Network/TcpSocket.hpp>

#include <iostream>
#include <random>

#include "morpion_packet.h"

namespace morpion
{
    void MorpionServer::ReceivePacket()
    {
        if (selector_.wait(sf::milliseconds(20)))
        {
            for (auto& socket : sockets_)
            {
                if (selector_.isReady(socket))
                {
                    sf::Packet receivedPacket;
                    sf::Socket::Status receivingStatus;
                    do
                    {
                        receivingStatus = socket.receive(receivedPacket);
                    } while (receivingStatus == sf::Socket::Partial);

                    Packet statusPacket;
                    receivedPacket >> statusPacket;
                    switch (static_cast<PacketType>(statusPacket.packetType))
                    {
                    case PacketType::MOVE:
                    {
                        MovePacket movePacket;
                        receivedPacket >> movePacket;
                        ManageMovePacket(movePacket);
                        break;
                    }
                    }

                }
            }
        }
    }

    PlayerNumber MorpionServer::CheckWinner() const
    {
        std::array<std::array<PlayerNumber, 3>, 3> board{};
        std::ranges::for_each(board, [](auto& line)
        {
            line.fill(255u);
        });
        for(unsigned i = 0; i < currentMoveIndex_; i++)
        {
            const auto& move = moves_[i];
            board[move.position.x][move.position.y] = move.playerNumber;
        }
        //Line
        for(unsigned i = 0; i < 3; i++)
        {
            PlayerNumber firstTile = board[i][0];
            bool victory = true;
            for(unsigned j = 1; j < 3; j++)
            {
                if (firstTile != board[i][j])
                {
                    victory = false;
                    break;
                }
            }
            if(victory && firstTile != 255u)
            {
                return firstTile;
            }
        }
        //Column
        for (unsigned i = 0; i < 3; i++)
        {
            PlayerNumber firstTile = board[0][i];
            bool victory = true;
            for (unsigned j = 1; j < 3; j++)
            {
                if (firstTile != board[j][i])
                {
                    victory = false;
                    break;
                }
            }
            if (victory && firstTile != 255u)
            {
                return firstTile;
            }
        }
        //First diagonal
        {
            PlayerNumber firstTile = board[0][0];
            bool victory = true;
            for (unsigned i = 1; i < 3; i++)
            {
                if (firstTile != board[i][i])
                {
                    victory = false;
                    break;
                }
            }
            if (victory && firstTile != 255u)
            {
                return firstTile;
            }
        }
        {
            const std::array diagonal = {
                sf::Vector2i(2,0),
                sf::Vector2i(1,1),
                sf::Vector2i(0,2)
            };
            PlayerNumber firstTile = board[diagonal[0].x][diagonal[0].y];
            bool victory = true;
            for (unsigned i = 1; i < 3; i++)
            {
                if (firstTile != board[diagonal[i].x][diagonal[i].y])
                {
                    victory = false;
                    break;
                }
            }
            if (victory && firstTile != 255u)
            {
                return firstTile;
            }
        }
        return 255u;
    }

    void MorpionServer::ManageMovePacket(const MovePacket& movePacket)
    {
        std::cout << "Player " << movePacket.playerNumber + 1 <<
            " made move " << movePacket.position.x << ',' <<
            movePacket.position.y << '\n';

        if (phase_ != MorpionPhase::GAME)
            return;

        if(currentMoveIndex_ % 2 != movePacket.playerNumber)
        {
            //TODO return to player an error msg
            return;
        }

        if(movePacket.position.x > 2 || movePacket.position.y > 2)
        {
            return;
        }

        for(unsigned char i = 0; i < currentMoveIndex_; i++)
        {
            if(moves_[i].position == movePacket.position)
                //TODO return an error msg
                return;
        }

        auto& currentMove = moves_[currentMoveIndex_];
        currentMove.position = movePacket.position;
        currentMove.playerNumber = movePacket.playerNumber;
        currentMoveIndex_++;
        EndType endType = EndType::NONE;
        if(currentMoveIndex_ == 9)
        {
            //TODO end of game
            endType = EndType::STALEMATE;
        }
        //TODO check victory condition
        PlayerNumber winningPlayer = CheckWinner();
        if(winningPlayer != 255u)
        {
            endType = winningPlayer ? EndType::WIN_P2 : EndType::WIN_P1;
        }
        //TODO send end of game packet
        if(endType != EndType::NONE)
        {
            EndPacket endPacket{};
            endPacket.packetType = PacketType::END;
            endPacket.endType = endType;

            //sent new move to all players
            for (auto& socket : sockets_)
            {
                sf::Packet sentPacket;
                sentPacket << endPacket;
                sf::Socket::Status sentStatus;
                do
                {
                    sentStatus = socket.send(sentPacket);
                } while (sentStatus == sf::Socket::Partial);

            }

            phase_ = MorpionPhase::END;
        }
        MovePacket newMovePacket = movePacket;
        newMovePacket.packetType = PacketType::MOVE;

        //sent new move to all players
        for(auto& socket: sockets_)
        {
            sf::Packet sentPacket;
            sentPacket << newMovePacket;
            sf::Socket::Status sentStatus;
            do
            {
                sentStatus = socket.send(sentPacket);
            } while (sentStatus == sf::Socket::Partial);
            
        }
    }

    int MorpionServer::Run()
    {
        if (listener_.listen(serverPortNumber) != sf::Socket::Done)
        {
            std::cerr << "[Error] Server cannot bind port: " << serverPortNumber << '\n';
            return EXIT_FAILURE;
        }
        std::cout << "Server bound to port " << serverPortNumber << '\n';

        while (true)
        {
            switch (phase_)
            {
            case MorpionPhase::CONNECTION:
                ReceivePacket();
                UpdateConnectionPhase();
                break;
            case MorpionPhase::GAME:
                ReceivePacket();
                break;
            case MorpionPhase::END:
                return EXIT_SUCCESS;
            default:;
            }
        }
    }

    void MorpionServer::StartNewGame()
    {
        //Switch to Game state
        phase_ = MorpionPhase::GAME;
        //Send game init packet
        std::cout << "Two players connected!\n";

        std::default_random_engine generator;
        std::uniform_int_distribution<int> distribution(0, 1);
        int dice_roll = distribution(generator);

        for (unsigned char i = 0; i < sockets_.size(); i++)
        {
            GameInitPacket gameInitPacket{};
            gameInitPacket.packetType = PacketType::GAME_INIT;
            gameInitPacket.playerNumber = i != dice_roll;
            sf::Packet sentPacket;
            sentPacket << gameInitPacket;
            sf::Socket::Status sentStatus;
            do
            {
                sentStatus = sockets_[i].send(sentPacket);
            } while (sentStatus == sf::Socket::Partial);
        }
    }

    void MorpionServer::UpdateConnectionPhase()
    {
        // accept a new connection
        const auto nextIndex = GetNextSocket();

        if (nextIndex != -1)
        {
            auto& newSocket = sockets_[nextIndex];
            if (listener_.accept(newSocket) == sf::Socket::Done)
            {
                std::cout << "New connection from " <<
                    newSocket.getRemoteAddress().toString() << ':' << newSocket.
                    getRemotePort() << '\n';
                newSocket.setBlocking(false);
                selector_.add(newSocket);
                if (nextIndex == 1)
                {
                    StartNewGame();

                }
            }
        }
    }

    int MorpionServer::GetNextSocket()
    {
        for (int i = 0; i < maxClientNmb; i++)
        {
            if (sockets_[i].getLocalPort() == 0)
            {
                return i;
            }
        }
        return -1;
    }
}
