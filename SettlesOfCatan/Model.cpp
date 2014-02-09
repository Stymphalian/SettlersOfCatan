#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <string>
#include <list>
#include <iostream>

#include "Logger.h"
#include "Util.h"
#include "Configuration.h"
#include "Model.h"
#include "Player.h"
#include "Tiles.h"
#include "model_structs.h"
#include "M_math.h"

/*
	Setting the model to default empty values.
	Therefore usually 0 or -1.
*/

void Model::set_defaults(){
	_model_error = Model::MODEL_ERROR_NONE;
	_num_dice = 0;
	_num_dice_sides = 0;
	_thief_pos_x = -1;
	_thief_pos_y = -1;
	_roll_value = 0;
	_turn_count = 0;
	_dirty_flag_for_longest_road = false;
	_num_levels = 0;
	_num_extensions = 0;
	_even_middle_row = false;
	_config.apply_defaults();
	_bank.zero_out();
	_num_players = 0;
	_current_player = -1;
	_player_holding_largest_army_card = -1;
	_player_holding_longest_road_card = -1;
	_players.clear();
	_board_width = 0;
	_board_height = 0;
	_board_size = 0;
	_num_game_tiles = 0;
	_num_water_tiles = 0;
	if(_board != nullptr){
		delete[] _board;
	}
	_board = nullptr;
	_vertex_array.clear();
	_face_array.clear();
	_deck_pos = 0;
	_dev_deck.clear();		
}

Model::Model(int num_players) : logger(Logger::getLog()){
	logger.log(Logger::DEBUG, "Model Constructor(num_players=%d)", num_players);	
	_board = nullptr;
	set_defaults();

	init(num_players);	
}

Model::~Model(){
	logger.log(Logger::DEBUG, "Model::~Model Destructor");
	_players.clear();

	delete[] _board;
	_vertex_array.clear();
	_face_array.clear();
	_dev_deck.clear();
}

/*
Setups the Model such that it can support X num of players.
@Parameters int num_players -- initialize the model based on the number of players
Preconditions:
	The Model must be empty before using init()
	A call to set_defaults() before using init() is recommended.
*/
void Model::init(int num_players){	
	logger.log("Model::init(%d) ", num_players);
	srand((unsigned int)time(NULL));
	this->_model_error = MODEL_ERROR_NONE;
	this->_num_dice = Configuration::num_dice;
	this->_num_dice_sides = Configuration::num_dice_sides;
	this->_thief_pos_x = -1;
	this->_thief_pos_y = -1;
	this->_roll_value = 0;
	this->_turn_count = 0;
	this->_dirty_flag_for_longest_road = false;
	logger.log(Logger::DEBUG, "Model::init() num_dice=%d,num_dice_sides=%d", _num_dice, _num_dice_sides);
	logger.log(Logger::DEBUG, "Model::init() thief_pos_x=%d,thief_pos_y=%d", _thief_pos_x, _thief_pos_y);
	logger.log(Logger::DEBUG, "Model::init() roll_value=%d,turn_count=%d", _roll_value, _turn_count);

	
	// get the number of levels and number of extensions for the number of players we have.
	// We need thie enough in order to determine the number of tiles, resources and cards we will have for this game
	if(num_players <= 4){
		_num_extensions = 0;
		_num_levels = 4;
		_even_middle_row = false;
	} else{
		_num_extensions = _config.num_of_extensions(num_players);
		if((_num_extensions + 5) % 2 == 0){
			_even_middle_row = true;
			_num_levels = 1 + (_num_extensions + 5) / 2; // + 1 for water tiles
		} else{
			_even_middle_row = false;
			_num_levels = 1 + 1 + (_num_extensions + 5) / 2; //  +1 for water tiles
		}
	}
	logger.log("Model::init() num_extension=%d num_levels=%d even_middle_row=%d",
		_num_extensions, _num_levels, _even_middle_row);

	// set the default configuration settings
	// This determines how many resource tiles, cards, and etc
	// this game will have given that we have X number of extensions.
	_config.apply_extensions(this->_num_extensions);
	for(int i = 0; i < resource_t::NUM_OF_RESOURCES; ++i){
		_bank.res[i] = _config.resource_cards_cap[i];
	}
	_config.print_configuration_to_log();

	// Setup the players
	// TODO: We will remove this once we create an apprpriate 
	// player and model interface.
	this->_num_players = num_players;
	this->_current_player = 0;
	this->_player_holding_largest_army_card = -1;
	this->_player_holding_longest_road_card = -1;
	_players.reserve(num_players);
	Player p;
	for(int i = 0; i < num_players; ++i){		
		_players.push_back(p);
	}
	init_players(&_players);

	// TODO: Eventually it would be nice to provides an outside interface
	// for loading a game board into the model. Until then, these calls
	// will remain private.
	// Setting up the game board		
	// setup the board width, height, size as well as the num game_tiles and water_tiles 
	establish_board_dimensions(this->_num_players);
	this->_board = new Tiles[this->_board_size];
	fill_board(
		this->_board,
		this->_num_levels,
		this->_num_extensions,
		this->_even_middle_row,
		this->_board_width,
		this->_board_height,
		this->_board_size,
		this->_num_game_tiles,
		this->_num_water_tiles,
		this->_config.num_tiles,
		this->_num_dice,
		this->_num_dice_sides
		);
	fill_vertex_face_on_board(
		this->_board,
		this->_board_width,
		this->_board_height,
		this->_vertex_array,
		this->_face_array
		);
	fill_vertex_face_arrays(
		this->_board,
		this->_board_width,
		this->_board_height,
		this->_vertex_array,
		this->_face_array		
		);
	
	// Setup the dev cards deck
	// TODO: Create interface to load a deck of dev cards into the model.
	_deck_pos = 0;
	fill_deck(
		this->_dev_deck,
		this->_config.dev_cards_cap,
		dev_cards_t::NUM_OF_DEV_CARDS
		);

	// clear all the model error that might have arisen during
	// the creation of the model.
	this->_model_error = Model::MODEL_ERROR_NONE;
}


/*
Determine the board dimensions given that we want to have X number of players.
@Parameters  int num_players -- the number of players used to establish the game board.
This function will set
	_board_width
	_board_height
	_board_size
	_num_game_tiles
	_num_water_tiles
*/
void Model::establish_board_dimensions(int num_players){
	logger.log(Logger::DEBUG, "Model::establish_board_dimension(num_players=%d)", num_players);

	// Get the number of levels	
	int num_levels = 0;
	if(num_players <= 4){
		num_levels = 4;
	} else{
		int num_extensions = _config.num_of_extensions(num_players);
		if((num_extensions + 5) % 2 == 0){
			num_levels = 1 + (num_extensions + 5) / 2; // + 1 for water tiles
		} else{
			num_levels = 1 + 1 + (num_extensions + 5) / 2; //  +1 for water tiles
		}
	}

	// Get the size of the board, and length of the longest row in the board
	if(_even_middle_row == false){
		_board_width = 2 * num_levels - 1;
		_board_height = _board_width;
		_board_size = _board_width*_board_height;
		_num_game_tiles = 3 * num_levels*num_levels - 3 * num_levels + 1;  // 6(n(n+1)/2) +1 for n >= 2, where n is the number of levels
		_num_water_tiles = 6 * (num_levels - 1);
	} else{
		_board_width = num_levels * 2;
		_board_height = _board_width + 1;
		_board_size = _board_width*_board_height;
		_num_game_tiles = 3*(num_levels*num_levels) + num_levels;
		_num_water_tiles = 6*(num_levels) - 2;
	}
	logger.log(Logger::DEBUG, "Model::establish_board_dimensions m_board_width=%d,m_board_height=%d", _board_width,_board_height);
	logger.log(Logger::DEBUG, "Model::establish_board_dimensions m_board_size=%d", _board_size);
	logger.log(Logger::DEBUG, "Model::establish_board_dimensions m_num_game_tiles=%d", _num_game_tiles);
	logger.log(Logger::DEBUG, "Model::establish_board_dimensions m_num_water_tiles=%d", _num_water_tiles);
}


