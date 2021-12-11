#include <SFML/Graphics.hpp>
#include <iostream>
#include <memory>
#include <chrono>
#include "UdpSocket.h"
#include "../GameState/GameState.h"

using namespace sf;

//ip address of udp server
std::string const kIpAddr = "127.0.0.1";
//The port on which to listen for incoming data
u_short const kPort = 8888;
size_t const kBufferSize = 512;
char buffer[kBufferSize];

void sleep(unsigned long us)
{
	auto start = std::chrono::high_resolution_clock::now();
	auto finish = std::chrono::high_resolution_clock::now();
	auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(finish - start);
	while (microseconds.count() < us)
	{
		finish = std::chrono::high_resolution_clock::now();
		microseconds = std::chrono::duration_cast<std::chrono::microseconds>(finish - start);
	}
}

int main(int argc, const char* argv[])
{
	std::unique_ptr<UdpSocket> sock_ptr;

	std::cout << "Enter your name: ";	
	std::string name;
	getline(std::cin,name);

	GameState state;

	try
	{
		sock_ptr = std::make_unique<UdpSocket>(kIpAddr, kPort);
	}
	catch (std::exception const& err)
	{
		std::cout << "Couldn't init socket: " << err.what() << "\n";
		exit(EXIT_FAILURE);
	}

	//start communication
	//send the message
	if (sock_ptr->send(name.c_str(), name.length()) != 0)
	{
		std::cout << "Failed to send\n";
		exit(EXIT_FAILURE);
	}

	std::cout << "request sent\n";

	RenderWindow window(VideoMode(720, 360), "Perfect Game");
	View view;
	view.reset(FloatRect(0, 0, 720, 360));

	Image map_image;
	map_image.loadFromFile("../tiles.png");
	Texture map;
	map.loadFromImage(map_image);
	Sprite map_s;
	map_s.setTexture(map);
	map_s.scale(1.5, 1.5);

	Image heroimage;
	heroimage.loadFromFile("../hero.png");
	heroimage.createMaskFromColor(Color(0, 0, 255));

	Texture herotexture;
	herotexture.loadFromImage(heroimage);

	Sprite herosprite;
	herosprite.setTexture(herotexture);
	herosprite.setTextureRect(IntRect(5, 3, 60, 60));
	herosprite.scale(1.5, 1.5);

	Font font;
	if (!font.loadFromFile("../Electrolize-Regular.ttf"))
	{
		err() << "Couldn't load font\n";
		return 1;
	}

	Text text(name, font, 20);
	text.setFillColor(Color::Black);
	text.setStyle(Text::Bold);
	float width = text.getLocalBounds().width;

	RectangleShape select(Vector2f(90, 90));
	select.setFillColor(Color(117, 80, 61, 100));

	window.setVerticalSyncEnabled(true);
	window.setKeyRepeatEnabled(false);

	PlayerPos position = {0,1};

	while (1)
	{
		//receive a reply and print it
		size_t sz = kBufferSize;
		if (sock_ptr->recv(buffer, sz) != 0)
		{
			std::cout << "No data to recv\n";
			sleep(1e5);
			continue;
			//exit(EXIT_FAILURE);
		}

		std::cout << "Received game state: " << sz << "\n";
		state.deserialize(buffer, sz);
		Player* p = state.getPlayer(name);
		p->updatePos(position.first, position.second);
		PlayerPos curr_pos = p->getPos();
		std::cout << "Received player pos: (";
		std::cout << (int)curr_pos.first << "," << (int)curr_pos.second << ")\n";
		
				
		Event event;
		while (window.pollEvent(event))
		{
			if (event.type == Event::KeyPressed)
			{
				if (event.key.code == Keyboard::Left || event.key.code == Keyboard::A) 
				{	
					auto pos = p->getPos();
					if ((pos.first != 0) && (state._map[pos.second][pos.first - 1] != GameState::Block::Ground))
					{
						p->updatePos(pos.first - 1, pos.second);
					}
					herosprite.setTextureRect(IntRect(65, 3, -60, 60));
				}
				else if (event.key.code == Keyboard::Right || event.key.code == Keyboard::D) 
				{
					auto pos = p->getPos();
					if ((pos.first != state._cols - 1) && (state._map[pos.second][pos.first + 1] != GameState::Block::Ground))
					{
						p->updatePos(pos.first + 1, pos.second);
					}
					herosprite.setTextureRect(IntRect(5, 3, 60, 60));
				}
				else if (event.key.code == Keyboard::Up || event.key.code == Keyboard::W) 
				{
					auto pos = p->getPos();
					if ((pos.second != 0) &&
						(state._map[pos.second - 1][pos.first] != GameState::Block::Ground))
					{
						p->updatePos(pos.first, pos.second - 1);
					}
						
				}
				else if (event.key.code == Keyboard::Down || event.key.code == Keyboard::S) 
				{
					auto pos = p->getPos();
					if ((pos.second != state._rows - 1) &&
						(state._map[pos.second + 1][pos.first] != GameState::Block::Ground))
					{
						p->updatePos(pos.first, pos.second + 1);
					}						
				}
			}

			else if (event.type == Event::MouseMoved)
			{
				select.setPosition((event.mouseMove.x / 90) * 90, (event.mouseMove.y / 90) * 90);
			}

			else if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left)
			{
				GameIdx i = event.mouseButton.x / 90;
				GameIdx j = event.mouseButton.y / 90;
				if (Vector2f(i * 90, j * 90) == select.getPosition())
				{
					if (state._map[j][i] == GameState::Block::Background)
						state.updateMap(j, i, GameState::Block::Ground);
					else state.updateMap(j, i, GameState::Block::Background);
				}

			}

			else if (event.type == Event::Closed)
				window.close();
		}

		window.clear();

		for (size_t i = 0; i < state._rows; i++)
			for (size_t j = 0; j < state._cols; j++)
			{
				if (state._map[i][j] == GameState::Block::Background)
					if (i > 1)
						map_s.setTextureRect(IntRect(194, 0, 60, 60));
					else
						map_s.setTextureRect(IntRect(255, 0, 60, 60));

				else if (state._map[i][j] == GameState::Block::Ground)
					if (i == 2)
						map_s.setTextureRect(IntRect(72, 0, 60, 60));
					else
						map_s.setTextureRect(IntRect(133, 0, 60, 60));

				map_s.setPosition(j * 90, i * 90);
				window.draw(map_s);
			}

		Vertex line[] =
		{
			Vertex(Vector2f(p->getPos().first * 90 - 90, p->getPos().second * 90 - 90)),
			Vertex(Vector2f(p->getPos().first * 90 + 120, p->getPos().second * 90 - 90))

		};

		line[0].color = Color(117, 80, 61);
		line[1].color = Color(117, 80, 61);

		for (size_t i = 0; i < 4; i++)
		{
			line[0].position = Vector2f(p->getPos().first * 90 - 90, p->getPos().second * 90 - 90 + i * 90);
			line[1].position = Vector2f(p->getPos().first * 90 + 180, p->getPos().second * 90 - 90 + i * 90);

			window.draw(line, 2, Lines);
		}

		for (size_t i = 0; i < 4; i++)
		{
			line[0].position = Vector2f(p->getPos().first * 90 - 90 + i * 90, p->getPos().second * 90 - 90);
			line[1].position = Vector2f(p->getPos().first * 90 - 90 + i * 90, p->getPos().second * 90 + 180);

			window.draw(line, 2, Lines);
		}

		herosprite.setPosition(p->getPos().first * 90, p->getPos().second * 90);
		if (p->getPos().second != 0)
			text.setPosition(p->getPos().first * 90 + 45 - width / 2, p->getPos().second * 90 - 30);
		else
			text.setPosition(p->getPos().first * 90 + 45 - width / 2, 90);

		Vector2f pos = herosprite.getPosition();

		if (select.getPosition().x >= (pos.x - 90) && select.getPosition().x <= (pos.x + 90) &&
			select.getPosition().y >= (pos.y - 90) && select.getPosition().y <= (pos.y + 90) &&
			select.getPosition() != pos)
		{

			window.draw(select);
		}
				
		window.draw(herosprite);
		window.draw(text);

		position = p->getPos();

		Sprite enemy;
		enemy.setTexture(herotexture);
		enemy.setColor(Color(255, 255, 255, 155));
		enemy.setTextureRect(IntRect(5, 3, 60, 60));
		enemy.scale(1.5, 1.5);

		for (auto& it : state._players) 
		{
			if (it.first != name) 
			{
				enemy.setPosition(it.second.getPos().first * 90, it.second.getPos().second * 90);
				Text text1(it.first, font, 20);
				text1.setFillColor(Color(0, 0, 0, 155));
				text1.setStyle(Text::Bold);
				float width1 = text1.getLocalBounds().width;
				if (it.second.getPos().second != 0)
					text1.setPosition(it.second.getPos().first * 90 + 45 - width1 / 2, it.second.getPos().second * 90 - 30);
				else
					text1.setPosition(it.second.getPos().first * 90 + 45 - width1 / 2, 90);

				window.draw(enemy);
				window.draw(text1);
			}
		}
		
		window.display();

		sz = kBufferSize;
		p->serialize(buffer, sz);
		if (sock_ptr->send(buffer, sz) != 0)
		{
			std::cout << "Failed to send pos\n";
			exit(EXIT_FAILURE);
		}
	}

	system("pause");
	return 0;
}