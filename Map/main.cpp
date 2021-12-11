#include <SFML/Graphics.hpp> 
#include "Map.h"

using namespace std;
using namespace sf;


int main()
{
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
    int x = 0;
    int y = 1;

    Font font;
    if (!font.loadFromFile("Electrolize-Regular.ttf"))
    {
        err() << "Couldn't load font\n";
        return 1;
    }

    Text text("Player", font, 20);
    text.setFillColor(Color::Black);
    text.setStyle(Text::Bold);
    float width = text.getLocalBounds().width;

    RectangleShape select(Vector2f(90, 90));
    select.setFillColor(Color(117, 80, 61, 100));
        
    window.setVerticalSyncEnabled(true);
    window.setKeyRepeatEnabled(false);
    
    while (window.isOpen())
    {
        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::KeyPressed)
            {
                if (event.key.code == Keyboard::Left || event.key.code == Keyboard::A) {
                    if (x != 0 && TileMap[y][x-1] != 1)
                        herosprite.setPosition(x--, y);
                    herosprite.setTextureRect(IntRect(65, 3, -60, 60));
                }
                else if (event.key.code == Keyboard::Right || event.key.code == Keyboard::D) {
                    if (x != WIDTH_MAP - 1 && TileMap[y][x+1] != 1)
                        herosprite.setPosition(x++, y);
                    herosprite.setTextureRect(IntRect(5, 3, 60, 60));
                }
                else if (event.key.code == Keyboard::Up || event.key.code == Keyboard::W) {
                    if (y != 0 && TileMap[y-1][x] != 1)
                        herosprite.setPosition(x, y--);
                }
                else if (event.key.code == Keyboard::Down || event.key.code == Keyboard::S) {
                    if (y != HEIGHT_MAP - 1 && TileMap[y+1][x] != 1)
                        herosprite.setPosition(x, y++);
                }
            }

            else if (event.type == Event::MouseMoved)
            {
                select.setPosition((event.mouseMove.x / 90) * 90, (event.mouseMove.y / 90) * 90);                
            }

            else if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left)
            {
                int i = event.mouseButton.x / 90;
                int j = event.mouseButton.y / 90;
                if (herosprite.getPosition().x >= (i * 90 - 90) && herosprite.getPosition().x <= (i * 90 + 90) &&
                    herosprite.getPosition().y >= (j * 90 - 90) && herosprite.getPosition().y <= (j * 90 + 90) &&
                    Vector2f(i * 90, j * 90) != herosprite.getPosition())
                {
                    if (TileMap[j][i] == 0)
                        TileMap[j][i] = 1;
                    else TileMap[j][i] = 0;
                }

            }

            else if (event.type == Event::Closed)
                window.close();
        }

        window.clear();

        for (size_t i = 0; i < HEIGHT_MAP; i++)
            for (size_t j = 0; j < WIDTH_MAP; j++)
            {
                if (TileMap[i][j] == 0)
                    if (i > 1)
                        map_s.setTextureRect(IntRect(194, 0, 60, 60));
                    else
                        map_s.setTextureRect(IntRect(255, 0, 60, 60));

                else if (TileMap[i][j] == 1)
                    if (i == 2)
                        map_s.setTextureRect(IntRect(72, 0, 60, 60));
                    else
                        map_s.setTextureRect(IntRect(133, 0, 60, 60));

                map_s.setPosition(j * 90, i * 90);
                window.draw(map_s);
            }

        Vertex line[] =
        {
            Vertex(Vector2f(x * 90 - 90,y * 90 - 90)),
            Vertex(Vector2f(x * 90 + 120,y * 90 - 90))

        };

        line[0].color = Color(117, 80, 61);
        line[1].color = Color(117, 80, 61);

        for (size_t i = 0; i < 4; i++)
        {
            line[0].position = Vector2f(x * 90 - 90, y * 90 - 90 + i * 90);
            line[1].position = Vector2f(x * 90 + 180, y * 90 - 90 + i * 90);

            window.draw(line, 2, Lines);
        }
        
        for (size_t i = 0; i < 4; i++)
        {
            line[0].position = Vector2f(x * 90 - 90 + i * 90, y * 90 - 90);
            line[1].position = Vector2f(x * 90 - 90 + i * 90, y * 90 + 180);

            window.draw(line, 2, Lines);
        }
             

        herosprite.setPosition(x * 90, y * 90);
        if (y != 0)
            text.setPosition(x * 90 + 45 - width / 2, y * 90 - 30);
        else
            text.setPosition(x * 90 + 45 - width / 2, 90);

        Vector2f pos = herosprite.getPosition();

        if (select.getPosition().x >= (pos.x - 90) && select.getPosition().x <= (pos.x + 90) &&
            select.getPosition().y >= (pos.y - 90) && select.getPosition().y <= (pos.y + 90) &&
            select.getPosition() != herosprite.getPosition())
        {

            window.draw(select);
        }

        window.draw(herosprite);
        window.draw(text);

        window.display();
    }       

    return 0;
}