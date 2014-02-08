#include <string>

#include "Configuration.h"
#include "ini_reader.h"

std::string Configuration::log_name;
std::string Configuration::data_directory;
int Configuration::num_players;
unsigned Configuration::rin_data;

std::string Configuration::title;
int Configuration::DISP_X;
int Configuration::DISP_Y;
int Configuration::DISP_W;
int Configuration::DISP_H;
int Configuration::VERTEX_HITBOX_W;
int Configuration::VERTEX_HITBOX_H;
int Configuration::HEXTILE_W;
int Configuration::HEXTILE_H;
int Configuration::SPRITE_SMALL_W;
int Configuration::SPRITE_SMALL_H;
int Configuration::SPRITE_MEDIUM_W;
int Configuration::SPRITE_MEDIUM_H;
int Configuration::SPRITE_LARGE_W;
int Configuration::SPRITE_LARGE_H;
int Configuration::SPRITE_FACE_W;
int Configuration::SPRITE_FACE_H;
int Configuration::FPS;

int Configuration::num_dice;
int Configuration::num_dice_sides;

int Configuration::road_costs[5];
int Configuration::settlement_costs[5];
int Configuration::city_costs[5];
int Configuration::dev_card_costs[5];

int Configuration::default_player_road;
int Configuration::default_player_settlement;
int Configuration::default_player_city;
int Configuration::default_player_dev_cards;

int Configuration::default_sheep_tiles;
int Configuration::default_brick_tiles;
int Configuration::default_wood_tiles;
int Configuration::default_wheat_tiles;
int Configuration::default_ore_tiles;
int Configuration::default_sheep_port;
int Configuration::default_brick_port;
int Configuration::default_wood_port;
int Configuration::default_wheat_port;
int Configuration::default_ore_port;
int Configuration::default_trade_ports;
int Configuration::default_desert_tiles;

int Configuration::default_bank_brick;
int Configuration::default_bank_ore;
int Configuration::default_bank_sheep;
int Configuration::default_bank_wheat;
int Configuration::default_bank_wood;

int Configuration::default_bank_dev_monopoly;
int Configuration::default_bank_dev_road_building;
int Configuration::default_bank_dev_year_plenty;
int Configuration::default_bank_dev_soldier;
int Configuration::default_bank_dev_victory;

int Configuration::extensions_bank_resources;
int Configuration::extensions_bank_dev_monopoly;
int Configuration::extensions_bank_dev_road_building;
int Configuration::extensions_bank_dev_year_plenty;
int Configuration::extensions_bank_dev_soldier;
int Configuration::extensions_bank_dev_victory;


bool Configuration::defaults(){
	// global configurations
	Configuration::log_name = "jordan.log";
	Configuration::data_directory = "data";
	Configuration::num_players = 0;
	Configuration::rin_data = 0;

	// Game configurations
	Configuration::title = "Settlers of Catan";
	Configuration::DISP_X = 0;
	Configuration::DISP_Y = 0;
	Configuration::DISP_W = 480;
	Configuration::DISP_H = 480;

	// view-game configurations
	Configuration::VERTEX_HITBOX_W = 0;
	Configuration::VERTEX_HITBOX_H = 0;
	Configuration::HEXTILE_W = 0;
	Configuration::HEXTILE_H = 0;
	Configuration::SPRITE_SMALL_W = 0;
	Configuration::SPRITE_SMALL_H = 0;
	Configuration::SPRITE_MEDIUM_W = 0;
	Configuration::SPRITE_MEDIUM_H = 0;
	Configuration::SPRITE_LARGE_W = 0;
	Configuration::SPRITE_LARGE_H = 0;
	Configuration::SPRITE_FACE_W = 0;
	Configuration::SPRITE_FACE_H = 0;
	Configuration::FPS = 0;

	Configuration::num_dice = 0;
	Configuration::num_dice_sides = 0;

	memset(Configuration::road_costs, 0, sizeof(int)* 5);
	memset(Configuration::settlement_costs, 0, sizeof(int)* 5);
	memset(Configuration::city_costs, 0, sizeof(int)* 5);
	memset(Configuration::dev_card_costs, 0, sizeof(int)* 5);

	Configuration::default_player_road = 0;
	Configuration::default_player_settlement = 0;
	Configuration::default_player_city = 0;
	Configuration::default_player_dev_cards = 0;
	
	Configuration::default_sheep_tiles = 0;
	Configuration::default_brick_tiles = 0;
	Configuration::default_wood_tiles = 0;
	Configuration::default_wheat_tiles = 0;
	Configuration::default_ore_tiles = 0;
	Configuration::default_sheep_port = 0;
	Configuration::default_brick_port = 0;
	Configuration::default_wood_port = 0;
	Configuration::default_wheat_port = 0;
	Configuration::default_ore_port = 0;
	Configuration::default_trade_ports = 0;
	Configuration::default_desert_tiles = 0;
	
	Configuration::default_bank_brick = 0;
	Configuration::default_bank_ore = 0;
	Configuration::default_bank_sheep = 0;
	Configuration::default_bank_wheat = 0;
	Configuration::default_bank_wood = 0;
	
	Configuration::default_bank_dev_monopoly = 0;
	Configuration::default_bank_dev_road_building = 0;
	Configuration::default_bank_dev_year_plenty = 0;
	Configuration::default_bank_dev_soldier = 0;
	Configuration::default_bank_dev_victory = 0;
	
	Configuration::extensions_bank_resources = 0;
	Configuration::extensions_bank_dev_monopoly = 0;
	Configuration::extensions_bank_dev_road_building = 0;
	Configuration::extensions_bank_dev_year_plenty = 0;
	Configuration::extensions_bank_dev_soldier = 0;
	Configuration::extensions_bank_dev_victory = 0;

	return true;
}

