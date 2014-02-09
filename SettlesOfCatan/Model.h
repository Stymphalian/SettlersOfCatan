#pragma once
#include <vector>
#include <list>
#include "Tiles.h"
#include "model_structs.h"
#include "Logger.h"
class Player;

class Model{
public:
	enum model_error_codes_e{
		MODEL_ERROR_NONE =0,
		MODEL_ERROR_ERROR,
		MODEL_ERROR_BANK_RESOURCES,
		MODEL_ERROR_BANK_RESOURCE_BRICK, /* order of the resources here is important, alphabetic*/
		MODEL_ERROR_BANK_RESOURCE_ORE,
		MODEL_ERROR_BANK_RESOURCE_SHEEP,
		MODEL_ERROR_BANK_RESOURCE_WHEAT,
		MODEL_ERROR_BANK_RESOURCE_WOOD,		
		MODEL_ERROR_BANK_DEV_CARD,
		MODEL_ERROR_RESOURCES,
		MODEL_ERROR_PLAYER_RESOURCE_BRICK, /* order of the resources here is important, alphabetic*/
		MODEL_ERROR_PLAYER_RESOURCE_ORE,
		MODEL_ERROR_PLAYER_RESOURCE_SHEEP,
		MODEL_ERROR_PLAYER_RESOURCE_WHEAT,
		MODEL_ERROR_PLAYER_RESOURCE_WOOD,
		MODEL_ERROR_PLAYER_NOT_ENOUGH_BUILDINGS,
		MODEL_ERROR_PLACE_THIEF,
		MODEL_ERROR_PLACE_SETTLEMENT,
		MODEL_ERROR_PLACE_CITY,
		MODEL_ERROR_PLACE_ROAD,
		MODEL_ERROR_PLAY_DEV_CARD,
		MODEL_ERROR_INVALID_TILE,
		MODEL_ERROR_INVALID_FACE,
		MODEL_ERROR_INVALID_VERTEX,
		NUM_model_error_codes_e
	};
	// Constructors and Destructors
	Model(int num_players);
	virtual ~Model();

	// Methods
	//get data structures
	int get_error();
	bool set_error(Model::model_error_codes_e code);
	Tiles* get_tile(int col, int row);
	Tiles* get_tile_from_face(int face, int* c, int* r);
	Tiles* get_tile_from_vertex(int vertex, int* c, int* r);	
	vertex_face_t* get_vertex(int vertex_key);
	vertex_face_t* get_face(int face_key);
	std::vector<vertex_face_t>::iterator get_vertices_begin();
	std::vector<vertex_face_t>::iterator get_vertices_end();
	std::vector<vertex_face_t>::iterator get_faces_begin();
	std::vector<vertex_face_t>::iterator get_faces_end();
	// get members
	const dev_cards_t* get_dev_card();
	resource_t get_building_cost(building_t::buildings card);
	int get_board_height();
	int get_board_width();
	int get_num_vertices();
	int get_num_faces();
	int get_thief_x();
	int get_thief_y();
	int get_player_with_largest_army();
	int get_player_with_longest_road();
	int get_turn_count();
	int get_roll_value();
	int get_num_dice();
	int get_num_dice_sides();
	int get_current_player();
	int get_num_players();
	resource_t const* get_bank();
 	bool set_current_player(int player);
	bool set_player_with_largest_army(int player);
	bool set_player_with_longest_road(int player);
	
	// actions
	bool reset(); // resets the dev cards, bank, vertices, and faces
	void end_turn();
	bool bank_exchange(resource_t* give, resource_t* take);
	bool place_thief(int x, int y);
	bool build_building(building_t::buildings building, int key,int player);
	bool can_build_road(int face,int player);
	bool can_build_settlement(int vertex,int player);
	bool can_build_city(int vertex, int player);
	int roll(int num_dice, int num_sides);	

