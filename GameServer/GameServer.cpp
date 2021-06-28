#include <iostream>
#include <SFML/Network.hpp>
#include <thread>
#include <mutex>
#include <string>
#include <vector>
#include "../GameLib/PlayerInfo.h"
using namespace std;

#define MAX_PLAYERS 2
#define COUNTDOWN 30
#define PLAYER_SIZE_COLLISION 100


GAME_STATE gamestate = GAME_STATE::PENDING;

void AddClient(vector<Client*> *clients, sf::IpAddress ip, unsigned short port)
{
	Client* newClient = new Client;
	newClient->ID = 0;
	newClient->IP = ip;
	newClient->PORT = port;
	cout << "Client " << newClient->IP << " - " << newClient->PORT << " has been added to the server" << endl;
	clients->push_back(newClient);
}

void SendChallenge(sf::UdpSocket* socket, sf::IpAddress ip, unsigned short port)
{
	sf::Packet packet;
	CMD_TYPE CMD = CMD_TYPE::CHALLENGE;
	packet << CMD;
	
	sf::Socket::Status status = socket->send(packet,ip, port);
	if (status != sf::Socket::Status::Done)
	{
			cout << "Error sending to " << ip << " : " << port << endl;;
	}
}

void SendWelcome(int id, sf::UdpSocket* socket, vector<Client*> clients)
{
	sf::Packet packet;
	CMD_TYPE CMD = CMD_TYPE::WELLCOME;
	int players = clients.size();
	packet << CMD << id << players;
	for (int i = 0; i < clients.size(); i++)
	{
		sf::Socket::Status status = socket->send(packet, clients[i]->IP, clients[i]->PORT);
		if (status != sf::Socket::Status::Done)
		{
			cout << "Error sending to " << clients[i]->IP << ":" << clients[i]->PORT << endl;
		}
	}
}

void SendKill(int id, sf::UdpSocket* socket, std::vector<Client*> clients) {
	sf::Packet packet;
	CMD_TYPE CMD = CMD_TYPE::KILL;
	packet << CMD << id;
	for (int i = 0; i < clients.size(); i++)
	{
		sf::Socket::Status status = socket->send(packet, clients[i]->IP, clients[i]->PORT); //Enviem un missatge a cada client que tenim guardat, amb la seva IP i el seu PORT
		if (status != sf::Socket::Status::Done)
		{ 
			cout << "Error sending to " << clients[i]->IP << ":" << clients[i]->PORT << std::endl;
		}
		else cout << "S'ha enviat kill client a " << clients[i]->IP << " : " << clients[i]->PORT << endl;
	}
}

void SendMoveACK(int id, int x, int y, sf::UdpSocket* socket, std::vector<Client*> clients) {
	sf::Packet packet;
	CMD_TYPE CMD = CMD_TYPE::MOVE_ACK;
	packet << CMD << id << x << y;
	for (int i = 0; i < clients.size(); i++)
	{
		sf::Socket::Status status = socket->send(packet, clients[i]->IP, clients[i]->PORT); //Enviem un missatge a cada client que tenim guardat, amb la seva IP i el seu PORT
		if (status != sf::Socket::Status::Done)
		{ 
			cout << "Error sending to " << clients[i]->IP << ":" << clients[i]->PORT << std::endl;
		}
	}
}

void SendStartGame(sf::UdpSocket* socket, std::vector<Client*> clients)
{
	gamestate = GAME_STATE::GAME;
	sf::Packet packet;
	CMD_TYPE CMD = CMD_TYPE::START;
	int countdown = COUNTDOWN;
	packet << CMD << countdown;
	for (int i = 0; i < clients.size(); i++)
	{
		sf::Socket::Status status = socket->send(packet, clients[i]->IP, clients[i]->PORT); //Enviem un missatge a cada client que tenim guardat, amb la seva IP i el seu PORT
		if (status != sf::Socket::Status::Done)
		{
			cout << "Error sending to " << clients[i]->IP << ":" << clients[i]->PORT << std::endl;
		}
	}
}

void SendPong(sf::UdpSocket* socket, std::vector<Client*> clients)
{
	sf::Packet packet;
	CMD_TYPE CMD = CMD_TYPE::PONG;
	packet << CMD;
	for (int i = 0; i < clients.size(); i++)
	{
		sf::Socket::Status status = socket->send(packet, clients[i]->IP, clients[i]->PORT); //Enviem un missatge a cada client que tenim guardat, amb la seva IP i el seu PORT
		if (status != sf::Socket::Status::Done)
		{
			cout << "Error sending to " << clients[i]->IP << ":" << clients[i]->PORT << std::endl;
		}
	}
}


bool clientExists(std::vector<Client*> clients, sf::IpAddress ip, unsigned short port)
{
	bool exists = false;
	for (int i = 0; i < clients.size(); i++)
	{
		if (clients[i]->IP == ip && clients[i]->PORT == port)
		{ 
			exists = true;
		}
	}
	return exists;
}


void RemoveClient(std::vector<Client*> *clients, int ID)
{
	clients->erase(clients->begin() + ID); 
}


