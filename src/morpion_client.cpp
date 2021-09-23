#include "morpion_client.h"

#include <array>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <iostream>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Network/Packet.hpp>
#include <SFML/Network/Socket.hpp>

#include "morpion_packet.h"

namespace morpion
{
sf::Socket::Status MorpionClient::Connect(sf::IpAddress address, unsigned short portNumber)
{
    const auto status = socket_.connect(address, portNumber);
    socket_.setBlocking(false);
    return status;
}

MorpionPhase MorpionClient::GetPhase() const
{
    return phase_;
}

bool MorpionClient::IsConnected() const
{
    return socket_.getLocalPort() != 0;
}

void MorpionClient::Init()
{
}

void MorpionClient::ReceivePacket(sf::Packet& packet)
{
    Packet morpionPacket{};
    packet >> morpionPacket;
    
    switch (static_cast<PacketType>(morpionPacket.packetType))
    {
    case PacketType::GAME_INIT:
    {
        GameInitPacket gameInitPacket{};
        packet >> gameInitPacket;
        playerNumber_ = gameInitPacket.playerNumber;
        phase_ = MorpionPhase::GAME;
        std::cout << "You are player " << gameInitPacket.playerNumber + 1 << '\n';
        break;
    }
    case PacketType::MOVE:
    {
        if (phase_ != MorpionPhase::GAME)
            break;
        MovePacket movePacket;
        packet >> movePacket;
        std::cout << "Receive move packet from player " << 
            movePacket.playerNumber + 1 << " with position: "<< movePacket.position.x<<','<<movePacket.position.y << '\n';
        auto& currentMove = moves_[currentMoveIndex_];
        currentMove.position = movePacket.position;
        currentMove.playerNumber = movePacket.playerNumber;
        currentMoveIndex_++;
        break;
    }
    case PacketType::END:
    {
        if(phase_ != MorpionPhase::GAME)
        {
            break;
        }
        EndPacket endPacket;
        packet >> endPacket;
        switch (endPacket.endType)
        {
        case EndType::STALEMATE: 
            endMessage_ = "Stalemate";
            break;
        case EndType::WIN_P1:
            endMessage_ = playerNumber_ == 0 ? "You won" : "You lost";
            break;
        case EndType::WIN_P2: 
            endMessage_ = playerNumber_ == 1 ? "You won" : "You lost";
            break;
        case EndType::ERROR: 
            endMessage_ = "Error";
            break;
        default: ;
        }
        phase_ = MorpionPhase::END;
        break;
    }
    default: 
        break;
    }
}

void MorpionClient::Update()
{
    //Receive packetS
    if(socket_.getLocalPort() != 0)
    {
        sf::Packet receivedPacket;
        sf::Socket::Status status;
        do
        {
            status = socket_.receive(receivedPacket);
        } while (status == sf::Socket::Partial);

        if (status == sf::Socket::Done)
        {
            ReceivePacket(receivedPacket);
        }

        if (status == sf::Socket::Disconnected)
        {
            socket_.disconnect();
            std::cerr << "Server disconnected\n";
        }
    }
}

void MorpionClient::Destroy()
{
}

int MorpionClient::GetPlayerNumber() const
{
    return playerNumber_;
}

void MorpionClient::SendNewMove(sf::Vector2i position)
{
    MovePacket movePacket;
    movePacket.packetType = PacketType::MOVE;
    movePacket.position = position;
    movePacket.playerNumber = playerNumber_;
    sf::Packet packet;
    packet << movePacket;
    sf::Socket::Status sentStatus;
    do
    {
        sentStatus = socket_.send(packet);
    } while (sentStatus == sf::Socket::Partial);
}

const std::array<Move, 9>& MorpionClient::GetMoves() const
{
    return moves_;
}

unsigned char MorpionClient::GetMoveIndex() const
{
    return currentMoveIndex_;
}

std::string_view MorpionClient::GetEndMessage() const
{
    return endMessage_;
}

MorpionView::MorpionView(MorpionClient& client) : client_(client)
{
}

void MorpionView::DrawImGui()
{
    switch(client_.GetPhase())
    {
    case MorpionPhase::CONNECTION:
    {
        if (client_.IsConnected())
            return;
        ImGui::Begin("Client");

        ImGui::InputText("Ip Address", &ipAddressBuffer_);
        ImGui::InputInt("Port Number", &portNumber_);
        if (ImGui::Button("Connect"))
        {
            const auto status = client_.Connect(sf::IpAddress(ipAddressBuffer_), portNumber_);
            if (status != sf::Socket::Done)
            {
                switch (status)
                {
                case sf::Socket::NotReady:
                    std::cerr << "Not ready to connect to " << ipAddressBuffer_ << ':' << portNumber_ << '\n';
                    break;
                case sf::Socket::Partial:
                    std::cerr << "Connecting to " << ipAddressBuffer_ << ':' << portNumber_ << '\n';
                    break;
                case sf::Socket::Disconnected:
                    std::cerr << "Disconnecting to " << ipAddressBuffer_ << ':' << portNumber_ << '\n';
                    break;
                case sf::Socket::Error:
                    std::cerr << "Error connecting to " << ipAddressBuffer_ << ':' << portNumber_ << '\n';
                    break;
                default:;
                }
            }
            else
            {
                std::cout << "Successfully connected to server\n";
            }

        }
        ImGui::End();
        break;
    }
    case MorpionPhase::GAME:
    {
        const auto playerNumber = client_.GetPlayerNumber();
        ImGui::Begin("Client");
        ImGui::Text("You are player %d", playerNumber + 1);

        std::array<char, 10> board;
        board.fill(' ');
        board[9] = 0;
        const auto& moves = client_.GetMoves();
        for (unsigned char i = 0; i < client_.GetMoveIndex(); i++)
        {
            const auto& move = moves[i];
            board[move.position.y * 3 + move.position.x] = move.playerNumber ? 'X' : 'O';
        }
        ImGui::Text("%s", board.data());


        ImGui::InputInt2("New Move", currentPosition_.data());
        if (client_.GetMoveIndex() % 2 == playerNumber)
        {
            if (ImGui::Button("Send"))
            {
                client_.SendNewMove(sf::Vector2i(currentPosition_[0], currentPosition_[1]));
            }
        }
        ImGui::End();
        break;
    }
    case MorpionPhase::END:
    {
        ImGui::Begin("Client");
        ImGui::Text("%s", client_.GetEndMessage().data());
        ImGui::End();
        break;
    }
    default: ;
    }
    

    
}

void MorpionView::Draw(sf::RenderWindow& window)
{
    if(windowSize_.x == 0)
    {
        Resize(sf::Vector2i(window.getSize()));
    }
    DrawBoard(window);
    const auto& moves = client_.GetMoves();
    for(unsigned i = 0; i < client_.GetMoveIndex(); i++)
    {
        DrawMove(window, moves[i]);
    }
}

void MorpionView::OnEvent(const sf::Event& event)
{
    if(event.type == sf::Event::Resized)
    {
        Resize(sf::Vector2i(
            event.size.width,
            event.size.height ));
    }
}

void MorpionView::DrawBoard(sf::RenderWindow& window)
{
    for(int i = 0; i < 2; i++)
    {
        rect_ = sf::RectangleShape();
        rect_.setFillColor(sf::Color::White);
        rect_.setPosition(sf::Vector2f(
            boardOrigin_.x, 
            boardOrigin_.y + tileSize_.y * (i + 1)));
        rect_.setSize(sf::Vector2f(
            boardWindowSize_.x, 
            thickness));
        rect_.setOrigin(sf::Vector2f(0, thickness / 2));
        window.draw(rect_);
    }
    for (int j = 0; j < 2; j++)
    {
        rect_ = sf::RectangleShape();
        rect_.setFillColor(sf::Color::White);
        rect_.setPosition(sf::Vector2f(
            boardOrigin_.x+tileSize_.x*(j+1), 
            boardOrigin_.y));
        rect_.setSize(sf::Vector2f(
            thickness, 
            boardWindowSize_.y));
        rect_.setOrigin(sf::Vector2f(thickness / 2, 0));
        window.draw(rect_);
    }
}

void MorpionView::DrawMove(sf::RenderWindow& window, const Move& move)
{
    if(move.playerNumber == 0)
    {
        DrawCross(window, move.position);
    }
    else
    {
        DrawCircle(window, move.position);
    }
}

void MorpionView::DrawCircle(sf::RenderWindow& window, sf::Vector2i pos)
{
    circle_.setFillColor(sf::Color::Transparent);
    circle_.setOutlineColor(sf::Color::White);
    circle_.setOutlineThickness(thickness);
    const auto radius = tileSize_.x / 2 - 2 * thickness;
    circle_.setRadius(radius);
    circle_.setOrigin(sf::Vector2f(radius-2*thickness, radius-2*thickness));
    circle_.setPosition(sf::Vector2f(
        boardOrigin_.x+tileSize_.x*pos.x+radius,
        boardOrigin_.y+tileSize_.y*pos.y+radius
    ));
    window.draw(circle_);
}

void MorpionView::DrawCross(sf::RenderWindow& window, sf::Vector2i pos)
{
    rect_.setFillColor(sf::Color::White);
    rect_.setSize(sf::Vector2f(tileSize_.x, thickness));
    rect_.setOrigin(sf::Vector2f(tileSize_.x / 2, thickness / 2));
    rect_.setPosition(sf::Vector2f(
        boardOrigin_.x+tileSize_.x*pos.x+tileSize_.x/2,
        boardOrigin_.y+tileSize_.y*pos.y+tileSize_.y/2
    ));
    rect_.setRotation(45.0f);
    window.draw(rect_);
    rect_.setRotation(-45.0f);
    window.draw(rect_);
}

void MorpionView::Resize(sf::Vector2i newWindowSize)
{
    windowSize_ = newWindowSize;
    const auto minValue = std::min(windowSize_.x, windowSize_.y);
    boardWindowSize_ = { minValue, minValue };
    boardOrigin_ = {
        (windowSize_.x - boardWindowSize_.x) / 2,
        (windowSize_.y - boardWindowSize_.y) / 2
    };
    tileSize_ = {
        (boardWindowSize_.x - 2 * thickness) / 3,
        (boardWindowSize_.y - 2 * thickness) / 3,
    };
}
}
