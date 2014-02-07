#pragma once
#include "Tiles.h"
#include <string>

class vertex_face_t{
public:
	enum object_e { NONE, ROAD, SETTLEMENT, CITY, SIZE_OF_object_e };
	// variables
	object_e type; 
	int player;
	//constructor and desturctor
	vertex_face_t();
	virtual ~vertex_face_t();
	//methods
	bool is_assigned();
	void assign();
	void face(int key,int value);
	void vertex(int key,int value);
	void tile_x(int pos,int value);
	void tile_y(int pos, int value);
	
	int face(int key);
	int vertex(int key);
	int tile_x(int pos);
	int tile_y(int pos);
private:
	bool assign_once;
	struct node_t{
		int faces[4];
		int vertices[3];
		int tiles[3][2];
	}node;	
};

class resource_t{
public:	
	enum resource { BRICK, ORE, SHEEP, WHEAT, WOOD, NUM_OF_RESOURCES };	
	static std::string resource_names[resource::NUM_OF_RESOURCES];

	int res[NUM_OF_RESOURCES];
	void zero_out();
};

class building_t{
public:
	enum buildings{ NONE,ROAD, SETTLEMENT, CITY, DEV_CARD, THIEF, NUM_OF_BUILDINGS };
	int type;
};

class dev_cards_t{
public:
	enum dev_cards_e{
		SOLDIER = 0, VICTORY, MONOPOLY, YEAR_PLENTY, ROAD_BUILDING,
		NUM_OF_DEV_CARDS, NUM_VICTORY_TITLES = 10
	};
	static std::string default_messages[NUM_OF_DEV_CARDS];
	static std::string victory_point_titles[NUM_VICTORY_TITLES];
	static std::string default_titles[NUM_OF_DEV_CARDS];

	//variables
	dev_cards_e type;	
	int _title;
	int _message;
	bool visible;
	int player;
	
	// constructor and destructor
	dev_cards_t();
	virtual ~dev_cards_t();

	// methods
	void init(dev_cards_e type);
	std::string title();
	std::string message();
private:
};

class configuration_t{
public:
	// variables
	int num_extensions;
	int num_tiles[Tiles::NUM_OF_TILES];
	int buildings_cap[building_t::NUM_OF_BUILDINGS];
	int dev_cards_cap[dev_cards_t::NUM_OF_DEV_CARDS];
	int resource_cards_cap[resource_t::NUM_OF_RESOURCES];
	resource_t building_costs[building_t::NUM_OF_BUILDINGS];

	//consturctor and destructor
	configuration_t();
	virtual ~configuration_t();
	//methods
	void apply_defaults();
	void apply_extensions(int num_exts);
	int num_of_extensions(int num_players);
	int num_of_players_from_extension(int exts);
	void print_configuration_to_log();
private:	
};