int playerAvailable(std::vector<Client*> clients)
{
	bool oneAvailable, twoAvailable, threeAvailable, fourAvailable;
	oneAvailable = twoAvailable = threeAvailable = fourAvailable = true;

	for (int i = 0; i < clients.size(); i++)
	{
		if (clients[i]->ID == 1) { oneAvailable = false; }
		if (clients[i]->ID == 2) { twoAvailable = false; }
		if (clients[i]->ID == 3) { threeAvailable = false; }
		if (clients[i]->ID == 4) { fourAvailable = false; }
	}

	if (oneAvailable) { return 1; }
	if (twoAvailable) { return 2; }
	if (threeAvailable) { return 3; }
	if (fourAvailable) { return 4; }
}


void ChangeID(std::vector<Client*> clients, int id)
{
	clients[id - 1]->ID = id;
}


void checkMSG(sf::Packet packet, std::vector<Client*> clients, sf::UdpSocket* socket,sf::IpAddress senderIP, unsigned short senderPort) {

	CMD_TYPE CMD;
	packet >> CMD;

	switch (CMD) {
	case CMD_TYPE::HELLO: {
		int ID;
		packet >> ID;
		SendChallenge(socket, senderIP, senderPort);
		
		
		break;
	}
	case CMD_TYPE::CHALLENGE_R: {
		int available = playerAvailable(clients);
		std::cout << available << std::endl;
		SendWelcome(available, socket, clients);
		ChangeID(clients, available);

		break;
	}

	case CMD_TYPE::BYE: {
		int ID;
		packet >> ID;
		cout << "The ID from the player that disconnect is : "<< ID << endl;
		RemoveClient(&clients, ID - 1);
		SendKill(ID, socket, clients);
		break;
	}
	case CMD_TYPE::MOVE: {
		int ID, tempX, tempY;
		packet >> ID >> tempX >> tempY;

		//int rivalX, rivalY;

		//for (int i = 0; i < clients.size(); i++)
		//{
		//	if (clients[i]->ID == ID)
		//	{
		//		clients[i]->x = tempX;
		//		clients[i]->y = tempY;
		//	}
		//	else
		//	{
		//		rivalX = clients[i]->x;
		//		rivalY = clients[i]->y;

		//	}

		//}

		//for (int i = 0; i < clients.size(); i++)
		//{
		//	if (clients[i]->ID == ID)
		//	{
		//		if (((clients[i]->x < rivalX + PLAYER_SIZE_COLLISION) && (clients[i]->x > rivalX)) &&
		//			((clients[i]->y <= rivalY + PLAYER_SIZE_COLLISION) && (clients[i]->y > rivalY)))
		//		{
		//			cout << "COLISION" << endl;
		//		}
		//		else
		//		{
		//			//SendMoveACK(ID, tempX, tempY, socket, clients);
		//		}
		//	}
		//}		


		//cout << "ID player : " << ID << "amb valor X " << tempX << " i valor Y " << tempY << endl;
		SendMoveACK(ID, tempX, tempY, socket, clients);

		break;
	}
	case CMD_TYPE::RESTART: {
		
		cout << "Restarting the Game" << endl;
		SendStartGame(socket, clients);

		break;
	}
	case CMD_TYPE::PING: {
		int ID;
		packet >> ID;
		//cout << "Rebo PING de Player " << ID << endl;
		SendPong(socket, clients);
		break;
	}
	}
}

void printAll(std::vector<Client*> clients)
{
	for (int i = 0; i < clients.size(); i++)
	{
		cout << clients[i]->ID << " : " << clients[i]->IP << "-" << clients[i]->PORT << endl;
	}
}


int main()
{
	sf::UdpSocket sock;
	sock.setBlocking(false);
	sf::Socket::Status status = sock.bind(PORT_SERVER);
	if (status == sf::Socket::Done)
	{
		cout << "Server has started and waiting for new clients ..." << endl;
	}
	if (status == sf::Socket::Status::NotReady)
	{

	}
	if (status == sf::Socket::Status::Error)
	{
		cout << "Error binding the server socket " << endl;
	}

	vector<Client*> clients;
	bool ServerRunning = true;

	while (ServerRunning)
	{

		sf::Packet packet;
		sf::IpAddress senderIP;
		unsigned short senderPORT;
		status = sock.receive(packet, senderIP, senderPORT);
		if (status == sf::Socket::Status::NotReady)
		{
			//Waiting for clients to connect to the server
		}
		if (status == sf::Socket::Status::Error)
		{
			cout << "Error receiving from " << senderIP << "-" << senderPORT << endl;
		}
		if (status == sf::Socket::Done)
		{

			switch (gamestate)
			{
			case PENDING:

				if (clients.size() == MAX_PLAYERS)
				{
					cout << "comença el game" << endl;
					SendStartGame(&sock, clients);
					gamestate = GAME_STATE::GAME;
				}
			case GAME:
				if (!clientExists(clients, senderIP, senderPORT))
				{
					if (clients.size() > MAX_PLAYERS)
					{
						cout << "WE HAVE REACHED THE MAX AMOUNT OF PLAYERS" << endl;
					}
					else
					{
						AddClient(&clients, senderIP, senderPORT);
					}

				}
				checkMSG(packet, clients, &sock, senderIP, senderPORT);
				//printAll(clients);

			case GAMEOVER:
				break;
			default:
				break;
			}
		}
	}
}