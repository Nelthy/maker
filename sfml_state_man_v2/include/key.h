#include "SFML/System/Clock.hpp"
#include "SFML/Window/Keyboard.hpp"

struct key{
	key(unsigned int keynum): boundkey(sf::Keyboard::Key(keynum)) {};
	sf::Clock time;
	float timeheld = 0;
	bool pressedlast = 0;
	bool isheld = 0;
	bool ispressed = 0;
	bool isreleased = 0;
	sf::Keyboard::Key boundkey;
};