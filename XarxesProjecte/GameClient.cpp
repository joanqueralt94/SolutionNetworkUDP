#include "../GameLib/PlayerInfo.h"
#include <SFML\Network.hpp>
#include <SFML\Graphics.hpp>
#include <string>
#include <thread>
#include <mutex>
#include <cstring>
#include <iostream>
#include <vector>
#include <list>
using namespace std;

#define COUNTDOWN 30


int player = 0; //Variable per indiciar el jugador
int countdown = COUNTDOWN;
bool isWelcomed = false;

bool gameStarted = false;
bool gameRestart = false;

void SendHello(sf::UdpSocket *socket)
{
	sf::Packet packet;
	CMD_TYPE CMD = CMD_TYPE::HELLO;
	packet << CMD << player;
	sf::Socket::Status status = socket->send(packet, IP_SERVER, PORT_SERVER);
	if (status != sf::Socket::Status::Done)
	{
		cout << "Error sending Hello server" << endl;
	}
	//else { cout << "Correctly sent a Hello Packet" << endl; }
}

void SendChallengeR(sf::UdpSocket *socket)
{
	sf::Packet packet;
	CMD_TYPE CMD = CMD_TYPE::CHALLENGE_R;
	packet << CMD;
	sf::Socket::Status status = socket->send(packet, IP_SERVER, PORT_SERVER);
	if (status != sf::Socket::Status::Done)
	{
		cout << "Error sending ChallengeR to server" << endl;
	}
}


void SendBye(sf::UdpSocket *socket)
{
	sf::Packet packet;
	CMD_TYPE CMD = CMD_TYPE::BYE;
	packet << CMD << player;
	sf::Socket::Status status = socket->send(packet, IP_SERVER, PORT_SERVER);
	if (status != sf::Socket::Status::Done)
	{
		cout << "Error sending Bye to server" << endl;
	}
}

void SendACK(sf::UdpSocket *socket, int idPacket)
{
	sf::Packet packet;
	CMD_TYPE CMD = CMD_TYPE::ACK;
	packet << CMD << idPacket;
	//cout << "Envio ACK de ID PACKET " << idPacket << endl;
	sf::Socket::Status status = socket->send(packet, IP_SERVER, PORT_SERVER);
	if (status != sf::Socket::Status::Done)
	{
		cout << "Error sending Bye to server" << endl;
	}
}

void SendRestartGame(sf::UdpSocket *socket)
{
	sf::Packet packet;
	CMD_TYPE CMD = CMD_TYPE::RESTART;
	packet << CMD;
	sf::Socket::Status status = socket->send(packet, IP_SERVER, PORT_SERVER);
	if (status != sf::Socket::Status::Done)
	{
		cout << "Error sending Bye to server" << endl;
	}
}

void SendMove(sf::UdpSocket *socket, int x, int y) {
	sf::Packet packet;
	CMD_TYPE CMD = CMD_TYPE::MOVE;
	packet << CMD << player << x << y;
	sf::Socket::Status status = socket->send(packet, IP_SERVER, PORT_SERVER);
	if (status != sf::Socket::Status::Done)
	{ 
		cout << "Error sending Move to server" << std::endl;
	}
}

void SendPing(sf::UdpSocket *socket, int ID)
{
	sf::Packet packet;
	CMD_TYPE CMD = CMD_TYPE::PING;
	packet << CMD << ID;
	sf::Socket::Status status = socket->send(packet, IP_SERVER, PORT_SERVER);
	if (status != sf::Socket::Status::Done)
	{
		cout << "Error sending Bye to the server" << endl;
	}
}