/*
fill the this->board with active tiles.
@Parameter Tiles* board -- An already allocated block of tiles
@Parameter int num_levels --
@Parameter int num_extensions -- to be used in get_ring_level() calls
@Parameter bool even_middle_row -- to be used in get_ring_level() calls
@Parameter int board_width,
@Parameter int board_height,
@Parameter int board_size -- board_width * board_height
@Parameter int num_game_tiles -- the total number of tiles ( land_tils + water_tiles)
@Parameter int num_water_tiles -- number of water tiles.
@Parameter int* num_tiles -- configuration of how many land tiles we can use.
@Parameter int num_dice -- To be used to determine how many possible roll values are allowed
@Parameter int num_dice_sides -- to be used to determine how many possible roll values are allowed
@Preconditions:
	* Makes use of the internal functions
		get_ring_level();
		level_from_row();
		numtiles_from_row();
		num_offset_tiles_from_row();
   * board is already allocated and is of size board_size
	* num_tiles is valid and tells you how many how each tile you can use.
*/
bool Model::fill_board(
	Tiles* board,
	int num_levels,
	int num_extensions,
	bool even_middle_row,
	int board_width,
	int board_height,
	int board_size,
	int num_game_tiles,
	int num_water_tiles,
	int* config_num_tiles,
	int num_dice,
	int num_dice_sides
	)
{
	if(!board){ return false; }
	Logger& logger = Logger::getLog();
	logger.log(Logger::DEBUG, "Model::fill_board(board,num_levels=%d,num_extension=%d,even_middle_row=%d",
		num_levels, num_extensions, even_middle_row);
	logger.log(Logger::DEBUG, "Model::fill_board(board,board_width=%d,board_height=%d,board_size=%d,num_game_tiles=%d,num_water_tiles=%d",
		board_width, board_height, board_size, num_game_tiles, num_water_tiles);

	// mark which tiles are active
	int offset = 0;
	int num_tiles = 0;
	int active_count = 0;
	for(int row = 0; row < board_height; ++row){
		num_tiles = numtiles_from_row(row, num_levels, num_extensions);
		offset = num_offset_tiles_from_row(row, num_levels, num_extensions);
		logger.log(Logger::DEBUG, "Model::fill_board() offset=%d,num_tiles=%d", offset, num_tiles);

		// mark tiles as active
		for(int col = 0; col < num_tiles; ++col){
			++active_count;
			board[col + offset + row*board_width].active = 1;
			board[col + offset + row*board_width].reset_vertices_faces();
		}
	}
	logger.log(Logger::DEBUG, "Model.fill_board() active_count = %d", active_count);
	

	// for each active tile choose a tile to place
	// get an ordering of all the tiles
	logger.log(logger.DEBUG, "Model.fill_board() Setting up the tile ordering. num_game_tiles=%d", num_game_tiles);
	logger.log(logger.DEBUG, "Model.fill_board() Setting up the water_tile ordering. num_water_tiles=%d", num_water_tiles);
	logger.log(logger.DEBUG, "Model.fill_board() Setting up the roll ordering. number of roll tokens=%d", num_game_tiles - num_water_tiles);
	int* tile_order = new int[num_game_tiles];
	int* water_tile_order = new int[num_water_tiles];
	int* roll_order = new int[num_game_tiles - num_water_tiles];
	int tile_order_size = 0;
	int roll_order_size = 0;
	int water_tile_order_size = 0;

	// normal game tiles
	for(int i = 0; i < Tiles::NUM_OF_TILES; ++i){
		for(int j = 0; j < config_num_tiles[i]; ++j){
			if(Tiles::is_water_tile(i)){
				if(water_tile_order_size >= num_water_tiles){ continue; }
				water_tile_order[water_tile_order_size++] = i;
			} else{
				if(tile_order_size >= num_game_tiles){ continue; }
				tile_order[tile_order_size++] = i;
			}
		}
	}
	// add the additional water tiles
	while(water_tile_order_size < num_water_tiles){
		water_tile_order[water_tile_order_size++] = Tiles::WATER_TILE;
	}

	// Random order for the roll values on each tile
	// NOTE: #7 is not a valid roll value, so skip it.
	logger.log(logger.DEBUG, "Model.fill_board() Random ordering of tile roll probability, board_tiles=%d,water_tiles=%d", num_game_tiles, num_water_tiles);
	while(roll_order_size < (num_game_tiles - num_water_tiles)){
		for(int i = num_dice; i <= num_dice*num_dice_sides; ++i){
			if(i == 7){ continue; }
			if(roll_order_size >= (num_game_tiles - num_water_tiles)){ break; }
			roll_order[roll_order_size++] = i;
		}
	}
	// randomize
	logger.log(logger.DEBUG, "Model.fill_board() Randomizing tile ordering. tile_order_size=%d", tile_order_size);
	Util::shuffle(tile_order, tile_order_size);
	logger.log(logger.DEBUG, "Model.fill_board() Randomizing water tile ordering. water_tile_order_size=%d", water_tile_order_size);
	Util::shuffle(water_tile_order, water_tile_order_size);
	logger.log(logger.DEBUG, "Model.fill_board() Randomizing roll ordering. roll_order_size=%d", roll_order_size);
	Util::shuffle(roll_order, roll_order_size);
	for(int i = 0; i < tile_order_size; ++i){
		logger.log(logger.DEBUG, "Model.fill_board() tile_order [%d]=%d", i, tile_order[i]);
	}
	for(int i = 0; i < water_tile_order_size; ++i){
		logger.log(logger.DEBUG, "Model.fill_board() water_tile_order [%d]=%d", i, water_tile_order[i]);
	}
	for(int i = 0; i < roll_order_size; ++i){
		logger.log(logger.DEBUG, "Model.fill_board() roll_order [%d]=%d", i, roll_order[i]);
	}

	// place the tiles onto the game board
	logger.log(logger.DEBUG, "Model.fill_board() Placing the tiles onto the gameboard");
	int tile_pos = 0;
	int roll_pos = 0;
	int water_tile_pos = 0;
	Tiles* hex = 0;
	int count = 0;
	for(int row = 0; row < board_height; ++row){
		for(int col = 0; col < board_width; ++col){
			if(board[col + row*board_width].active == 0){ continue; }
			count++;

			hex = &board[col + row*board_width];
			hex->active = 1;
			hex->roll = 0;
			hex->type = Tiles::WATER_TILE; // use a default tile, just in case.			
			hex->ring_level = get_ring_level(row, col, num_levels,num_extensions,even_middle_row);

			// determine the type and roll value of the tile
			if(hex->ring_level == num_levels - 1){
				// place water tiles on the outer ring
				if(water_tile_pos >= water_tile_order_size){
					logger.log(Logger::ERROR, "Model.fill_board() More water tiles than expected. Capacity for only %d tiles.", water_tile_order_size);
					continue;
				}
				hex->type = water_tile_order[water_tile_pos++];
				hex->roll = 0;

			} else{
				// place normal land tiles
				if(tile_pos >= tile_order_size || roll_pos >= roll_order_size) {
					logger.log(Logger::ERROR, "Model.fill_board() More tiles than expected. Capacity for only %d/%d tiles and %d/%d tokens",
						tile_pos,tile_order_size,roll_pos,roll_order_size);
					break;
				}
								
				// the desert tile should not use a roll valus
				hex->roll = (hex->type == Tiles::DESERT_TILE) ? 0 : roll_order[roll_pos++];
				hex->type = tile_order[tile_pos++];
				
			}
			logger.log(Logger::DEBUG, "Tile [%d] col=%d,row=%d ring_level=%d,type=%d,roll=%d",
				count, col, row, hex->ring_level, hex->type, hex->roll);
		}
	}

	logger.log(Logger::DEBUG, "Model.fill_board() tiles_pos=%d,water_tiles_pos=%d,roll_pos=%d", tile_pos, water_tile_pos, roll_pos);
	// Cleanup
	delete[] tile_order;
	delete[] roll_order;
	delete[] water_tile_order;
	return true;
}


/*
Given a board and arrays for vertices and faces.
@Parameter Tiles* board -- the board of Tiles in which to add vertices and faces.
@Parameter int board_width -- the width of the board
@Parameter int board_height -- the height of the board
@Parameter std::vector<vertex_face_t>& vertex_array -- the array to put vertices.
@Parameter std::vector<vertex_face_t>& face_array -- the array to place the faces.
@Return: returns true on success.
*/
bool Model::fill_vertex_face_on_board(
	Tiles* board,
	int board_width,
	int board_height,
	std::vector<vertex_face_t>& vertex_array,
	std::vector<vertex_face_t>& face_array
	)
{
	// place the tiles onto the game board
	logger.log(logger.DEBUG, "Model.fill_vertex_face_on_board(board(%x,%x),board_width=%d,board_height=%d",board,_board,board_width,board_height);
	_board[0].vertices[0] = 0;
	Tiles* hex = 0;
	int count = 0;
	for(int row = 0; row < board_height; ++row){
		for(int col = 0; col < board_width; ++col){
			if(board[col + row*board_width].active == 0){ continue; }
			count++;
			hex = &board[col + row*board_width];

			// assign the vertices to the tile
			Tiles *other = nullptr, *other2 = nullptr;
			int x, y;
			for(int i = 0; i < 6; ++i){
				// the weird ternary expression is for the special case where we wrap around the clock
				// get the least clock wise neighbour
				hex->get_adjacent((i == 0) ? 5 : i - 1, col, row, &x, &y);
				other2 = (x < 0 || y < 0 || x >= board_width || y >= board_height) // UHUHUH?
					? nullptr : &board[x + y*board_width];

				// get the most clockwise neighbour
				hex->get_adjacent(i, col, row, &x, &y);
				other = nullptr;
				other = (x < 0 || y < 0 || x >= board_width || y >= board_height)
					? nullptr : &board[x + y*board_width];

				// most clockwise neighbour
				if(other != nullptr &&
					other->active == 1 &&
					other->vertices[(i + 4) % 6] != -1)
				{
					// vertex already exists, use that one
					hex->vertices[i] = other->vertices[(i + 4) % 6];
				}
				// least clockwise neighbour
				else if(other2 != nullptr &&
					other2->active == 1 &&
					other2->vertices[(i + 2) % 6] != -1)
				{
					// vertex already exists, use that one
					hex->vertices[i] = other2->vertices[(i + 2) % 6];
				} else
				{
					// add a new vertex node
					hex->vertices[i] = vertex_array.size();
					vertex_face_t vertex;
					vertex.type = vertex_face_t::NONE;
					vertex_array.push_back(vertex);
				}
			}

			// assign the faces to the board			
			for(int i = 0; i < 6; ++i){
				hex->get_adjacent(i, col, row, &x, &y);
				other = (x < 0 || y < 0 || x >= board_width || y >= board_height)
					? nullptr : &board[x + y*board_width];

				// check the neighbouring tile to see if a face node is already assigned
				if(other != nullptr &&
					other->active == 1 &&
					other->faces[(i + 3) % 6] != -1)
				{
					// face already exists, therefore use that one
					hex->faces[i] = other->faces[(i + 3) % 6];
				} else
				{
					// add a new face node					
					hex->faces[i] = face_array.size();
					vertex_face_t face;
					face.type = vertex_face_t::NONE;
					face_array.push_back(face);
				}
			}

		}
	}

	logger.log(Logger::DEBUG, "Model.fill_vertex_face_on_board() face_array.size() = %d", (int) face_array.size());
	logger.log(Logger::DEBUG, "Model.fill_vertex_face_on_board() vertex_array.size() = %d", (int) vertex_array.size());
	logger.log(logger.DEBUG, "Model.fill_vertex_face_on_board() Assignment of face and vertex nodes");

	count = 0;
	for(int row = 0; row < board_height; ++row){
		for(int col = 0; col < board_width; ++col){
			if(board[col + row*board_width].active == 1){
				count++;
				Tiles& hex = board[col + row*board_width];
				logger.log(logger.DEBUG, "[%d] row=%d,col=%d face={%d,%d,%d,%d,%d,%d} vertices={%d,%d,%d,%d,%d,%d}",
					count, row, col,
					hex.faces[0], hex.faces[1], hex.faces[2],
					hex.faces[3], hex.faces[4], hex.faces[5],
					hex.vertices[0], hex.vertices[1], hex.vertices[2],
					hex.vertices[3], hex.vertices[4], hex.vertices[5]
					);
			}
		}
	}
	return true;
}


