#pragma once
#include <string>
class ini_reader;

class Configuration{
public:
	static std::string log_name;
	static std::string data_directory;
	static int num_players;
	static unsigned rin_data;
	
	// game properties
	static std::string title;
	static int DISP_X;
	static int DISP_Y;
	static int DISP_W;
	static int DISP_H;
	static int VERTEX_HITBOX_W;
	static int VERTEX_HITBOX_H;
	static int HEXTILE_W;
	static int HEXTILE_H;
	static int SPRITE_SMALL_W;
	static int SPRITE_SMALL_H;
	static int SPRITE_MEDIUM_W;
	static int SPRITE_MEDIUM_H;
	static int SPRITE_LARGE_W;
	static int SPRITE_LARGE_H;
	static int SPRITE_FACE_W;
	static int SPRITE_FACE_H;
	static int FPS;

	// number of dice, and sides
	static int num_dice;
	static int num_dice_sides;
	// building costs
	static int road_costs[5];
	static int settlement_costs[5];
	static int city_costs[5];
	static int dev_card_costs[5];
	// default player buildings
	static int default_player_road;
	static int default_player_settlement;
	static int default_player_city;
	static int default_player_dev_cards;
	// default board tiles
	static int default_sheep_tiles;
	static int default_brick_tiles;
	static int default_wood_tiles;
	static int default_wheat_tiles;
	static int default_ore_tiles;
	static int default_sheep_port; 
	static int default_brick_port;
	static int default_wood_port;
	static int default_wheat_port;
	static int default_ore_port;
	static int default_trade_ports;
	static int default_desert_tiles;
	// default bank resources
	static int default_bank_brick;
	static int default_bank_ore;
	static int default_bank_sheep;
	static int default_bank_wheat;
	static int default_bank_wood;
	 // dev cards
	static int default_bank_dev_monopoly;
	static int default_bank_dev_road_building;
	static int default_bank_dev_year_plenty;
	static int default_bank_dev_soldier;
	static int default_bank_dev_victory;
	// extensions
	static int extensions_bank_resources;
	static int extensions_bank_dev_monopoly;
	static int extensions_bank_dev_road_building;
	static int extensions_bank_dev_year_plenty;
	static int extensions_bank_dev_soldier;
	static int extensions_bank_dev_victory;

	static bool defaults();
	static bool load();
	static void print();
private:
	Configuration(){};
	virtual ~Configuration(){};
	Configuration(const Configuration& orig);
	void operator= (Configuration const&);
};