void checkMSG(sf::Packet packet, std::vector<Player*> players,sf::UdpSocket*sock) {

	CMD_TYPE CMD;
	packet >> CMD;

	

	switch (CMD) {
	case CMD_TYPE::CHALLENGE: {
		
		cout << "Have received a Challenge and sending Challenge_R" << endl;
		SendChallengeR(sock);
		break;
	}
	case CMD_TYPE::WELLCOME: {
		isWelcomed = true;
		
		int ID, size;
		packet >> ID >> size;
		cout << "The player " << ID << " has entered to the game " 
			"and the players in the lobby are " << size << endl;
		if (player == 0) { player = ID;  players[player - 1]->alive = true; }
		else { players[ID - 1]->alive = true; }
		for (int i = 0; i < size; i++)
		{
			players[i]->alive = true;
		}

		break;
	}
	case CMD_TYPE::START: {

		int tempIdPacket;
		packet >> countdown >> tempIdPacket;
		cout << "Received a Packet ID " << tempIdPacket << endl;
		SendACK(sock, tempIdPacket);
		players[0]->x = 100;
		players[0]->y = 100;
		players[1]->x = 400; //S'afegeix la seva posicio x
		players[1]->y = 400; //S'afegeix la seva posicio y

		gameStarted = true;		
		

		break;
	}
	case CMD_TYPE::KILL: {
		int ID;
		packet >> ID;
		cout << "The player " << ID << " has left the game " << endl;
		players[ID - 1]->alive = false;
		break;
	}
	case CMD_TYPE::MOVE_ACK: {
		int ID, x, y;
		packet >> ID >> x >> y;
		players[ID - 1]->x = x;
		players[ID - 1]->y = y;
        //cout << "Receiving from player" << ID << " " << x << " and " << y << endl;
		break;
	}
	case CMD_TYPE::PONG: {

		//cout << "Received PONG from server" << endl;
		
		break;
	}
	}
}


void createPlayers(std::vector<Player*> *players) {

	Player* player1 = new Player; //Es crea un nou player
	player1->ID = 1; //Se li afegeix la seva ID
	player1->x = 100; //S'afegeix la seva posicio x
	player1->y = 100;
	player1->alive = false; //No es posa a viu aixi no apareix
	player1->Color = sf::Color::Green;
	players->push_back(player1); //S'afegeix aquest nou client al vector de clients

	Player* player2 = new Player; //Es crea un nou player
	player2->ID = 2; //Se li afegeix la seva ID
	player2->x = 400; //S'afegeix la seva posicio x
	player2->y = 400; //S'afegeix la seva posicio y
	player2->alive = false; //No es posa a viu aixi no apareix
	player2->Color = sf::Color::Red;
	players->push_back(player2); //S'afegeix aquest nou client al vector de clients

	Player* player3 = new Player; //Es crea un nou player
	player3->ID = 2; //Se li afegeix la seva ID
	player3->x = 100; //S'afegeix la seva posicio x
	player3->y = 400; //S'afegeix la seva posicio y
	player3->alive = false; //No es posa a viu aixi no apareix
	player3->Color = sf::Color::Yellow;
	players->push_back(player3); //S'afegeix aquest nou client al vector de clients

	Player* player4 = new Player; //Es crea un nou player
	player4->ID = 3; //Se li afegeix la seva ID
	player4->x = 400; //S'afegeix la seva posicio x
	player4->y = 400; //S'afegeix la seva posicio y
	player4->alive = false; //No es posa a viu aixi no apareix
	player4->Color = sf::Color::Blue;
	players->push_back(player4); //S'afegeix aquest nou client al vector de clients
}