/*
Given the vertices and the faces, fill out the vertex_face_t 
objects with the appropriate vert and face arrays.
@Parameter Tiles* board - The board Tiles from which to discover the connections between the faces and vertices.
@Parameter int board_width -- the width of the board.
@Parameter int board_height -- the height of the board.
@Parameter std::vector<vertex_face_t>& vertex_array -- the vertex array to fill out.
@Parameter std::vector<vertex_face_t>& face_array -- the face array to fill out.
@Return: Returns true
*/
bool Model::fill_vertex_face_arrays(
	Tiles* board, 
	int board_width,
	int board_height,
	std::vector<vertex_face_t>& vertex_array,
	std::vector<vertex_face_t>& face_array		
	)
{
	logger.log(Logger::DEBUG, "Model::fill_vertex_face_arrays() Filling face and vertex arrays with board(width=%d,height=%d)", board_width, board_height);
	
	Tiles* hex;
	Tiles* temp;
	Tiles* other = 0;
	vertex_face_t* t;
	int x, y;
	for(int row = 0; row < board_height; ++row){
		for(int col = 0; col < board_width; ++col){
			hex = &board[col + row*board_width];
			if(hex->active == 0){ continue; }

			// handle vertices
			for(int v = 0; v < 6; ++v){
				if(hex->vertices[v] >= (int)vertex_array.size() || hex->vertices[v] == -1){
					logger.log(Logger::DEBUG, "Model::fill_vertex_face_array() vertex_array index %d out of bounds %d", hex->vertices[v], vertex_array.size());
					continue;
				}
				t = &vertex_array[hex->vertices[v]];
				if(t->is_assigned() == true){ continue; }
				t->assign();

				// assign the hex tiles

				// assign the current tile to index 0
				t->tile_x(0, col);
				t->tile_y(0, row);
				// assign the most clockwise tile to index 1
				hex->get_adjacent(v, col, row, &x, &y);
				temp = (x < 0 || y < 0 || x >= board_width || y >= board_height) ? nullptr : &board[x + y*board_width];
				if(temp == nullptr || temp->active == false){
					x = y = -1;
				}
				t->tile_x(1, x);
				t->tile_y(1, y);
				// assign the least clockwise tile to index 2
				hex->get_adjacent((v + 5) % 6, col, row, &x, &y);
				temp = (x < 0 || y < 0 || x >= board_width || y >= board_height) ? nullptr : &board[x + y*board_width];
				if(temp == nullptr || temp->active == false){
					x = y = -1;
				}
				t->tile_x(2, x);
				t->tile_y(2, y);


				// assign the vertices and faces
				t->vertex(0, hex->vertices[(v + 1) % 6]);
				t->vertex(1, hex->vertices[(v + 5) % 6]);
				t->face(0, hex->faces[v]);
				t->face(1, hex->faces[(v + 5) % 6]);

				if( t->tile_x(1) != -1){
					// using the most clockwise tile to find the vertices and faces
					other = &board[t->tile_x(1)+ t->tile_y(1) * board_width];
					t->vertex(2, other->vertices[(v + 5) % 6]);
					t->face(2,other->faces[(v + 4) % 6]);
				} else if(t->tile_x(2) != -1){
					// using the least clockwise tile to find the vertices and faces
					other = &board[t->tile_x(2) + t->tile_y(2) * board_width];
					t->vertex(2,other->vertices[(v + 1) % 6]);
					t->face(2,other->faces[(v + 2) % 6]);
				} else{
					// the vertex exists only if both the neighbour tiles exists
					t->vertex(2, -1); //no vertex, 
					t->vertex(2, -1);// no face
				}
			}

			// handling the faces
			for(int f = 0; f < 6; ++f){
				if(hex->faces[f] >= (int)face_array.size() || hex->faces[f] == -1){
					logger.log(Logger::DEBUG, "Model::fill_vertex_face_array() face_array index %d out of bounds %d", hex->faces[f], face_array.size());
					continue;
				}
				t = &face_array[hex->faces[f]];
				if(t->is_assigned()){ continue; }
				t->assign();

				// assign the attching tiles
				t->tile_x(0, col);
				t->tile_y(0, row);
				// get the tile sharing the current face
				hex->get_adjacent(f, col, row, &x, &y);
				temp = (x < 0 || y < 0 || x >= board_width || y >= board_height) ? nullptr : &board[x + y*board_width];
				if(temp == nullptr || temp->active == false){
					x = y = -1;
				}
				t->tile_x(1, x);
				t->tile_y(1, y);

				// assign the vertices 
				t->vertex(0, hex->vertices[f]);
				t->vertex(1, hex->vertices[(f + 1) % 6]);

				// we want the leftmost hextile to be in tiles[0]				
				int vpos = f;
				if(f >= Tiles::HEXSOUTHWEST  && t->tile_x(1) != -1){
					// so we want to swap them
					int temp_x = t->tile_x(0);
					int temp_y = t->tile_y(0);
					t->tile_x(0, t->tile_x(1));
					t->tile_y(0, t->tile_y(1));
					t->tile_x(1, temp_x);
					t->tile_y(1, temp_y);
					vpos = (f + 3) % 6; // directly opposite face on the current tile
				}
				// vpos is the direction of the face in the left-most tile.

				// assign the attaching faces
				// obtain the left tile
				Tiles* left_tile = &board[t->tile_x(0) + t->tile_y(0)*_board_width];
				t->face(0, left_tile->faces[(vpos + 1) % 6]); // most clockwise inner face
				t->face(1, left_tile->faces[(vpos + 5) % 6]); // least clockwise inner face

				if(t->tile_x(1) == -1){
					// if no neightbour is attached to the current face, 
					// let us check other adjacent tiles to see if faces
					// are attached
					t->face(2, -1); // just set to -1 to begin with.
					t->face(3, -1);

					// check if the most clockwise neighbour exists
					left_tile->get_adjacent((vpos + 1) % 6, col, row, &x, &y);
					if(x >= 0 && y >= 0 && x < board_width && y < board_height){
						other = &board[x + y*board_width];
						t->face(2, other->faces[(vpos + 5) % 6]);
					}

					// check if the least clockwise neighbour exists
					left_tile->get_adjacent((vpos + 5) % 6, col, row, &x, &y);
					if(x >= 0 && y >= 0 && x < board_width && y < board_height){
						other = &board[x + y*board_width];
						t->face(3, other->faces[(vpos + 1) % 6]);
					}
				} else {
					// get the outer faces from the adjacent tile
					other = &board[t->tile_x(1)  + t->tile_y(1) * board_width];
					t->face(2, other->faces[(vpos + 2) % 6]);
					t->face(3, other->faces[(vpos + 4) % 6]);
				}
			}


		}
	}



	// -- L O G G I N G --
	int count;
	std::vector<vertex_face_t>::iterator it;
	logger.log(Logger::DEBUG, "vertex array %d", vertex_array.size());
	count = 0;
	for(it = vertex_array.begin(); it != vertex_array.end(); ++it){
		vertex_face_t* t = &(*it);
		logger.log(Logger::DEBUG, "vertex[%d] type=%d player=%d faces {%d,%d,%d} vertices {%d,%d,%d} tiles{ (%d,%d), (%d,%d), (%d,%d)}",
			count, t->type, t->player,
			t->face(0), t->face(1), t->face(2),
			t->vertex(0), t->vertex(1), t->vertex(2),
			t->tile_x(0), t->tile_y(0),
			t->tile_x(1), t->tile_y(1),
			t->tile_x(2), t->tile_y(2)
			);
		count++;

	}

	logger.log(Logger::DEBUG, "face array %d", face_array.size());
	count = 0;
	for(it = face_array.begin(); it != face_array.end(); ++it){
		vertex_face_t* t = &(*it);
		logger.log(Logger::DEBUG, "face[%d] type=%d player=%d faces {%d,%d,%d,%d} vertices {%d,%d} tiles{ (%d,%d), (%d,%d)}",
			count, t->type, t->player,
			t->face(0), t->face(1), t->face(2), t->face(3),
			t->vertex(0), t->vertex(1),
			t->tile_x(0), t->tile_y(0),
			t->tile_x(1), t->tile_y(1)
			);
		count++;
	}
	return true;
}

/*
Deterimines the ring level of a tile given the number of levels,
number of extension and if there is an even middle row.
A ring level is defined
center tile/center ring is level 0
moving outward you increase the ring level ( i.e 0 --> 1 --> 2)

@Paramter int row -- the row of the target tile
@Paramter int col -- the column of the target tile
@Paramter int levels -- How many leve
@Paramter int nextensions,
@Paramter bool even_middle_row

Makers use of
level_from_from_row()
numtiles_from_row()
num_offset_tiles_from_row();
*/
int Model::get_ring_level(int row, int col, int levels,int nextensions,bool even_middle_row){
	int dy, numtiles, offset, border_len, center, l, t, x = 0;

	if(even_middle_row == true){
		int level = level_from_row(row, levels, nextensions);
		numtiles = numtiles_from_row(row, levels, nextensions);
		offset = num_offset_tiles_from_row(row, levels, nextensions);
		center = offset + numtiles / 2;
		//Logger::getLog().log(Logger::DEBUG, "level =%d,numtiles=%d,offset=%d,center=%d", level, numtiles,offset,center);

		if(level == 0) {
			if(col < center){
				center--;
			}
			return abs(col - center);
		} else if(level == 1){
			return abs(col - center);
		} else{
			dy = level_from_row(row, levels, nextensions);
			numtiles = numtiles_from_row(row, levels, nextensions);
			offset = num_offset_tiles_from_row(row, levels, nextensions);
			border_len = abs(numtiles - levels);
			center = offset + numtiles / 2;
			l = abs(center - offset);
			t = l - border_len;
			if(numtiles % 2 == 0 && col >= center){
				l--; t--;
			}
			x = abs(center - col);

			if(x < t){
				return dy - 1;
			} else { return dy + x - t - 1; }
		}
		return -1;
	} else{
		// special case
		if(row == levels - 1){ return abs(levels - 1 - col); }
		dy = abs(levels - 1 - row);
		numtiles = numtiles_from_row(row, levels, nextensions);
		offset = num_offset_tiles_from_row(row, levels, nextensions);
		border_len = abs(numtiles - levels);
		center = offset + numtiles / 2;
		l = abs(center - offset);
		t = l - border_len;
		if(numtiles % 2 == 0 && col >= center){
			l--; t--;
		}
		x = abs(center - col);

		if(x < t){
			return dy;
		} else { return dy + x - t; }
	}
}

// the number of tiles in the front of the row which are blank/empty tiles
// give the number of levels there are.
int Model::num_offset_tiles_from_row(int row, int level, int num_extensions){
	int offset = level_from_row(row, level, num_extensions) / 2;
	if(row % 2 == 0){
		if((num_extensions != 0) && (5 + num_extensions) % 2 == 0){
			if(level % 2 == 1){ offset++; }
		} else{
			if(level % 2 == 0){ offset++; }
		}
	}
	return offset;
}

// returns the level based off of the row, and the number of levels
// 0 is the center hex, and it increases as you extend outwrads from the center
// OR if there is an even middle row, then 0 represent the ring of 4 vertices
int Model::level_from_row(int row, int levels, int num_extensions){
	if((num_extensions != 0) && (5 + num_extensions) % 2 == 0){
		return abs(row - levels);
	} else{
		return abs(levels - 1 - row);
	}
}

// Tells you how many tiles there are on this row given the number of levels there are
int Model::numtiles_from_row(int row, int level, int num_extensions){
	if((num_extensions != 0) && (5 + num_extensions) % 2 == 0){
		return 2 * level - level_from_row(row, level, num_extensions);
	} else{
		return 2 * level - 1 - level_from_row(row, level, num_extensions);
	}
}


bool Model::fill_deck(std::vector<dev_cards_t>& deck,int* default_cards, int size){
	if(!default_cards){ return false; }
	logger.log(logger.DEBUG, "Model::fill_deck(default_cards,size=%d)", size);

	// find the number of cards in the deck
	int deck_size = 0;
	for(int i = 0; i < size; ++i){
		deck_size += default_cards[i];
	}
	logger.log(logger.DEBUG, "Model::fill_deck() deck_size=%d", deck_size);

	// a random list of victory titles to use.
	int* victory_titles = new int[dev_cards_t::NUM_VICTORY_TITLES];
	int* random_cards = new int[deck_size];
	int victory_title_size = 0;
	int random_cards_size = 0;

	// Form a random ordering of the dev card deck, and victory titles
	for(int i = 0; i < dev_cards_t::NUM_VICTORY_TITLES; ++i){
		victory_titles[victory_title_size++] = i;
	}
	for(int i = 0; i < size; ++i){
		for(int j = 0; j < default_cards[i]; ++j){
			random_cards[random_cards_size++] = i;
		}
	}

	logger.log(logger.DEBUG, "Model::fill_deck() Creating a random ordering of the cards ( shuffle )");
	Util::shuffle(victory_titles, victory_title_size);
	Util::shuffle(random_cards, random_cards_size);

	// add each dev cards to the vector list
	logger.log(logger.DEBUG, "Model::fill_deck() Adding dev cards to deck in random order");
	int number_of_victory_cards = 0;
	for(int i = 0; i < random_cards_size; ++i){
		dev_cards_t dev_card;
		dev_card.init((dev_cards_t::dev_cards_e)random_cards[i]);

		// if the card is Victory card, then assign a unique title
		if(dev_card.type == dev_cards_t::VICTORY)
		{
			if(number_of_victory_cards >= victory_title_size){
				logger.log(logger.DEBUG, "Model::fill_deck()  Using default victory card title. No more unique victory titles");
			} else{
				if(number_of_victory_cards < victory_title_size){
					dev_card._title = victory_titles[number_of_victory_cards++];
				}
			}
		}

		// add to the deck
		deck.push_back(dev_card);
		logger.log(logger.DEBUG, "Model::fill_deck() [%d] title=%s", deck.size(), dev_card.title().c_str());
		logger.log(logger.DEBUG, "Model::fill_deck() [%d] message=%s", deck.size(), dev_card.message().c_str());
	}
	logger.log(logger.DEBUG, "Model::fill_deck() m_dev_deck.size()=%d",deck.size());

	// cleanup
	delete[] random_cards;
	delete[] victory_titles;
	return true;
}

