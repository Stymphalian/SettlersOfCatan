#pragma once
#include <string>
#include <vector>
#include <SDL.h>
#include "Model.h"

class Player
{
public:
	//variables
	std::string name;
	SDL_Color color;
	int hand_size;
	resource_t resources;
	std::vector<dev_cards_t> dev_cards;
	std::vector<int> buildings; // the vertex number
	std::vector<int> roads; // the vertex number
 	int victory_points;
	int num_soldiers;
	int building_cap[building_t::NUM_OF_BUILDINGS];

	//construtor and destructor
	Player();
	virtual ~Player();
	//methods
	void init(std::string name,SDL_Color color);
	bool add_resource(int type, int amount);
	int get_hand_size();
};

/*
name
colour
resource_t hand 
buliding_t // how many of each building do they have left
dev_cards // what cards do they own
			// which ones are face-up and face-down ( active or not )

vector<> buildings // what building are currently in play 
							// these will reference objects which are held in the MAIN model

trade();
get_resource();
*/