	// Until I create an interface between players and the model
	Player* get_player(int player);
	int num_victory_points_for_player(int player);
	bool bank_exchange(int player, resource_t* give, resource_t* take);
	bool trade_with_player(int A, resource_t* A_give, int B, resource_t* B_give);
	bool steal_random_resource_from_player(int target_player, int taking_player,int n = 1);
	void give_resources_from_roll(int roll);
	bool buy_dev_card(int player);
	bool pay_for_item(int player, resource_t* price);
	void init_players(std::vector<Player>* players);
	int get_largest_army_size(int player);
	int get_longest_road_length(int player);

private:
	Logger& logger;
	// private member variables	
	model_error_codes_e _model_error;
	int _num_dice;
	int _num_dice_sides;
	int _thief_pos_x;
	int _thief_pos_y;
	int _roll_value;
	int _turn_count;
	bool _dirty_flag_for_longest_road;
	
	int _num_levels; // how many rings are there
	int _num_extensions; // how many extensions were used
	bool _even_middle_row; // do we have an even middle row?

	configuration_t _config; // hold configuration data, such as how many tiles and cards we should have
	resource_t _bank;

	int _num_players;
	int _current_player;
	int _player_holding_largest_army_card;
	int _player_holding_longest_road_card;
	std::vector<Player> _players;
	
	int _board_width; //also the len of the longest row ( middle row)
	int _board_height;  // the number of rows on the game board
	int _board_size; // width*height	
	int _num_game_tiles; // how many game tiles are there, including the water tiles
	int _num_water_tiles; // the number of water tiles, include ports	
	Tiles* _board;
	
	std::vector<vertex_face_t> _vertex_array;
	std::vector<vertex_face_t> _face_array;

	int _deck_pos;
	std::vector<dev_cards_t> _dev_deck; // a list holding the deck of dev cards

			
	// methods
	void set_defaults();
	void init(int num_players);
	
	void establish_board_dimensions(int num_players);	
	// TODO: Make the fill* methods purely functional.
	bool fill_board(
		Tiles* board,
		int num_levels,
		int num_extensions,
		bool even_middle_row,
		int board_width,
		int board_height,
		int board_size,
		int num_game_tiles,
		int num_water_tiles,
		int* num_tiles,
		int num_dice,
		int num_dice_sides
		);
	bool fill_vertex_face_on_board(
		Tiles* board,
		int board_width,
		int board_height,
		std::vector<vertex_face_t>& vertex_array,
		std::vector<vertex_face_t>& face_array		
		);
	bool fill_vertex_face_arrays(
		Tiles* board,
		int board_width,
		int board_height,
		std::vector<vertex_face_t>& vertex_array,
		std::vector<vertex_face_t>& face_array		
		);
	int get_ring_level(int row, int col, int nlevels,int nextensions,bool even_middle_row);
	int level_from_row(int row, int levels, int num_extensions);
	int numtiles_from_row(int row, int levels, int num_extensions);
	int num_offset_tiles_from_row(int row, int levels, int num_extensions);
	
	bool fill_deck(
		std::vector<dev_cards_t>& deck,
		int* default_dev_cards_cap,
		int size
		);
	
	const dev_cards_t* draw_dev_card();
	bool build_city(int player, int pos);
	bool build_settlement(int player, int pos);
	bool build_road(int player, int pos);

	bool set_vertex(int player, int pos, int type );
	bool set_face(int player, int pos, int type);
	bool _can_build_settlement(int player, int vertex_key);
	bool _can_build_city(int player, int vertex_key);
	bool _can_build_road(int player, int face_key);
	Tiles* _find_tile_from_vertex(int vertex_key, int* col, int* row);
	Tiles* _find_tile_from_face(int face_key, int* col, int* row);
	
	int num_possiblities_dice(int sum, int num_dice, int num_sides);	

	bool is_in_array(int key, std::vector<int>* cover);
	int compute_longest_road_length(int player);	
	int _path_to_vertex(int player, int v1, int v2);
	std::vector<int> determine_longest_road_of_players(); // expensive!
	int _get_player_with_longest_road();
	int _get_player_with_largest_army();
	
	bool face_in_range(int key);
	bool vertex_in_range(int key);
	int get_common_vertex(int edge1, int edge2);
	int get_common_face(int vertex1, int vertex2);	

	Tiles* _get_tile(int col, int row);
	std::vector<int> determine_ports_owned_by_player(int player);
};