/*
@Return: returns nullptr if there are no more cards to draw from the deck.
*/
const dev_cards_t* Model::draw_dev_card(){
	if(_deck_pos >= (int)_dev_deck.size()){
		set_error(Model::MODEL_ERROR_BANK_DEV_CARD);
		return nullptr;
	} else{
		dev_cards_t* rs = &_dev_deck[_deck_pos];
		_deck_pos++;
		Logger::getLog().log(Logger::DEBUG, "Model::draw_dev_card %d of %d. title=%s,message=%s",
			_deck_pos, _dev_deck.size(), rs->title().c_str(), rs->message().c_str());
		return (const dev_cards_t*) rs;
	}
}

/*
Build a city on the game board.
Only build a city if it will result in a valid model.
Preconditions:
	player is within range.
	position is a valid vertex key.
	vertex is a setllement and owned by the player.
	The player has enough buildings to place the city.
Effects:
	Modifies the player object ( building capacity)

@Parameter int player -- the player to build the city for
@Parameter int pos -- the vertex position to place the city.
@Return: returns true if the city was built, false otherwise.
*/
bool Model::build_city(int player, int pos){
	if(pos < 0 || pos >= (int)_vertex_array.size() ||
		player < 0 || player >= _num_players)
	{
		Logger::getLog().log(Logger::ERROR, "Model::add_city() Out of range (player=%d/%d),(pos=%d/%d)", player, _num_players, pos, (int)_vertex_array.size());
		set_error(Model::MODEL_ERROR_PLACE_SETTLEMENT);
		return false;
	}
	if(_vertex_array[pos].type != vertex_face_t::SETTLEMENT ||
		_vertex_array[pos].type == vertex_face_t::CITY ||
		_vertex_array[pos].player != player)
	{
		Logger::getLog().log(Logger::DEBUG, "Model::add_city() Unsuitable Vertex type=%d,player=%d", (int)_vertex_array[pos].type, _vertex_array[pos].player);
		set_error(Model::MODEL_ERROR_PLACE_CITY);
		return false;
	}

	// TODO: Remove once we have a good interface with players and model.
	if(_players[player].building_cap[building_t::CITY] <= 0){
		Logger::getLog().log(Logger::DEBUG, "Model::add_city() player=%d Not enough buildings\n", player);
		set_error(Model::MODEL_ERROR_PLAYER_NOT_ENOUGH_BUILDINGS);
		return false;
	}

	// set the tile as a city now.
	set_vertex(player, pos, vertex_face_t::CITY);

	// TODO: Remove this.
	_players[player].building_cap[building_t::CITY]--;
	_players[player].building_cap[building_t::SETTLEMENT]++;
	Logger::getLog().log(Logger::DEBUG, "Model::add_city() player=%d,col=%d,row=%d,vertex=%d", player, _vertex_array[pos].tile_x(0),_vertex_array[pos].tile_y(0), pos);
	return true;
}


/*
Build a setllement on the game board.
Only build a settlement if it will result in a valid model.
Preconditions:
	player is within range.
	position is a valid vertex key.
	vertex does not alreay have a building set.
	The player has enough buildings to place the settlement.
Effects:
	Modifies the player object ( building capacity)

@Parameter int player -- the player to build the setllement for
@Parameter int pos -- the vertex position to place the setllement.
@Return: returns true if the setllement was built, false otherwise.
*/
bool Model::build_settlement(int player, int pos){
	if(pos < 0 || pos >= (int)_vertex_array.size() ||
		player < 0 || player >= _num_players)
	{
		Logger::getLog().log(Logger::ERROR, "Model::add_settlement() Out of range (player=%d/%d),(pos=%d/%d)", player, _num_players, pos, (int)_vertex_array.size());
		set_error(Model::MODEL_ERROR_PLACE_SETTLEMENT);
		return false;
	}
	if(_vertex_array[pos].type == vertex_face_t::SETTLEMENT ||
		_vertex_array[pos].type == vertex_face_t::CITY)
	{
		Logger::getLog().log(Logger::DEBUG, "Model::add_settlement() Vertex is already occupied  by %d", (int)_vertex_array[pos].type);
		set_error(Model::MODEL_ERROR_PLACE_SETTLEMENT);
		return false;
	}

	// TODO : Remove this once we have a good interface.
	if(_players[player].building_cap[building_t::SETTLEMENT] <= 0){
		Logger::getLog().log(Logger::DEBUG, "Model::add_settlement() Player %d does not have enough buildings", player);
		set_error(Model::MODEL_ERROR_PLAYER_NOT_ENOUGH_BUILDINGS);
		return false;
	}

	// find the tile associated with this vertex
	int col, row;
	Tiles* tile = _find_tile_from_vertex(pos, &col, &row);
	if(tile == nullptr){
		Logger::getLog().log(Logger::ERROR, "Model::add_settlement() Tile doesn't exist(col=%d,row=%d)", col, row);
		set_error(Model::MODEL_ERROR_PLACE_SETTLEMENT);
		return false;
	}

	// check for a connecting road, and space around the hex
	if(_can_build_settlement(player, pos)){
		set_vertex(player, pos, vertex_face_t::SETTLEMENT);
		
		// TODO: Remove this.
		_players[player].building_cap[building_t::SETTLEMENT]--;
		_players[player].buildings.push_back(pos);
		Logger::getLog().log(Logger::DEBUG, "Model::add_settlement player=%d,col=%d,row=%d,vertex=%d", player, col, row, pos);


		
		// WEIRD. Notice that we only need to check for road breaks
		// if atleast two out of the three vertices are owned by a different player.
		// we only need to re-calculate the length of that 'different' player's longest road.
		int marked_count = 0;
		int marked[2] = { -1, -1 };
		vertex_face_t* vert = &_vertex_array[pos];
		for(int i = 0; i < 3; ++i){
			if(vert->face(i) != -1){
				vertex_face_t* f = &_face_array[vert->face(i)];
				if(f->player == player){ continue; }
				if(f->player == -1){ continue; }
				if(f->type != vertex_face_t::ROAD){ continue; }
				if(marked_count >= 2){ continue; }
				marked[marked_count++] = f->player;
			}
		}		
		// make sure that the two roads connecting are owned by the same player
		// and is not -1.
		if(marked_count == 2 && marked[0] == marked[1] && marked[0] != -1){
			// recalculate the longest road for that player.
			_players[marked[0]].longest_road = compute_longest_road_length(marked[0]);
			Logger::getLog().log(Logger::DEBUG, "Model::build_settlement() Recalculating the longest road length for player %d to be %d",
				marked[0],_players[marked[0]].longest_road);
		}


		// WEIRD
		// Add any new ports that the player might own
		vert = &_vertex_array[pos];
		for(int i = 0; i < 3; ++i){
			Tiles* tiles = _get_tile(vert->tile_x(i), vert->tile_y(i));
			if(tiles == nullptr){ continue; }
			if(tiles->type >= Tiles::SHEEP_PORT && tiles->type <= Tiles::TRADE_PORTS){
				// don't add duplicate ports to the owned_ports list.
				if(is_in_array(tiles->type, &_players[player].owned_ports) == false){
					Logger::getLog().log(Logger::DEBUG, "Model::build_settlement() Player %d can now use port %d from tile col=%d,row=%d",
						player, tiles->type,vert->tile_x(i),vert->tile_y(i));
					_players[player].owned_ports.push_back(tiles->type);
				}
			}
		}
		
		return true;
	}
	Logger::getLog().log(Logger::DEBUG, "Model::add_settlement() Unable to build settlement");
	set_error(Model::MODEL_ERROR_PLACE_SETTLEMENT);
	return false;
}

/*
Build a road only it will result in a valid model.
Preconditions:
	player is within range.
	positions is a valid face key.
	face is not already set.
	The player has enough buildings to place the road
Effects:
	Modifies the player object ( building capacity)

@Parameter int player -- the player to build the road for
@Parameter int pos -- the vertex position to place the road.
@Return: returns true if the road was built, false otherwise.
*/
bool Model::build_road(int player, int pos){
	if(pos < 0 || pos >= (int)_face_array.size() ||
		player <0 || player >= _num_players)
	{
		Logger::getLog().log(Logger::ERROR, "Model::add_road() Out of bounds( player =%d/%d,pos =%d/%d)", player, _num_players, pos, (int)_face_array.size());
		set_error(Model::MODEL_ERROR_PLACE_ROAD);
		return false;
	}
	if(_face_array[pos].type != vertex_face_t::NONE)
	{
		Logger::getLog().log(Logger::DEBUG, "Model::add_road() Face is already filled with an object %d", _face_array[pos].type);
		set_error(Model::MODEL_ERROR_PLACE_ROAD);
		return false;
	}
	// TODO: Remove this once an interface between players and model is set.
	if(_players[player].building_cap[building_t::ROAD] <= 0){
		set_error(Model::MODEL_ERROR_PLAYER_NOT_ENOUGH_BUILDINGS);
		Logger::getLog().log(Logger::DEBUG, "Model::add_road()  Can't build road. Player %d doesn't have enough builidings", player);
		return false;
	}

	// check for a connecting road
	int col, row;
	Tiles* tile = _find_tile_from_face(pos, &col, &row);
	if(tile == nullptr){
		set_error(Model::MODEL_ERROR_PLACE_ROAD);
		Logger::getLog().log(Logger::ERROR, "Model::add_road()  Tile can't be found (col=%d,row=%d)", col, row);
		return false;
	}

	// if a road exists, then we can build a road
	if(_can_build_road(player, pos)){
		set_face(player, pos, vertex_face_t::ROAD);
		
		// TODO: Revmoe this.
		_players[player].building_cap[building_t::ROAD]--;
		_players[player].roads.push_back(pos);
		Logger::getLog().log(Logger::DEBUG, "Model::add_road player=%d,col=%d,row=%d,face=%d", player, col, row, pos);
	
		_players[player].longest_road = compute_longest_road_length(player);
		return true;
	}
	Logger::getLog().log(Logger::DEBUG, "Model::add_road()  Unable to build road.");
	set_error(Model::MODEL_ERROR_PLACE_ROAD);
	return false;
}

bool Model::set_vertex(int player, int vertex_key, int type){
	if(player < -1 || player >= _num_players){ return false; }
	if(vertex_key < 0 || vertex_key >= (int) _vertex_array.size()){ return false; }
	_vertex_array[vertex_key].type = (vertex_face_t::object_e) type;
	_vertex_array[vertex_key].player = player;
	return true;
}
bool Model::set_face(int player, int face_key, int type){
	if(player < -1 || player >= _num_players){ return false; }
	if(face_key < 0 || face_key >= (int)_face_array.size()){ return false; }
	_face_array[face_key].type = (vertex_face_t::object_e) type;
	_face_array[face_key].player = player;
	return true;
}

