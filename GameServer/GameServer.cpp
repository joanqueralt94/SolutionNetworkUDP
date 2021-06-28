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
#define IDCritPacketWelcome 12

int idCritPacket = 0;
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

void SendStartGame(sf::UdpSocket* socket, std::vector<Client*> clients, std::vector<critPack*> *critPackets)
{
	gamestate = GAME_STATE::GAME;
	sf::Packet packet;
	CMD_TYPE CMD = CMD_TYPE::START;
	int countdown = COUNTDOWN;
	
	for (int i = 0; i < clients.size(); i++)
	{
		idCritPacket = idCritPacket + i;
		packet << CMD << countdown << idCritPacket;
		critPack* tempCritPacket = new critPack;
		tempCritPacket->ID = idCritPacket;
		tempCritPacket->packet = packet;
		critPackets->push_back(tempCritPacket);

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


void checkMSG(sf::Packet packet, std::vector<Client*> clients, sf::UdpSocket* socket,sf::IpAddress senderIP, unsigned short senderPort, float* counter1, float* counter2, std::vector<critPack*> *critPackets) {
	
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
		cout << "The ID from the player that disconnect is : " << ID << endl;
		RemoveClient(&clients, ID - 1);
		SendKill(ID, socket, clients);
		break;
	}
	case CMD_TYPE::MOVE: {
		int ID, tempX, tempY;
		packet >> ID >> tempX >> tempY;

		bool colX = false;
		bool colY = false;

		int rivalX, rivalY;

		for (int i = 0; i < clients.size(); i++)
		{
			if (clients[i]->ID == ID)
			{
				clients[i]->x = tempX;
				clients[i]->y = tempY;
			}
			else
			{
				rivalX = clients[i]->x;
				rivalY = clients[i]->y;

			}

		}

		for (int i = 0; i < clients.size(); i++)
		{
			if (clients[i]->ID == ID)
			{
				if ((clients[i]->x < rivalX + PLAYER_SIZE_COLLISION) && (clients[i]->x > rivalX))
				{
					//cout << "COLLISION on X" << endl;
					colX = true;
				}
				else colX = false;

				if ((clients[i]->y <= rivalY + PLAYER_SIZE_COLLISION) && (clients[i]->y > rivalY))
				{
					//cout << "COLISION on Y" << endl;
					colY = true;
				}
				else colY = false;

			}
		}

		if (colX && colY)
		{
			cout << "Colision Total" << endl;
		}
		else
		{
			SendMoveACK(ID, tempX, tempY, socket, clients);
		}
		//cout << "ID player : " << ID << "amb valor X " << tempX << " i valor Y " << tempY << endl;
		//SendMoveACK(ID, tempX, tempY, socket, clients);

		break;
	}
	case CMD_TYPE::RESTART: {

		cout << "Restarting the Game" << endl;
		SendStartGame(socket, clients, critPackets);

		break;
	}
	case CMD_TYPE::PING: {
		int ID;
		packet >> ID;

		if (ID == 1)
		{
			cout << "RTT from Player 1 is: " << *counter1 << " ms" << endl;
			//if (*counter1 > 1000) SendKill(ID, socket, clients);

			*counter1 = 0;
		}
		else if (ID == 2)
		{
			cout << "RTT from Player 2 is: " << *counter2 << " ms" << endl;
			//if (*counter2 > 1000) SendKill(ID, socket, clients);
			*counter2 = 0;
		}
		//cout << "Rebo PING de Player " << ID << endl;
		SendPong(socket, clients);
		break;
	}
	case CMD_TYPE::ACK:
	{
		sf::Clock critClock;
		sf::Packet packet;
		int idTempCrit;
		packet >> idTempCrit;
		
		int idCrit = IDCritPacketWelcome;
		for (int i = 0; i < critPackets->size(); i++)
		{
			if (idTempCrit == idCrit)
			{
				cout << "S'ha deliminar això" << endl;
				critPackets[i].pop_back();
			}

		}
		if (critClock.getElapsedTime().asSeconds() >= 5)
		{
			if (critPackets->size() > 0)
			{
				SendStartGame(socket, clients, critPackets);
			}
		}
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
	vector<critPack*> critPackets;
	bool ServerRunning = true;

	sf::Clock pingClock1;
	sf::Clock pingClock2;


	float counterPing1 = 0;
	float counterPing2 = 0;

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
					SendStartGame(&sock, clients, &critPackets);
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

				counterPing1 = pingClock1.getElapsedTime().asMilliseconds();
				counterPing2 = pingClock2.getElapsedTime().asMilliseconds();

				pingClock1.restart();
				pingClock2.restart();

				checkMSG(packet, clients, &sock, senderIP, senderPORT, &counterPing1, &counterPing2, &critPackets);



				
				//printAll(clients);


			case GAMEOVER:
				break;
			default:
				break;
			}
		}
	}
}