bool Configuration::load(){
	Configuration::defaults();

	CONFIG_INI::ini_reader* config = CONFIG_INI::ini_factory::open("data/config.ini");
	if(config == NULL){ return false; }

	// global configurations
	Configuration::log_name = config->get_property("log_name").value;
	Configuration::data_directory = config->get_property("data_directory").value;
	Configuration::num_players = atoi(config->get_property("num_players").value.c_str());
	Configuration::rin_data = atoi(config->get_property("rin_data").value.c_str());	

	// Game configurations
	Configuration::title = config->get_property("title", "Game").value;
	Configuration::DISP_X = atoi(config->get_property("disp_x", "Game").value.c_str());
	Configuration::DISP_Y = atoi(config->get_property("disp_y", "Game").value.c_str());
	Configuration::DISP_W = atoi(config->get_property("disp_w", "Game").value.c_str());
	Configuration::DISP_H = atoi(config->get_property("disp_h", "Game").value.c_str());

	// view-game configurations
	Configuration::VERTEX_HITBOX_W = atoi(config->get_property("vertex_hitbox_w", "View_Game").value.c_str());
	Configuration::VERTEX_HITBOX_H = atoi(config->get_property("vertex_hitbox_h", "View_Game").value.c_str());
	Configuration::HEXTILE_W = atoi(config->get_property("hextile_w", "View_Game").value.c_str());
	Configuration::HEXTILE_H = atoi(config->get_property("hextile_h", "View_Game").value.c_str());
	Configuration::SPRITE_SMALL_W = atoi(config->get_property("sprite_small_w", "View_Game").value.c_str());
	Configuration::SPRITE_SMALL_H = atoi(config->get_property("sprite_small_h", "View_Game").value.c_str());
	Configuration::SPRITE_MEDIUM_W = atoi(config->get_property("sprite_medium_w", "View_Game").value.c_str());
	Configuration::SPRITE_MEDIUM_H = atoi(config->get_property("sprite_medium_h", "View_Game").value.c_str());
	Configuration::SPRITE_LARGE_W = atoi(config->get_property("sprite_large_w", "View_Game").value.c_str());
	Configuration::SPRITE_LARGE_H = atoi(config->get_property("sprite_large_h", "View_Game").value.c_str());
	Configuration::SPRITE_FACE_W = atoi(config->get_property("sprite_face_w", "View_Game").value.c_str());
	Configuration::SPRITE_FACE_H = atoi(config->get_property("sprite_face_h", "View_Game").value.c_str());
	Configuration::FPS = atoi(config->get_property("fps", "View_Game").value.c_str());

	// num dice and num of dice sides
	Configuration::num_dice = atoi(config->get_property("num_dice", "Model").value.c_str());
	Configuration::num_dice_sides = atoi(config->get_property("num_dice_sides", "Model").value.c_str());

	// setting the building costs
	int tok_pos = 0;
	char* tok = NULL;
	std::string tokens;
	
	tokens = config->get_property("road_costs", "Model").value;
	tok = strtok((char*)tokens.c_str(),",");
	tok_pos = 0;
	while(tok != NULL){
		if(tok_pos >= 5){ break; }
		Configuration::road_costs[tok_pos++] = atoi(tok);
		tok = strtok(NULL,",");
	}
	
	tokens = config->get_property("settlement_costs", "Model").value;
	tok = strtok((char*)tokens.c_str(), ",");
	tok_pos = 0;
	while(tok != NULL){
		if(tok_pos >= 5){ break; }
		Configuration::settlement_costs[tok_pos++] = atoi(tok);
		tok = strtok(NULL, ",");
	}

	tokens = config->get_property("city_costs", "Model").value;
	tok = strtok((char*)tokens.c_str(), ",");
	tok_pos = 0;
	while(tok != NULL){
		if(tok_pos >= 5){ break; }
		Configuration::city_costs[tok_pos++] = atoi(tok);
		tok = strtok(NULL, ",");
	}
	
	tokens = config->get_property("dev_card_costs", "Model").value;
	tok = strtok((char*)tokens.c_str(), ",");
	tok_pos = 0;
	while(tok != NULL){
		if(tok_pos >= 5){ break; }
		
		Configuration::dev_card_costs[tok_pos++] = atoi(tok);
		tok = strtok(NULL, ",");
	}

	Configuration::default_player_road = atoi(config->get_property("default_player_road", "Model").value.c_str());
	Configuration::default_player_settlement = atoi(config->get_property("default_player_settlement","Model").value.c_str());
	Configuration::default_player_city = atoi(config->get_property("default_player_city","Model").value.c_str());
	Configuration::default_player_dev_cards = atoi(config->get_property("default_player_dev_cards","Model").value.c_str());

	Configuration::default_sheep_tiles = atoi(config->get_property("default_sheep_tiles","Model").value.c_str());
	Configuration::default_brick_tiles = atoi(config->get_property("default_brick_tiles","Model").value.c_str());
	Configuration::default_wood_tiles = atoi(config->get_property("default_wood_tiles","Model").value.c_str());
	Configuration::default_wheat_tiles = atoi(config->get_property("default_wheat_tiles","Model").value.c_str());
	Configuration::default_ore_tiles = atoi(config->get_property("default_ore_tiles","Model").value.c_str());
	Configuration::default_sheep_port = atoi(config->get_property("default_sheep_port","Model").value.c_str());
	Configuration::default_brick_port = atoi(config->get_property("default_brick_port","Model").value.c_str());
	Configuration::default_wood_port = atoi(config->get_property("default_wood_port","Model").value.c_str());
	Configuration::default_wheat_port = atoi(config->get_property("default_wheat_port","Model").value.c_str());
	Configuration::default_ore_port = atoi(config->get_property("default_ore_port","Model").value.c_str());
	Configuration::default_trade_ports = atoi(config->get_property("default_trade_ports","Model").value.c_str());
	Configuration::default_desert_tiles = atoi(config->get_property("default_desert_tiles","Model").value.c_str());

	Configuration::default_bank_brick = atoi(config->get_property("default_bank_brick","Model").value.c_str());
	Configuration::default_bank_ore = atoi(config->get_property("default_bank_ore","Model").value.c_str());
	Configuration::default_bank_sheep = atoi(config->get_property("default_bank_sheep","Model").value.c_str());
	Configuration::default_bank_wheat = atoi(config->get_property("default_bank_wheat","Model").value.c_str());
	Configuration::default_bank_wood = atoi(config->get_property("default_bank_wood","Model").value.c_str());

	Configuration::default_bank_dev_monopoly = atoi(config->get_property("default_bank_dev_monopoly","Model").value.c_str());
	Configuration::default_bank_dev_road_building = atoi(config->get_property("default_bank_dev_road_building","Model").value.c_str());
	Configuration::default_bank_dev_year_plenty = atoi(config->get_property("default_bank_dev_year_plenty","Model").value.c_str());
	Configuration::default_bank_dev_soldier = atoi(config->get_property("default_bank_dev_soldier","Model").value.c_str());
	Configuration::default_bank_dev_victory = atoi(config->get_property("default_bank_dev_victory","Model").value.c_str());

	Configuration::extensions_bank_resources = atoi(config->get_property("extensions_bank_resources","Model").value.c_str());
	Configuration::extensions_bank_dev_monopoly = atoi(config->get_property("extensions_bank_dev_monopoly","Model").value.c_str());
	Configuration::extensions_bank_dev_road_building = atoi(config->get_property("extensions_bank_dev_road_building","Model").value.c_str());
	Configuration::extensions_bank_dev_year_plenty = atoi(config->get_property("extensions_bank_dev_year_plenty","Model").value.c_str());
	Configuration::extensions_bank_dev_soldier = atoi(config->get_property("extensions_bank_dev_soldier","Model").value.c_str());
	Configuration::extensions_bank_dev_victory = atoi(config->get_property("extensions_bank_dev_victory","Model").value.c_str());
	
	//Configuration::print();
	delete config;	
	return true;
}