/*
Check if you can build a road.
check to make sure that there is a road connecting to this placed road.
Check that the road is not blocked by another player's city.
Check that the road is not placed on a water tile face.

Note:
	There are special conditions if it is the player's first two roads.
	The road has the additonal condition in that it must be placed leading
	out of a settlement.

@Parameter int player -- the player to see if a road can be placed for.
@Parameter int pos - the face key to place the road.
@Return true if it is possible to build the road. false otherwise
*/
bool Model::_can_build_road(int player, int pos){
	if(player < 0 || player >= _num_players){ return false; }
	if(pos < 0 || pos >= (int)_face_array.size()){
		Logger::getLog().log(Logger::ERROR, "Model::can_build_road() pos is out of range %d/%d", pos, (int)_face_array.size());
		return false;
	}

	//vertex_face_t* face = &face_array[tile->faces[pos]];
	vertex_face_t* face = &_face_array[pos];
	bool road_exists;
	bool blocked;
	bool water_tiled;
	bool special_condition;

	// check the two faces beside to make sure one is a land tile
	water_tiled = true;
	for(int i = 0; i < 2; ++i){
		Tiles* t = get_tile(face->tile_x(i), face->tile_y(i));
		if(t == nullptr){ continue; }
		if(!t->is_water_tile(t->type)){ water_tiled = false; break; }
	}

	// check every face for a road that belong to the player
	vertex_face_t* edge = nullptr;
	road_exists = false;
	int connecting_road = -1;
	for(int i = 0; i < 4; ++i){
		if(face->face(i) == -1){ continue; }
		if(face->face(i) < 0 || face->face(i) >= (int)_face_array.size()){
			Logger::getLog().log(Logger::ERROR, "Model::can_build_road() %d/%d index out of range.", face->face(i), (int)_face_array.size());
			continue;
		}
		edge = &_face_array[face->face(i)];

		if(edge->type == vertex_face_t::ROAD && edge->player == player){
			road_exists = true;
			connecting_road = face->face(i);
			break;
		}
	}

	// check that there doesn't exist a city/settlement that
	// is blocking the placement of the road
	blocked = false;
	bool blocked_i[2] = { false, false };
	bool always_okay_to_build = false;
	//check the two vertices attached to this face.
	for(int i = 0; i < 2; ++i){
		if(!(vertex_in_range(face->vertex(i))) ){ continue; }
		vertex_face_t* target = &_vertex_array[face->vertex(i)];
		// if one of the vertices is a player owned settlement || city the
		// it is always possibel to build a road
		if((target->type == vertex_face_t::CITY || target->type == vertex_face_t::SETTLEMENT) &&
			target->player == player)
		{
			always_okay_to_build = true;
			break;
		}

		// check the two edges attached to the target vertex
		bool need = false;
		for(int i2 = 0; i2 < 3; i2++){
			// doesn't exist.
			if(!face_in_range(target->face(i2))){ continue; }
			// skip the target face.
			if(target->face(i2) == pos){ continue; }
		
			vertex_face_t* e = &_face_array[target->face(i2)];
			if(e->type == vertex_face_t::ROAD && e->player == player){
				need = true;
				break;
			}			
		}

		// if we don't need this vertex, then we can consider it as blocked
		if(need == false){
			blocked_i[i] = true;
		}else{
			if(target->player != player &&
				(target->type == vertex_face_t::CITY || target->type == vertex_face_t::SETTLEMENT)
				)
			{
				blocked_i[i] = true;
			} else{
				always_okay_to_build = true;
			}
		}
	}

	if(always_okay_to_build){
		blocked = false;
	} else{
		blocked = (blocked_i[0] && blocked_i[1]);
	}



	// TODO: Remove this condition somewhere else.
	// handle the special case in which the player is placing their first roads.
	// This means that there doesn't 
	// 1) need to be an existing linked road.
	// 2) And that one of the attached vertices must be a player owned settlment.
	special_condition = true;
	if((int)get_player(player)->roads.size() < 2){
		do{
			road_exists = true;
			special_condition = false;

			// for the two attached vertices for this face
			// check to see if there is a vertex in which there are no roads attached to.
			for(int vindex = 0; vindex < 2; vindex++){
				vertex_face_t* v1 = get_vertex(face->vertex(vindex));
				if(v1 == nullptr){ continue; }

				if(v1->type == vertex_face_t::SETTLEMENT && v1->player == player){
					bool is_okay = true;
					// check that this settlment doesn't already have an attached road
					for(int i = 0; i < 3; ++i){
						if(get_face(v1->face(i)) == nullptr){ continue; }
						if(get_face(v1->face(i))->type != vertex_face_t::NONE){
							// there is already an attached road.
							is_okay = false;
							break;
						}
					}

					if(is_okay == true){
						// no attached roads for this vertex.
						// it is a settlement
						// is is player owned
						special_condition = true;
						break;
					}
				}
			}
		} while(false);
	}


	Logger::getLog().log(Logger::DEBUG, "Model::_can_build_road() building road conditions road_exist=%d blocked=%d water_tiled=%d special_condition=%d",
					road_exists, blocked, water_tiled, special_condition);
	return (road_exists && !blocked && special_condition && !water_tiled);
}



bool Model::_can_build_city(int player, int v){
	if(player < 0 || player >= _num_players){ return false; }
	if(v < 0 || v >= (int)_vertex_array.size()){
		Logger::getLog().log(Logger::ERROR, "Model::can_build_settlement() index out of range v=%d/%d", v, (int)_vertex_array.size());
		return false;
	}
	vertex_face_t* vertex = &_vertex_array[v];
	return (vertex->type == vertex_face_t::SETTLEMENT && vertex->player == player);
}


/*
Check if you can build a settlement
check to make sure that there is a road connecting to this vertex.
Check that there is a space ( vertex ) between each settlement.
Check that the road is not placed on a water tile vertex.

Note:
There are special conditions if it is the player's first two settlements.
The first  two settlments do not need a connecting road.

@Parameter int player -- the player to see if a settlement can be placed for.
@Parameter int pos - the vertex key to place the settlement.
@Return true if it is possible to build the settlement. false otherwise
*/
bool Model::_can_build_settlement(int player, int v){
	if(player <0 || player >= _num_players){ return false; }
	if(v < 0 || v >= (int)_vertex_array.size()){
		Logger::getLog().log(Logger::ERROR, "Model::can_build_settlement() index out of range v=%d/%d", v, (int)_vertex_array.size());
		return false;
	}

	vertex_face_t* vertex = &_vertex_array[v];
	bool has_space = false;
	bool is_connected = false;
	bool water_tiled = true;

	// check the three tiles beside to make sure one is a land tile
	water_tiled = true;
	for(int i = 0; i < 3; ++i){
		Tiles* t = get_tile(vertex->tile_x(i), vertex->tile_y(i));
		if(t == nullptr){ continue; }
		if(!t->is_water_tile(t->type)){ water_tiled = false; break; }
	}

	// check every vertex for for an existing settlement/city 
	vertex_face_t* point = nullptr;
	has_space = true;
	for(int i = 0; i < 3; ++i){
		if(vertex->vertex(i) == -1){ continue; }
		if(vertex->vertex(i) < 0 || vertex->vertex(i) >= (int)_vertex_array.size()){
			Logger::getLog().log(Logger::ERROR, "Model::can_build_settlement() %d/%d vertex index out of range", vertex->vertex(i), (int)_vertex_array.size());
			continue;
		}
		point = &_vertex_array[vertex->vertex(i)];
		if(point->type == vertex_face_t::CITY ||
			point->type == vertex_face_t::SETTLEMENT)
		{
			has_space = false;
			break;
		}
	}

	// check for the existence of a road
	vertex_face_t* edge = nullptr;
	is_connected = false;
	for(int i = 0; i < 3; ++i){
		if(vertex->face(i) == -1){ continue; }
		if(vertex->face(i) < 0 || vertex->face(i) >= (int)_face_array.size()){
			Logger::getLog().log(Logger::ERROR, "Model::can_build_settlement() %d/%d face index out of range", vertex->face(i), (int)_face_array.size());
			continue;
		}

		edge = &_face_array[vertex->face(i)];
		if(edge->type == vertex_face_t::ROAD && edge->player == player){
			is_connected = true;
			break;
		}
	}

	// TODO: Move this condition somewhere else
	// Handle the speical case in which the player is placing their 
	// first few settlments and roads.
	if(get_player(player) != nullptr){
		if(get_player(player)->buildings.size() < 2){
			is_connected = true;
		}
	}

	return (has_space && is_connected && water_tiled == false);
}



/*
Retrieves a Tile which contains the vertex key
@Return: reuturn nullptr if the tile is not found.
*/
Tiles* Model::_find_tile_from_vertex(int vertex, int* col, int* row)
{
	if(vertex < 0 || (unsigned)vertex >= _vertex_array.size()){
		Logger::getLog().log(Logger::ERROR, "Model::find_tile_from_vertex() %d out of range %d-%d",
			vertex, 0, (int) _vertex_array.size());
		set_error(Model::MODEL_ERROR_INVALID_VERTEX);
		return nullptr;
	}
	vertex_face_t* v = &_vertex_array[vertex];
	if(v->tile_x(0) == -1){ return nullptr; }
	if(col != nullptr){ *col = v->tile_x(0); }
	if(row != nullptr){ *row = v->tile_y(0); }
	return &_board[v->tile_x(0) + v->tile_y(0) * _board_width];
}

/*
Retrieves a Tile which contains the face key.
@Return: reuturn nullptr if the tile is not found.
*/
Tiles* Model::_find_tile_from_face(int face, int* col, int* row)
{
	if(face < 0 || (unsigned)face >= _face_array.size()){
		Logger::getLog().log(Logger::ERROR, "Model::find_tile_from_face() %d out of range %d-%d",
			face, 0, (int) _face_array.size());
		set_error(Model::MODEL_ERROR_INVALID_FACE);
		return nullptr;
	}
	vertex_face_t* f = &_face_array[face];
	if(f->tile_x(0) == -1){ return nullptr; }
	if(col != nullptr){ *col = f->tile_x(0); }
	if(row != nullptr){ *row = f->tile_y(0); }
	return &_board[f->tile_x(0) + f->tile_y(0)* _board_width];
}


int Model::num_possiblities_dice(int sum, int num_dice, int num_sides){
	int limit = (sum - num_dice) / num_sides;
	int count = 0;
	int temp = 0;
	for(int k = 0; k < limit; ++k){
		temp *= (k % 2 == 0) ? 1 : -1;
		temp *= M_math::get().combination(num_dice, k);
		temp *= M_math::get().combination(sum - num_sides*k - 1, num_dice - 1);
		count += temp;
	}
	return count;
}


bool Model::is_in_array(int key, std::vector<int>* cover){
	for(int i = 0; i < (int)cover->size(); ++i){
		if(key == (*cover)[i]){ return true; }
	}
	return false;
}

