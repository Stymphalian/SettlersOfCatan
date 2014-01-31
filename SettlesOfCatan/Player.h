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
	std::vector<const dev_cards_t*> dev_cards;
	std::vector<int> buildings; // the vertex number
	std::vector<int> roads; // the face number
	std::vector<int> owned_ports;
	int num_soldiers;
	int longest_road;
	int building_cap[building_t::NUM_OF_BUILDINGS];

	//construtor and destructor
	Player();
	virtual ~Player();
	//methods
	bool add_resource(int type, int amount);
	int get_hand_size();
	bool owns_port(Tiles::tiles port_type);
	void init(std::string name,
		SDL_Color color,
		int hand_size,
		resource_t start_resources,
		int num_soldiers,
		int start_building_cap[building_t::NUM_OF_BUILDINGS]
		);
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
