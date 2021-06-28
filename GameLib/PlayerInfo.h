#pragma once
#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <string>
#include <thread>
#include <mutex>
#include <cstring>
#include <vector>
#include <list>

#define PLAYER_SIZE 50



sf::IpAddress IP_SERVER = sf::IpAddress::getLocalAddress();
unsigned short PORT_SERVER = 50000;

int WINDOW_WIDTH = 800;
int WINDOW_HEIGHT = 600;

enum GAME_STATE
{
	PENDING,
	GAME,
	GAMEOVER
};

enum CMD_TYPE
{
	HELLO,
	WELLCOME,
	CHALLENGE,
	CHALLENGE_R,
	MOVE,
	MOVE_ACK,
	PING,
	PONG,
	KILL,
	END,
	START,
	RESTART,
	ACK,
	BYE,
	EMPTY
};

struct Client
{
public:
	int ID;
	sf::IpAddress IP;
	unsigned short PORT;
	int x;
	int y;
};

struct Player
{
public:
	int ID;
	int x;
	int y;
	int speed = 4;
	bool alive;
	sf::Color Color;
};


//Overload send and recieve function for packet to recieve an enumerator
inline
sf::Packet& operator << (sf::Packet& packet, CMD_TYPE& CMD) { return packet << (int)CMD; }

inline
sf::Packet& operator >> (sf::Packet& packet, CMD_TYPE& CMD) {
	int id;
	auto info = packet >> id;
	CMD = (CMD_TYPE)id;
	return info;
}