/*
Determine the longest road length for the given player.

The algorithm works as follows:
let E be all the roads owned by the play
let V be all the vertices which are the endpoints of the set E.
let A[][] 
For every pair of vertice vi,vj in V
	compute A[vi][vj] = longest road between vi and vj
Determine the longest road by finding the maximum value in A.

@Parameter int player - the key to determine which player we are dealing with.
@Return - int  the length of the longest road for this player.
*/
int Model::compute_longest_road_length(int player){
	if(player < 0 || player >= (int)_players.size()){ return -1; }

	Player* p = &_players[player];
	std::vector<int> road_vertices;
	road_vertices.reserve(20);

	// add all the vertices that the roads are connected to
	// dont add duplicates.
	for(int i = 0; i < (int)p->roads.size(); ++i){
		vertex_face_t* face = &_face_array[p->roads[i]];

		for(int j = 0; j < 2; ++j){
			if(face->vertex(j) == -1){ continue; }
			if(is_in_array(face->vertex(j), &road_vertices)){ continue; }

			road_vertices.push_back(face->vertex(j));
		}
	}

	int len = road_vertices.size();
	int* longest_path = new int[len*len]; // ALLOCATION
	memset(longest_path, 0, sizeof(int)*len*len);
	int longest = 0;
	int v1 = -1;
	int v2 = -1;

	// for every pair of vertices find the greatest distance between them.
	for(int row = 0; row < len; ++row){
		for(int col = 0; col < len; ++col){
			v1 = road_vertices[row];
			v2 = road_vertices[col];
			longest_path[col + row*len] = _path_to_vertex(player, v1, v2);
		}
	}

	// what is the longest path?
	longest = 0;
	v1 = v2 = -1;
	for(int row = 0; row < len; ++row){
		for(int col = 0; col < len; ++col){
			if(longest_path[col + row*len] > longest){
				longest = longest_path[col + row*len];
				v1 = road_vertices[row];
				v2 = road_vertices[col];
			}
		}
	}

	// DEBUGGING logging
	if(false){
		Logger::getLog().log_nnl(Logger::DEBUG,"Model::compute_longest_road_length(player=%d)      ",player);
		for(int col = 0; col < len; ++col){
			Logger::getLog().log_append(Logger::DEBUG,"%3d ", road_vertices[col]);
		}
		Logger::getLog().log_append(Logger::DEBUG, "\n");

		for(int row = 0; row < len; ++row){			
			Logger::getLog().log_nnl(Logger::DEBUG, "Model::compute_longest_road_length(player=%d)",player);
			Logger::getLog().log_append(Logger::DEBUG,"%3d ", road_vertices[row]);
			for(int col = 0; col < len; ++col){
				Logger::getLog().log_append(Logger::DEBUG,"%3d ", longest_path[col + row*len]);
			}
			Logger::getLog().log_append(Logger::DEBUG, "\n");
		}
		Logger::getLog().log_append(Logger::DEBUG, "\n");
		Logger::getLog().log(Logger::DEBUG,"Model::compute_longest_road_length(player=%d) longest=%d from %d to %d", player, longest, v1, v2);
	}

	if(true){	
		printf("      ");
		for(int col = 0; col < len; ++col){
			printf("%3d ", road_vertices[col]);
		}
		printf("\n");
	
		for(int row = 0; row < len; ++row){
			printf("[%3d] ", road_vertices[row]);
			for(int col = 0; col < len; ++col){
				printf("%3d ", longest_path[col + row*len]);
			}
			printf("\n");
		}
		printf("player=%d longest=%d from %d to %d\n", player, longest, v1, v2);
	}

	delete[] longest_path;
	return longest;
}

/*
Determines the longest path between vertices v1 and v2, given that
we can only follow edges/roads owned by the player, and any enemy
settlement/city/vertices whcih are in the way breaks the path.

Details on how the algorithm works.
This is basically a modified DFS on a undirected dis-connnected cyclic graph.

function(v,v2):
	let ecover be an array recording which edges we have already visited.
	let stack be a stack holding vertices to visit.
	let estack be a stack holding the edge that was taken to reach the current vertex
	let len_stack be a stack recording what is the length to the next vertex.
	let connected = 0 be the initial length of the longest path connecting v1 and v2

	push v onto the stack the starting vertex
	push 0 onto the len_stack

	while( stack is not empty):
		let e = estack.pop()
		let v = stack.pop()
		let len = len_stack.pop()

		add e to the ecover list

		if v is the target vertex v2 AND
		the current length 'len' is greater than the recorded length 'connected',
		then let connected == len

		if the current vertex is an enemy owned city
		then continue

		for every (vertex,road) pair connecting to vertex v
			if the road has already been covered
			then continue

			if the road is not a player owned road
			then add the road to the ecover AND continue

			push onto the stack vertex
			push onto the estack road
			push onto the len_stack (len+1)
		end for
	end while
	return connected
end function


@Parameter int player -- the player for which we can follow roads along
@Parameter int v1 -- the start vertex for the path
@Parameter int v2 -- the ending destination for the path.
@Return : the longest path from v1 to v2
// assuming player,v1 and v2 are within their appropriate ranges.
*/

int Model::_path_to_vertex(int player, int v1, int v2){
	std::vector<int> cover;
	std::vector<int> ecover;
	cover.reserve(20);
	ecover.reserve(20);
	std::list<int> stack;
	std::list<int> edge_stack; // used to keep track of what edge we used to get here.
	std::list<int> len_stack;
	
	// push the start vertex onto the stack
	stack.push_back(v1);
	len_stack.push_back(0);
	edge_stack.push_back(-1);

	int v;
	int e;
	int len;
	int connected = 0;
	bool first_time = true;
	while(!stack.empty()){
		v = stack.front();
		e = edge_stack.front();
		len = len_stack.front();
		edge_stack.erase(edge_stack.begin());
		stack.erase(stack.begin());
		len_stack.erase(len_stack.begin());

		// mark this edge as traveresed.
		// special case of the first vertex
		if(e != -1){ ecover.push_back(e); }
					
		// check to see if this is the target vertex
		if(v == v2 && len > connected){
			connected = len;
		}

	
		// make sure that the vertex is not an enemy owned city.
		vertex_face_t* vertex = &_vertex_array[v];
		if(vertex->player != player && 
			vertex->player != -1 &&
			(vertex->type == vertex_face_t::CITY || vertex->type == vertex_face_t::SETTLEMENT))
		{
			if(first_time && v == v1 && v == v2){
				// special condition in which the origin and destination vertex
				// is a enemy owned vertex.
				first_time = false;
			}else{ 
				continue; 
			}
		}
		
		// add all the other neighbours to the stack
		vertex = &_vertex_array[v];
		for(int i = 0; i < 3; ++i){
			// make sure that a player owned road is connecting the target vertex
			if(vertex->face(i) == -1){ continue; }
			vertex_face_t* road = &_face_array[vertex->face(i)];
			if(road->type != vertex_face_t::ROAD || road->player != player){
				if(!is_in_array(vertex->face(i),&ecover)){
					ecover.push_back(vertex->face(i));
				}
				continue;
			}
			// if this is a back-edge then keep going.
			if(is_in_array(vertex->face(i), &ecover)){continue;}
			
			// check to make sure that the neighbour exists
			if(!vertex_in_range(vertex->vertex(i))){ continue; }

			stack.push_front(vertex->vertex(i));
			len_stack.push_front(len + 1);
			edge_stack.push_front(vertex->face(i));
		}
	}
	return connected;
}
/*
Determines the longest road for each player.
This will interate through each player and determine from the game board
the length of their longest road.

This is a relatively expensive call when the number of roads on the board
is around 30. This is the case because we use a (n^2)*(n^3) algoritm
to determine the longest road for the player( ie compute_longest_road_length )
@Return: A vector-list with the length of the longest road for each player.

TODO: To make this call a little less expensive, we can cache the result.
We can save the road length for each player, so that if the users needs
to know the lenght for that player it is a O(1) look-up instead
of needing do this entire beast.
*/
std::vector<int> Model::determine_longest_road_of_players(){
	clock_t time = clock();
	std::vector<int> len_list;
	len_list.reserve(_num_players);
	for(int i = 0; i < _num_players; ++i){
		len_list.push_back(0);
	}

	// compute the longest road lenght for every player.
	for(int i = 0; i < (int)_players.size(); ++i){
		len_list[i] = compute_longest_road_length(i);

		// WEIRD!
		_players[i].longest_road = len_list[i];
	}

	Logger::getLog().log("Model::player_holding_longest_road_card() \
time taken for %d players = (%5f s)",_num_players, (double)(clock()-time)/CLOCKS_PER_SEC);
	return len_list;
}

/*
Determines which player has the longest road.
Preconditions:
	the player object already hold the value of their longest road.
*/
int Model::_get_player_with_longest_road(){
	int length = 0;
	int player = 0;
	int candidate = 0;

	if(_player_holding_longest_road_card == -1){
		// TODO : This 4 should be a configurable value.
		player = -1;
		length = 4;
	} else {
		player = _player_holding_longest_road_card;
		length = _players[player].longest_road;
	}

	// check all the other player to see if anyone has a longer road.
	for(int i = 0; i < (int)_players.size(); ++i){
		// assigne the longest road to the player object.
		_players[i].longest_road = _players[i].longest_road;

		if(_player_holding_longest_road_card == i){
			continue;
		}
		candidate = _players[i].longest_road;
		if(candidate > length){
			length = candidate;
			player = i;
		}
	}

	_player_holding_longest_road_card = player;
	return player;
}


/*
Make sure that the key is within range of _face_array.size();
*/
bool Model::face_in_range(int key){
	return (key >= 0 && key < (int)_face_array.size());
}
/*
Make sure that the key is within the range of _vertex_array.size()
*/
bool Model::vertex_in_range(int key){
	return (key >= 0 && key < (int)_vertex_array.size());
}

/*
Find the vertex key of the common vertex between these two edges.
@Parameter int edge1 -- One of the edges
@Parameter int edge2 -- One of the edges
@Return -1 if there does NOT exists a common vertex.
*/
int Model::get_common_vertex(int edge1, int edge2){
	if(!face_in_range(edge1) || !face_in_range(edge2)){
		Logger::getLog().log(Logger::DEBUG, "Model::get_common_vertex out of range edge1=%d/%d, edge2=%d,%d",
			edge1, (int)_face_array.size(), edge2, (int)_face_array.size());
		return -1;

	}

	vertex_face_t* e1 = &_face_array[edge1];
	vertex_face_t* e2 = &_face_array[edge2];
	for(int i = 0; i < 2; ++i){
		if(!vertex_in_range(e1->vertex(i))){ continue; }
		for(int j = 0; j < 2; ++j){
			if(!vertex_in_range(e2->vertex(j))){ continue; }
			// we have found a common vertex between the two faces.
			if(e1->vertex(i) == e2->vertex(j)){ return e1->vertex(i); }
		}
	}
	return -1;
}

/*
Find the common face between theses two vertices
@Parameter int vertex1 - the first vertex
@Parameter int vertex2 - the second vertex
@Return -1 if there does NOT exist a common face.
*/
int Model::get_common_face(int vertex1, int vertex2){
	if(!vertex_in_range(vertex1) || !vertex_in_range(vertex2)){
		Logger::getLog().log(Logger::DEBUG, "Model::get_common_edge out of range vertex1=%d/%d, vertex2=%d,%d",
			vertex1, (int)_vertex_array.size(), vertex2, (int)_vertex_array.size());
		return -1;
	}

	vertex_face_t* v1 = &_vertex_array[vertex1];
	vertex_face_t* v2 = &_vertex_array[vertex2];
	for(int i = 0; i < 3; ++i){
		if(face_in_range(v1->face(i)) == false){ continue; }
		for(int j = 0; j < 3; ++j){
			if(face_in_range(v2->face(j)) == false){ continue; }

			// we have foudn a common edge between the two vertices
			if(v1->face(i) == v2->face(j)) { return v1->face(i); }
		}
	}
	return -1;
}


Tiles* Model::_get_tile(int col, int row){
	if(col < 0 || row < 0 || col >= _board_width || row >= _board_height){
		//set_error(Model::MODEL_ERROR_INVALID_TILE);
		return nullptr;
	}
	return &_board[col + row*_board_width];
}