void Configuration::print(){
	int count = 0;
	printf("%d=%s\n",count++,Configuration::log_name.c_str());
	printf("%d=%s\n", count++, Configuration::data_directory.c_str());
	printf("%d=%d\n", count++, Configuration::num_players);
	printf("%d=%d\n", count++, Configuration::rin_data);

	printf("%d=%s\n", count++, Configuration::title.c_str());
	printf("%d=%d\n", count++, Configuration::DISP_X);
	printf("%d=%d\n",count++,Configuration::DISP_Y);
	printf("%d=%d\n",count++,Configuration::DISP_W);
	printf("%d=%d\n",count++,Configuration::DISP_H);
	printf("%d=%d\n",count++,Configuration::VERTEX_HITBOX_W);
	printf("%d=%d\n",count++,Configuration::VERTEX_HITBOX_H);
	printf("%d=%d\n",count++,Configuration::HEXTILE_W);
	printf("%d=%d\n",count++,Configuration::HEXTILE_H);
	printf("%d=%d\n",count++,Configuration::SPRITE_SMALL_W);
	printf("%d=%d\n",count++,Configuration::SPRITE_SMALL_H);
	printf("%d=%d\n",count++,Configuration::SPRITE_MEDIUM_W);
	printf("%d=%d\n",count++,Configuration::SPRITE_MEDIUM_H);
	printf("%d=%d\n",count++,Configuration::SPRITE_LARGE_W);
	printf("%d=%d\n",count++,Configuration::SPRITE_LARGE_H);
	printf("%d=%d\n",count++,Configuration::SPRITE_FACE_W);
	printf("%d=%d\n",count++,Configuration::SPRITE_FACE_H);
	printf("%d=%d\n",count++,Configuration::FPS);

	printf("%d=%d\n",count++,Configuration::num_dice);
	printf("%d=%d\n",count++,Configuration::num_dice_sides);

	for(int i = 0; i < 5; ++i){
		printf("%d=%d\n", count,Configuration::road_costs[i]);
	}
	count++;

	for(int i = 0; i < 5; ++i){
		printf("%d=%d\n", count, Configuration::settlement_costs[i]);
	}
	count++;

	for(int i = 0; i < 5; ++i){
		printf("%d=%d\n", count, Configuration::city_costs[i]);
	}
	count++;

	for(int i = 0; i < 5; ++i){
		printf("%d=%d\n", count, Configuration::dev_card_costs[i]);
	}
	count++;

	printf("%d=%d\n",count++,Configuration::default_player_road);
	printf("%d=%d\n",count++,Configuration::default_player_settlement);
	printf("%d=%d\n",count++,Configuration::default_player_city);
	printf("%d=%d\n",count++,Configuration::default_player_dev_cards);

	printf("%d=%d\n",count++,Configuration::default_sheep_tiles);
	printf("%d=%d\n",count++,Configuration::default_brick_tiles);
	printf("%d=%d\n",count++,Configuration::default_wood_tiles);
	printf("%d=%d\n",count++,Configuration::default_wheat_tiles);
	printf("%d=%d\n",count++,Configuration::default_ore_tiles);
	printf("%d=%d\n",count++,Configuration::default_sheep_port);
	printf("%d=%d\n",count++,Configuration::default_brick_port);
	printf("%d=%d\n",count++,Configuration::default_wood_port);
	printf("%d=%d\n",count++,Configuration::default_wheat_port);
	printf("%d=%d\n",count++,Configuration::default_ore_port);
	printf("%d=%d\n",count++,Configuration::default_trade_ports);
	printf("%d=%d\n",count++,Configuration::default_desert_tiles);

	printf("%d=%d\n",count++,Configuration::default_bank_brick);
	printf("%d=%d\n",count++,Configuration::default_bank_ore);
	printf("%d=%d\n",count++,Configuration::default_bank_sheep);
	printf("%d=%d\n",count++,Configuration::default_bank_wheat);
	printf("%d=%d\n",count++,Configuration::default_bank_wood);

	printf("%d=%d\n",count++,Configuration::default_bank_dev_monopoly);
	printf("%d=%d\n",count++,Configuration::default_bank_dev_road_building);
	printf("%d=%d\n",count++,Configuration::default_bank_dev_year_plenty);
	printf("%d=%d\n",count++,Configuration::default_bank_dev_soldier);
	printf("%d=%d\n",count++,Configuration::default_bank_dev_victory);

	printf("%d=%d\n",count++,Configuration::extensions_bank_resources);
	printf("%d=%d\n",count++,Configuration::extensions_bank_dev_monopoly);
	printf("%d=%d\n",count++,Configuration::extensions_bank_dev_road_building);
	printf("%d=%d\n",count++,Configuration::extensions_bank_dev_year_plenty);
	printf("%d=%d\n",count++,Configuration::extensions_bank_dev_soldier);
	printf("%d=%d\n",count++,Configuration::extensions_bank_dev_victory);
}