int main()
{
	sf::UdpSocket sock;
	sock.setBlocking(false);
	vector<Player*> players;
	createPlayers(&players);
	sf::Vector2i screenDimensions(WINDOW_WIDTH, WINDOW_HEIGHT);
	sf::RenderWindow window;
	sf::Socket::Status status;


	sf::Font font;
	if (!font.loadFromFile("arial_narrow_7.ttf"))
	{
		cout << "Can't load the font file" << std::endl;
	}

	sf::Clock clockSendingHello;
	sf::Time connectingTime = clockSendingHello.getElapsedTime();

	sf::Clock clockTimer;
	sf::Time timerTime = clockTimer.getElapsedTime();

	sf::Clock clockPing;
	sf::Time pingTime = clockPing.getElapsedTime();

	sf::Clock clockSendMov;
	sf::Time movTime = clockSendMov.getElapsedTime();
	
	string seconds = "Timer: " + std::to_string(countdown);

	sf::Text timerText(seconds, font, 14);
	timerText.setFillColor(sf::Color(0, 160, 0));
	timerText.setStyle(sf::Text::Bold);
	timerText.setPosition(sf::Vector2f(300, 10));
	timerText.setCharacterSize(20);


	window.create(sf::VideoMode(screenDimensions.x, screenDimensions.y), "GAME");

	while (window.isOpen())
	{
		sf::Event evento;
		while (window.pollEvent(evento))
		{
			switch (evento.type)
			{
			case sf::Event::Closed:
				window.close();
				SendBye(&sock);
				break;
			case sf::Event::KeyPressed:
				if (evento.key.code == sf::Keyboard::Escape)
				{
					SendBye(&sock);
					window.close();
				}
				if (evento.key.code == sf::Keyboard::Space)
				{
					if (countdown == 0)
					{
						cout << "DEMANAR REINICI PARTIDA" << endl;
						SendRestartGame(&sock);
					}
				}
				if (evento.key.code == sf::Keyboard::I)
				{
					cout << "This is your ID" << " : " << players[player-1]->ID << endl;
										
				}
				if (evento.key.code == sf::Keyboard::W)
				{
					players[player - 1]->y = players[player - 1]->y - players[player-1]->speed;
				}
				if (evento.key.code == sf::Keyboard::S)
				{
					players[player - 1]->y =players[player-1]->y + players[player - 1]->speed;
				}
				if (evento.key.code == sf::Keyboard::A)
				{
					players[player - 1]->x = players[player-1]->x - players[player-1]->speed;
				}
				if (evento.key.code == sf::Keyboard::D)
				{
					players[player - 1]->x = players[player - 1]->x + players[player - 1]->speed;
					
					
				}
			
			default:
				break;
			}
			
		}




		sf::Packet packet;
		sf::IpAddress senderIP;
		unsigned short senderPORT;
		status = sock.receive(packet, senderIP, senderPORT);
		if (status == sf::Socket::Status::NotReady){}
		if (status == sf::Socket::Status::Error) {}
		if (status == sf::Socket::Status::Done)
		{
			checkMSG(packet, players, &sock);
		}

		if (!isWelcomed)
		{
			if (clockSendingHello.getElapsedTime().asSeconds() >= 5)
			{
				cout << "You are sending a Hello packet every 5 seconds" << endl;
				SendHello(&sock);
				clockSendingHello.restart();
			}
		}
		else
		{
			if (clockPing.getElapsedTime().asMilliseconds() >= 1000)
			{
				SendPing(&sock, players[player - 1]->ID);
				clockPing.restart();
			}
			
		}
		
		

		for (int i = 0; i < players.size(); i++)
		{
			if (players[i]->alive)
			{
				sf::CircleShape PlayersCercle(PLAYER_SIZE);
				PlayersCercle.setFillColor(players[i]->Color);
				PlayersCercle.setPosition(sf::Vector2f(players[i]->x, players[i]->y));
				window.draw(PlayersCercle);
			}
		}
			

		if (gameStarted)
		{
			if (countdown > 0)
			{
				if (clockTimer.getElapsedTime().asSeconds() >= 1)
				{
					countdown = countdown - 1;
					//cout << "Timer: " << countdown << endl;

					string tempString = "Timer: " + std::to_string(countdown);
					timerText.setString(tempString);
					clockTimer.restart();
				}
				else if (clockSendMov.getElapsedTime().asMilliseconds() >= 100)
				{
					//cout << "ENVIARE MOV CADA BASTANT" << endl;
					SendMove(&sock, players[player - 1]->x, players[player - 1]->y);
					clockSendMov.restart();
				}
			}
			else
			{
				string tempString = "Press Space to Restart de Game";
				timerText.setString(tempString);
				gameStarted = false;
			}
		}


		window.draw(timerText);
		window.display();
		window.clear();		
	}
	
}