std::vector<int> Model::determine_ports_owned_by_player(int player){
	std::vector<int> owned_ports;
	if(player < 0 || player >= _num_players){ return owned_ports; }	

	Player* p = &_players[player];
	for(int i = 0; i < (int) p->buildings.size(); ++i){
		vertex_face_t* vertex = &_vertex_array[p->buildings[i]];

		// for every tile attached to the 
		for(int i = 0; i < 3; ++i){
			Tiles* tile = _get_tile(vertex->tile_x(i), vertex->tile_y(i));
			if(tile == nullptr){ continue; }

			// if the tile is a port, then we can mark it as player owned.
			if(tile->type >= Tiles::SHEEP_PORT && tile->type <= Tiles::TRADE_PORTS){
				// don't add duplicates.
				if(is_in_array(tile->type, &owned_ports) == false){
					owned_ports.push_back(tile->type);
				}
			}
		}
	}
	return owned_ports;
}


// -----------------------------------------------------------------
//				 P U B L I C			 M E T H O D S
// -----------------------------------------------------------------
bool Model::set_error(Model::model_error_codes_e code){
	_model_error = code;
	return true;
}
int Model::get_error(){
	return _model_error;
}

/*
@Paramter int col -- The column in which our target tile exists
@Paramter int row -- The Row in which our target tile exists
@Return -- return nullptr on faliure
*/
Tiles* Model::get_tile(int col, int row){
	return _get_tile(col, row);
}

/*
@Paramter int face -- face num in which we want to retrieve a Tile from
@Paramter int* c -- if not nullptr, it is filled with the col of the retieved tile
@Paramter int* r -- if not nullptr, it is filled with the row of the retieved tile
@Return -- return nullptr on failure
*/
Tiles* Model::get_tile_from_face(int face, int* c, int* r){
	return _find_tile_from_face(face, c, r);
}
/*
@Paramter int vertex -- vertex in which we want to retrieve a Tile from
@Paramter int* c -- if not nullptr, it is filled with the col of the retieved tile
@Paramter int* r -- if not nullptr, it is filled with the row of the retieved tile
@Return -- return nullptr on failure
*/
Tiles* Model::get_tile_from_vertex(int vertex, int* c, int* r){
	return _find_tile_from_vertex(vertex, c, r);
}


/*
@Paramter int num -- the key to the vertex in which we wan to retrieve.
@Return -- return nullptr on failure
*/
vertex_face_t* Model::get_vertex(int num){
	if(num < 0 || num >= (int)_vertex_array.size()){
		//set_error(Model::MODEL_ERROR_INVALID_VERTEX);
		return nullptr;
	}
	return &_vertex_array[num];
}
/*
@Paramter int num -- the key to the face in which we want to retrieve
@Return -- return nullptr on failure
*/
vertex_face_t* Model::get_face(int num){
	if(num < 0 || num >= (int)_face_array.size()){
		//set_error(Model::MODEL_ERROR_INVALID_FACE);
		return nullptr;
	}
	return &_face_array[num];
}


std::vector<vertex_face_t>::iterator Model::get_vertices_begin(){
	return _vertex_array.begin();
}
std::vector<vertex_face_t>::iterator Model::get_vertices_end(){
	return _vertex_array.end();
}
std::vector<vertex_face_t>::iterator Model::get_faces_begin(){
	return _face_array.begin();
}
std::vector<vertex_face_t>::iterator Model::get_faces_end(){
	return _face_array.end();
}


/*
@Return: A point to a dev_cards_t object.
*/
const dev_cards_t* Model::get_dev_card(){	
	return draw_dev_card();
}

/*
Retrieve a resource_t object telling you how many resources a
particular building costs.
@Return: Returns resource_t object
*/
resource_t Model::get_building_cost(building_t::buildings card){
	if(card < 0 || card >= building_t::NUM_OF_BUILDINGS){
		set_error(Model::MODEL_ERROR_ERROR);
		return _config.building_costs[0];
	}
	return _config.building_costs[card];
}

// get members
int Model::get_board_height(){ return _board_height; }
int Model::get_board_width(){ return _board_width; }
int Model::get_num_vertices(){ return (int)_vertex_array.size(); }
int Model::get_num_faces(){ return (int)_face_array.size(); }
int Model::get_thief_x(){ return _thief_pos_x; }
int Model::get_thief_y(){ return _thief_pos_y; }
int Model::get_player_with_largest_army(){ return _player_holding_largest_army_card; }
int Model::get_player_with_longest_road(){ return _get_player_with_longest_road(); }
int Model::get_turn_count(){ return _turn_count; }
int Model::get_roll_value(){ return _roll_value;}
int Model::get_num_dice(){ return _num_dice; }
int Model::get_num_dice_sides(){ return _num_dice_sides; }
int Model::get_current_player(){ return _current_player; }
int Model::get_num_players(){ return _num_players; }
resource_t const* Model::get_bank(){ return (resource_t const*)&_bank; }
bool Model::set_current_player(int player){
	if(player < -1 || player >= _num_players){ return false; }
	_current_player = player;
	return true;
};
bool Model::set_player_with_largest_army(int player){
	if(player < -1 || player >= _num_players){ return false; }
	_player_holding_largest_army_card = player;
	return true;
}
bool Model::set_player_with_longest_road(int player){
	if(player < -1 || player >= _num_players){ return false; }
	_player_holding_longest_road_card = player;
	return true;
}

/*
Clears all the values from the vertices, faces and tiles.
Reset the largest_army and longest road cards,
Reset the bank resources
Removes the thief from the board.
@Return return true on success. False on any kind of failure.
*/
bool Model::reset(){
	logger.log(Logger::DEBUG, "Model::reset()");
	// The thief should not be on the board
	_thief_pos_x = -1;
	_thief_pos_y = -1;

	// reset the roll value and turn_count
	_roll_value = 0;
	_turn_count = 0;

	// reset the bank to full resources
	for(int i = 0; i < resource_t::NUM_OF_RESOURCES; ++i){
		_bank.res[i] = _config.resource_cards_cap[i];
	}

	// reset the current player
	_current_player = 0;
	// No one should hold the largest army or longest road card.
	_player_holding_largest_army_card = -1;
	_player_holding_longest_road_card = -1;

	// clear all the vertices of built buildings
	for(int i = 0; i < (int)_vertex_array.size(); ++i){
		set_vertex(-1, i, vertex_face_t::NONE);
	}

	// clear all the faces of built roads
	for(int i = 0; i < (int)_face_array.size(); ++i){
		set_face(-1, i, vertex_face_t::NONE);
	}

	// clear all the tiles of stuff..
	for(int row = 0; row < _board_height; ++row){
		for(int col = 0; col < _board_width; ++col){
			if(_board[col + row*_board_width].active == 0){ continue; }
			// do nothing
		}
	}

	// reset the dev_deck to be a full deck
	_deck_pos = 0;
	for(int i = 0; i < (int)_dev_deck.size(); ++i){
		_dev_deck[i].player = -1;
		_dev_deck[i].visible = false;
	}

	// Reset Player DATA
	// TODO : Rework when we have a clear player-model interface.
	for(int i = 0; i < _num_players; ++i){
		_players[i].hand_size = 0;
		_players[i].dev_cards.clear();
		_players[i].buildings.clear();
		_players[i].roads.clear();
		_players[i].num_soldiers = 0;
		_players[i].longest_road = 0;
		_players[i].resources.zero_out();
		_players[i].owned_ports.clear();
		
		// reset the number of buildings a player can place.
		for(int j = 0; j < building_t::NUM_OF_BUILDINGS; ++j){
			_players[i].building_cap[j] = _config.buildings_cap[j];
		}
	}

	// reset the modelerror code
	_model_error = Model::MODEL_ERROR_NONE;	


	return true;
}

/*
increment and modulos the _current_player
increment the _turn_count variable.
*/
void Model::end_turn(){
	++_turn_count;
	if(_num_players == 0){ return; }
	_current_player = (_current_player + 1) % _num_players;
	
}


/*
@Parameter resource_t* give -- The amount of resources that the bank will receive on this exchange.
@Parameter resource_t* take -- The amout of resources that the bank is giving away in thie exchange.
@Return: Return true on a successful exchange, and false otherwise.
			If false, the bank state will NOT have changed.
*/
bool Model::bank_exchange(resource_t* give, resource_t* take){
	if(!give || !take){ return false; }
	resource_t bank_copy = _bank;
	for(int i = 0; i < resource_t::NUM_OF_RESOURCES; ++i){
		bank_copy.res[i] = bank_copy.res[i] + give->res[i] - take->res[i];
		if(bank_copy.res[i] < 0){ return false; }
		if(bank_copy.res[i] > _config.resource_cards_cap[i]){ return false; }
	}
	// the transaction is good, therefore set the bank values.
	_bank = bank_copy;
	return true;
}


/*
Places the thief on a tile.
Only land tiles are considered valid tiles.

@Parameter int col -- must be a valid col position to place the thief
@Parameter int row -- must be a valid row position to place the thief
@Return: Return true upon success.
*/
bool Model::place_thief(int col, int row){
	// check that the thief isn't placed on a water tile
	if(col < 0 || row < 0 || col >= _board_width || row >= _board_height ||
		Tiles::is_water_tile(_board[col + row*_board_width].type))
	{
		set_error(Model::MODEL_ERROR_PLACE_THIEF);
		return false;
	}

	_thief_pos_x = col;
	_thief_pos_y = row;
	Logger::getLog().log(Logger::DEBUG, "Model::move_thief(col=%d,row=%d)", _thief_pos_x, _thief_pos_y);
	return true;
}

/*
builds a building in the appropriate positions
@Parameter building -- Can be of type ROAD, SETTLEMENT, or CITY
@Parameter pos -- the vertex of face position to build the building.
@Parameter player -- the player in which to associate with the building.
@Return: returns true if the building was successfully built.
*/
bool Model::build_building(building_t::buildings building, int pos, int player){
	// check if we have enough resources to pay for the building.
	if(get_player(player) == nullptr){ return false; }
	if(get_player(player)->buildings.size() >= 2 &&
		get_player(player)->roads.size() >= 2
		)
	{		
		resource_t price = get_building_cost(building);
		for(int i = 0; i < resource_t::NUM_OF_RESOURCES; ++i){
			// the player doesn't have enough resources to pay for the item.
			if(get_player(player)->resources.res[i] < price.res[i]){
				set_error(Model::MODEL_ERROR_RESOURCES);
				return false;
			}
		}
	}

	// Build whatever building we need to build.
	bool rs = false;
	switch(building) {
	case(building_t::CITY) : rs = build_city(player, pos); break;
	case(building_t::SETTLEMENT) : rs = build_settlement(player, pos); break;
	case(building_t::ROAD) : rs = build_road(player, pos); break;
	}

	
	if(rs == true){
		// DONT, need to pay
		return true;
		// TODO: Remove this logic, and place in a player-to-model interface.
		// don't need to pay if you are placing the first couple of buildings.
		if((int)get_player(player)->buildings.size() - 1 < 2 &&
			(building == building_t::SETTLEMENT || building == building_t::CITY)){
			return true;
		}
		if((int)get_player(player)->roads.size() - 1 < 2 && building == building_t::ROAD){
			return true;
		}

		// pay the necessary resources
		resource_t price = get_building_cost(building);
		pay_for_item(player, &price);
		return true;
	}
	return false;
}

bool Model::can_build_road(int player, int face){
	return _can_build_road(player, face);
}
bool Model::can_build_settlement(int player, int vertex){
	return _can_build_settlement(player, vertex);
}
bool Model::can_build_city(int player,int vertex){
	return _can_build_city(player, vertex);
}

int Model::roll(int num_dice,int num_sides=6){
	_roll_value = 0;
	for(int i = 0; i < num_dice; ++i){
		_roll_value += rand() % num_sides + 1;
	}
	Logger::getLog().log(Logger::DEBUG, "Model::roll(dice=%d,num sides=%d) = %d",
												num_dice, num_sides,_roll_value);
	return _roll_value;
}



/*
TODO Create a player interface which will accomplish these tasks
*/
Player* Model::get_player(int player){
	if(player < 0 || player >= _num_players){
		//set_error(Model::MODEL_ERROR_ERROR);
		return nullptr;
	}
	return &_players[player];
}


int Model::num_victory_points_for_player(int player){
	if(player < 0 || player >= _num_players){ return -1; }
	Player& p = _players[player];

	int points = 0;

	// count the number of victory cards.
	for(int i = 0; i < (int)p.dev_cards.size(); ++i){
		if(p.dev_cards[i]->visible == false || p.dev_cards[i]->type == dev_cards_t::VICTORY){
			continue;
		}
		points++;
	}

	// count the number of settlements and cities
	for(int i = 0; i < (int)p.buildings.size(); ++i){
		if(this->_vertex_array[p.buildings[i]].type == vertex_face_t::CITY){
			points += 2;
		} else{
			points++;
		}
	}

	if(player == _player_holding_largest_army_card){ points += 2; }
	if(player == _player_holding_longest_road_card){ points += 2; }
	return points;
}

bool Model::bank_exchange(int player, resource_t* give, resource_t* take){
	if(!give || !take){ return false; }
	if(player < 0 || player >= _num_players){ return false; }
	/*Logger::getLog().log(Logger::DEBUG, "Model::bank_exchange player=%d give=%d,%d,%d,%d,%d take=%d,%d,%d,%d,%d",
		player,
		give->res[0], give->res[1], give->res[2], give->res[3], give->res[4],
		take->res[0], take->res[1], take->res[2], take->res[3], take->res[4]
		);
		*/

	resource_t player_copy = _players[player].resources;
	resource_t bank_copy = _bank;
	bool good_flag = true;;
	//Logger::getLog().log(Logger::DEBUG, "Model::bank_exchange old_bank=%d,%d,%d,%d,%d",
//		_bank.res[0], _bank.res[1], _bank.res[2], _bank.res[3], _bank.res[4]);

	for(int i = 0; i < resource_t::NUM_OF_RESOURCES; ++i){
		if(
			(bank_copy.res[i] - take->res[i] < 0) ||
			(bank_copy.res[i] + give->res[i] > _config.resource_cards_cap[i])
			)
		{
			int code = Model::MODEL_ERROR_BANK_RESOURCE_BRICK + i;
			set_error((Model::model_error_codes_e)code);
			good_flag = false;
			break;
		}
		if(player_copy.res[i] - give->res[i] < 0){
			int code = Model::MODEL_ERROR_PLAYER_RESOURCE_BRICK + i;
			set_error((Model::model_error_codes_e)code);
			good_flag = false;
			break;
		}

		bank_copy.res[i] -= take->res[i];
		bank_copy.res[i] += give->res[i];
		player_copy.res[i] -= give->res[i];
		player_copy.res[i] += take->res[i];
	}
	if(good_flag == false){
		//Logger::getLog().log(Logger::DEBUG, "Model::bank_exchange failed");
		return false;
	}	

	// perform the transaction
	_players[player].resources = player_copy;
	_bank = bank_copy;
	/*Logger::getLog().log(Logger::DEBUG, "Model::bank_exchange new_bank=%d,%d,%d,%d,%d",
		_bank.res[0],
		_bank.res[1],
		_bank.res[2],
		_bank.res[3],
		_bank.res[4]);*/
	return true;
}

/*
@Parameter int A - one of players who is exchange
@Parameter resource_t* A_give - the amount of resources that player A is trading
@Parameter int B - one of the players in this exchange
@Parameter resource_t B_give - the amout of resources that player B is trading
@Return true for a successful transaction, false otherwise. The appropriate model error
code will be set if there is an error.
*/
bool Model::trade_with_player(int A, resource_t* A_give, int B, resource_t* B_give){
	if(get_player(A) == nullptr || get_player(B) == nullptr){ return false; }
	if(!A_give || !B_give){ return false; }
	Player* player_a = get_player(A);
	Player* player_b = get_player(B);
	// logging
	resource_t A_original = player_a->resources;
	resource_t B_original = player_b->resources;;

	// make sure that both players have enough resources to do the exchange.
	for(int i = 0; i < resource_t::NUM_OF_RESOURCES; ++i){
		if(player_a->resources.res[i] < A_give->res[i]){ return false; }
		if(player_b->resources.res[i] < B_give->res[i]){ return false; }

	}

	// make sure that after the exchange that the player isn't
	// left in an invalide state 
	for(int i = 0; i < resource_t::NUM_OF_RESOURCES; ++i){
		if(player_a->resources.res[i] + B_give->res[i] - A_give->res[i] < 0){ return false; }
		if(player_b->resources.res[i] + A_give->res[i] - B_give->res[i] < 0){ return false; }
	}

	// do the exchange	
	for(int i = 0; i < resource_t::NUM_OF_RESOURCES; ++i){
		player_a->resources.res[i] += B_give->res[i] - A_give->res[i];
		player_b->resources.res[i] += A_give->res[i] - B_give->res[i];
	}


	// print out some logging messages
	Logger::getLog().log(Logger::DEBUG, "Model::trade_with_player(playerA=%d,A_give,playerB=%d,B_give",A,B);
	for(int i = 0; i < resource_t::NUM_OF_RESOURCES; ++i){
		Logger::getLog().log(Logger::DEBUG, "[player=%d] resource %d = %d --> %d",
			A, i, A_original.res[i],player_a->resources.res[i]);
	}
	for(int i = 0; i < resource_t::NUM_OF_RESOURCES; ++i){
		Logger::getLog().log(Logger::DEBUG, "[player=%d] resource %d = %d --> %d",
			B, i, B_original.res[i], player_b->resources.res[i]);
	}
	return true;
}

/*
@Parameter int target_player - the player we are stealing from
@Parameter int taking_player - the player that is stealing from the target
@Parameter int number - the number of cards to steal from the target player
@Return -- return false if we failed to steal a resource.
			  reutrn true if we have stolen all the we possibly can from the target layer.
*/
bool Model::steal_random_resource_from_player(int target_player,int taking_player, int number){
	Player* get = get_player(target_player);
	Player* take = get_player(taking_player);
	if(get == nullptr){ return false; }
	if(take == nullptr){ return false; }

	while(number > 0){
		// no cards to steal from, that is acceptable
		if(get->get_hand_size() <= 0){ return true; }

		// make sure that that is a resource we can take from		
		int count = 0;
		int card = rand() % resource_t::NUM_OF_RESOURCES;
		while(get->resources.res[card] == 0 && count < resource_t::NUM_OF_RESOURCES){
			card = (card+1)%resource_t::NUM_OF_RESOURCES;
			count++;
		}
		// this should never happen?
		if(count >= resource_t::NUM_OF_RESOURCES){ return false; }

		// do the exchange
		take->add_resource(card, 1);
		get->add_resource(card, -1);				
		number--;
	}
	return true;
}


void Model::give_resources_from_roll(int roll){
	Tiles* hex = nullptr;
	int amount;
	int res_type;
	resource_t give;
	resource_t take;
	give.zero_out();

	for(int row = 0; row < _board_height; ++row){
		for(int col = 0; col < _board_width; ++col){
			// do not give resources if the tile contains a thief
			if(row == _thief_pos_y && col == _thief_pos_x){
				logger.log(Logger::DEBUG, "Model::give_resources_from_roll(roll=%d) Thief tile reached col=%d,row=%d",
					roll, _thief_pos_x,_thief_pos_y);
				continue;
			}

			hex = &_board[col + row*_board_width];
			if(hex->active == 0){ continue; }
			if(hex->roll != roll){ continue; }

			// check all 6 vertices for settlements/cities owned by players
			for(int i = 0; i < 6; ++i){
				take.zero_out();
				//if(get_vertex(hex->vertices[i]) == nullptr){ continue; }
				if(hex->vertices[i] < 0 || hex->vertices[i] >= (int)_vertex_array.size()){ continue; }
				if(_vertex_array[hex->vertices[i]].type == vertex_face_t::NONE){ continue; }

				amount = 0;
				res_type = 0;
				if(_vertex_array[hex->vertices[i]].type == vertex_face_t::CITY){
					amount = 2;
				} else if(_vertex_array[hex->vertices[i]].type == vertex_face_t::SETTLEMENT) {
					amount = 1;
				}

				switch(hex->type){
				case(Tiles::SHEEP_TILE) : res_type = resource_t::SHEEP; break;
				case(Tiles::ORE_TILE) : res_type = resource_t::ORE; break;
				case(Tiles::BRICK_TILE) : res_type = resource_t::BRICK; break;
				case(Tiles::WOOD_TILE) : res_type = resource_t::WOOD; break;
				case(Tiles::WHEAT_TILE) : res_type = resource_t::WHEAT; break;
				default:{continue; }
				}

				take.res[res_type] = amount;
				if(bank_exchange(_vertex_array[hex->vertices[i]].player, &give, &take)){
					Logger::getLog().log(
						Logger::DEBUG, 
						"Model::give_resource_from_roll col=%d,row=%d,type=%d amount=%d,player=%d",
						col, row, res_type, amount,
						_vertex_array[hex->vertices[i]].player);
				}
			}
			// we have now checked all the veritces for this tile
		}
	}
	return;
}

bool Model::buy_dev_card(int player){
	if(player < 0 || player >= _num_players){ return false; }
	if(_dev_deck.empty()){ return false; }
	// pay for the card
	resource_t price = get_building_cost(building_t::DEV_CARD);
	if(pay_for_item(player, &price) == false){
		return false;
	}

	const dev_cards_t* card = draw_dev_card();
	_players[player].dev_cards.push_back(card);
	return true;
}


// this is a transaction, and I need to make it atomic
bool Model::pay_for_item(int player, resource_t* price){
	if(!price){ return false; }
	resource_t take;
	for(int i = 0; i < resource_t::NUM_OF_RESOURCES; ++i){
		take.res[i] = 0;
	}

	bool rs = bank_exchange(player, price, &take);
	Logger::getLog().log(Logger::DEBUG, "Model::pay_for_item=%d", rs);
	return rs;
}

void Model::init_players(std::vector<Player>* players){
	Logger::getLog().log(Logger::DEBUG, "Model::init_players()");
	resource_t start_resources;
	start_resources.zero_out();
	for(int i = 0; i < resource_t::NUM_OF_RESOURCES; ++i){
		start_resources.res[i] = 0;
	}

	for(int i = 0; i < (int)players->size(); ++i){
		(*players)[i].init(
			"Player", { rand() % 256, rand() % 256, rand() % 256, 255 },
			0, start_resources, 0,
			_config.buildings_cap
			);
	}
}

int Model::get_largest_army_size(int player){
	if(player < 0 || player >= (int)_players.size()){ return -1; }
	return _players[player].num_soldiers;
}
int Model::get_longest_road_length(int player){
	if(player < 0 || player >= (int)_players.size()){ return -1; }
	return _players[player].longest_road;
}