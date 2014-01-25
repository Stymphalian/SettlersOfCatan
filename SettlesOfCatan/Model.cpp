#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <string>
#include <list>
#include <iostream>

#include "Logger.h"
#include "Util.h"
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

Model::Model(int num_players) : logger(Logger::getLog("jordan.log")){
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
Preconditions:
	The Model must be empty before using init()
	A call to set_defaults() before using init() is recommended.
*/
void Model::init(int num_players){	
	logger.log("Model::init(%d) ", num_players);
	srand((unsigned int)time(NULL));
	this->_model_error = MODEL_ERROR_NONE;
	this->_num_dice = 2;
	this->_num_dice_sides = 6;
	this->_thief_pos_x = -1;
	this->_thief_pos_y = -1;
	this->_roll_value = 0;
	this->_turn_count = 0;
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
	Logger& logger = Logger::getLog("jordan.log");
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
	logger.log(logger.DEBUG, "Model.fill_board() Random ordering of tile roll probability, board_tiles=%d,water_tiles=%d", num_game_tiles, num_water_tiles);
	while(roll_order_size < (num_game_tiles - num_water_tiles)){
		for(int i = num_dice; i <= num_dice*num_dice_sides; ++i){
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

				// assign the attaching faces
				t->face(0, hex->faces[(vpos + 1) % 6]); // most clockwise inner face
				t->face(1, hex->faces[(vpos + 5) % 6]); // least clockwise inner face

				if(t->tile_x(1) == -1){
					// if no neightbour is attached to the current face, 
					// let us check other adjacent tiles to see if faces
					// are attached
					t->face(2, -1); // just set to -1 to begin with.
					t->face(3, -1);

					// check if the most clockwise neighbour exists
					hex->get_adjacent((vpos + 1) % 6, col, row, &x, &y);
					if(x >= 0 && y >= 0 && x < board_width && y < board_height){
						other = &board[x + y*board_width];
						t->face(2, other->faces[(vpos + 5) % 6]);
					}

					// check if the least clockwise neighbour exists
					hex->get_adjacent((vpos + 5) % 6, col, row, &x, &y);
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
		//Logger::getLog("jordan.log").log(Logger::DEBUG, "level =%d,numtiles=%d,offset=%d,center=%d", level, numtiles,offset,center);

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

dev_cards_t Model::draw_dev_card(){
	if(_deck_pos >= (int)_dev_deck.size()){
		set_error(Model::MODEL_ERROR_BANK_DEV_CARD);
		dev_cards_t rs;
		return rs;
	} else{
		dev_cards_t rs = _dev_deck[_deck_pos];
		_deck_pos++;
		Logger::getLog("jordan.log").log(Logger::DEBUG, "Model::draw_dev_card %d of %d. title=%s,message=%s",
			_deck_pos, _dev_deck.size(), rs.title().c_str(), rs.message().c_str());
		return rs;
	}
}

bool Model::build_city(int player, int pos){
	if(pos < 0 || pos >= (int)_vertex_array.size() ||
		player < 0 || player >= _num_players)
	{
		Logger::getLog("jordan.log").log(Logger::ERROR, "Model::add_city() Out of range (player=%d/%d),(pos=%d/%d)", player, _num_players, pos, (int)_vertex_array.size());
		set_error(Model::MODEL_ERROR_PLACE_SETTLEMENT);
		return false;
	}
	if(_vertex_array[pos].type != vertex_face_t::SETTLEMENT ||
		_vertex_array[pos].type == vertex_face_t::CITY ||
		_vertex_array[pos].player != player)
	{
		Logger::getLog("jordan.log").log(Logger::DEBUG, "Model::add_city() Unsuitable Vertex type=%d,player=%d", (int)_vertex_array[pos].type, _vertex_array[pos].player);
		set_error(Model::MODEL_ERROR_PLACE_CITY);
		return false;
	}
	if(_players[player].building_cap[building_t::CITY] <= 0){
		Logger::getLog("jordan.log").log(Logger::DEBUG, "Model::add_city() player=%d Not enough buildings\n", player);
		set_error(Model::MODEL_ERROR_PLAYER_NOT_ENOUGH_BUILDINGS);
		return false;
	}

	// set the tile as a city now.
	set_vertex(player, pos, vertex_face_t::CITY);

	// TODO: Remove this.
	_players[player].building_cap[building_t::CITY]--;
	_players[player].building_cap[building_t::SETTLEMENT]++;
	//	m_players[player].buildings.push_back(pos);	 // don't need to push back because it is already there.
	Logger::getLog("jordan.log").log(Logger::DEBUG, "Model::add_city() player=%d,col=%d,row=%d,vertex=%d", player, _vertex_array[pos].tile_x(0),_vertex_array[pos].tile_y(0), pos);
	return true;
}

bool Model::build_settlement(int player, int pos){
	if(pos < 0 || pos >= (int)_vertex_array.size() ||
		player < 0 || player >= _num_players)
	{
		Logger::getLog("jordan.log").log(Logger::ERROR, "Model::add_settlement() Out of range (player=%d/%d),(pos=%d/%d)", player, _num_players, pos, (int)_vertex_array.size());
		set_error(Model::MODEL_ERROR_PLACE_SETTLEMENT);
		return false;
	}
	if(_vertex_array[pos].type == vertex_face_t::SETTLEMENT ||
		_vertex_array[pos].type == vertex_face_t::CITY)
	{
		Logger::getLog("jordan.log").log(Logger::DEBUG, "Model::add_settlement() Vertex is already occupied  by %d", (int)_vertex_array[pos].type);
		set_error(Model::MODEL_ERROR_PLACE_SETTLEMENT);
		return false;
	}
	if(_players[player].building_cap[building_t::SETTLEMENT] <= 0){
		Logger::getLog("jordan.log").log(Logger::DEBUG, "Model::add_settlement() Player %d does not have enough buildings", player);
		set_error(Model::MODEL_ERROR_PLAYER_NOT_ENOUGH_BUILDINGS);
		return false;
	}

	// find the tile associate with this vertex
	int col, row;
	Tiles* tile = _find_tile_from_vertex(pos, &col, &row);
	if(tile == nullptr){
		Logger::getLog("jordan.log").log(Logger::ERROR, "Model::add_settlement() Tile doesn't exist(col=%d,row=%d)", col, row);
		set_error(Model::MODEL_ERROR_PLACE_SETTLEMENT);
		return false;
	}

	// check for a connecting road, and space around the hex
	if(_can_build_settlement(player, pos)){
		set_vertex(player, pos, vertex_face_t::SETTLEMENT);
		
		// TODO: Remove this.
		_players[player].building_cap[building_t::SETTLEMENT]--;
		_players[player].buildings.push_back(pos);
		Logger::getLog("jordan.log").log(Logger::DEBUG, "Model::add_settlement player=%d,col=%d,row=%d,vertex=%d", player, col, row, pos);
		return true;
	}
	Logger::getLog("jordan.log").log(Logger::DEBUG, "Model::add_settlement() Unable to build settlement");
	set_error(Model::MODEL_ERROR_PLACE_SETTLEMENT);
	return false;
}


bool Model::build_road(int player, int pos){
	if(pos < 0 || pos >= (int)_face_array.size() ||
		player <0 || player >= _num_players)
	{
		Logger::getLog("jordan.log").log(Logger::ERROR, "Model::add_road() Out of bounds( player =%d/%d,pos =%d/%d)", player, _num_players, pos, (int)_face_array.size());
		set_error(Model::MODEL_ERROR_PLACE_ROAD);
		return false;
	}
	if(_face_array[pos].type != vertex_face_t::NONE)
	{
		Logger::getLog("jordan.log").log(Logger::DEBUG, "Model::add_road() Face is already filled with an object %d", _face_array[pos].type);
		set_error(Model::MODEL_ERROR_PLACE_ROAD);
		return false;
	}
	if(_players[player].building_cap[building_t::ROAD] <= 0){
		set_error(Model::MODEL_ERROR_PLAYER_NOT_ENOUGH_BUILDINGS);
		Logger::getLog("jordan.log").log(Logger::DEBUG, "Model::add_road()  Can't build road. Player %d doesn't have enough builidings", player);
		return false;
	}

	// check for a connecting road
	int col, row;
	Tiles* tile = _find_tile_from_face(pos, &col, &row);
	if(tile == nullptr){
		set_error(Model::MODEL_ERROR_PLACE_ROAD);
		Logger::getLog("jordan.log").log(Logger::ERROR, "Model::add_road()  Tile can't be found (col=%d,row=%d)", col, row);
		return false;
	}

	// if a road exists, then we can build a road
	if(_can_build_road(player, pos)){
		set_face(player, pos, vertex_face_t::ROAD);
		
		// TODO: Revmoe this.
		_players[player].building_cap[building_t::ROAD]--;
		_players[player].roads.push_back(pos);
		Logger::getLog("jordan.log").log(Logger::DEBUG, "Model::add_road player=%d,col=%d,row=%d,face=%d", player, col, row, pos);
		return true;
	}
	Logger::getLog("jordan.log").log(Logger::DEBUG, "Model::add_road()  Unable to build road.");
	set_error(Model::MODEL_ERROR_PLACE_ROAD);
	return false;
}

bool Model::set_vertex(int player, int vertex_key, int type){
	if(player < 0 || player >= _num_players){ return false; }
	if(vertex_key < 0 || vertex_key >= (int) _vertex_array.size()){ return false; }
	_vertex_array[vertex_key].type = (vertex_face_t::object_e) type;
	_vertex_array[vertex_key].player = player;
	return true;
}
bool Model::set_face(int player, int face_key, int type){
	if(player < 0 || player >= _num_players){ return false; }
	if(face_key < 0 || face_key >= (int)_face_array.size()){ return false; }
	_face_array[face_key].type = (vertex_face_t::object_e) type;
	_face_array[face_key].player = player;
	return true;
}

bool Model::_can_build_road(int player, int pos){
	if(player < 0 || player >= _num_players){ return false; }
	if(pos < 0 || pos >= (int)_face_array.size()){
		Logger::getLog("jordan.log").log(Logger::ERROR, "Model::can_build_road() pos is out of range %d/%d", pos, (int)_face_array.size());
		return false;
	}

	//vertex_face_t* face = &face_array[tile->faces[pos]];
	vertex_face_t* face = &_face_array[pos];
	bool road_exists;
	bool blocked;
	bool water_tiled;

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
	for(int i = 0; i < 4; ++i){
		if(face->face(i) == -1){ continue; }
		if(face->face(i) < 0 || face->face(i) >= (int)_face_array.size()){
			Logger::getLog("jordan.log").log(Logger::ERROR, "Model::can_build_road() %d/%d index out of range.", face->face(i), (int)_face_array.size());
			continue;
		}
		edge = &_face_array[face->face(i)];

		if(edge->type == vertex_face_t::ROAD && edge->player == player){
			road_exists = true;
			break;
		}
	}

	// check that there doesn't exist a city/settlement that
	// is blocking the placement of the road
	vertex_face_t* vert = nullptr;
	blocked = false;
	for(int i = 0; i < 2; ++i){
		if(face->vertex(i) == -1){ continue; }
		if(face->vertex(i) < 0 || face->vertex(i) >= (int)_vertex_array.size()){
			Logger::getLog("jordan.log").log(Logger::ERROR, "Model::can_build_road() %d/%d vertex index out of range", face->vertex(i), (int)_vertex_array.size());
			continue;
		}
		vert = &_vertex_array[face->vertex(i)];

		if((vert->type == vertex_face_t::CITY || vert->type == vertex_face_t::SETTLEMENT)
			&& vert->player != player)
		{
			blocked = true;
			break;
		}
	}

	// TODO: Remove this condition somewhere else.
	// handle the special case in which the player is placing their first roads.
	// This means that there doesn't 
	// 1) need to be an existing linked road.
	// 2) And that one of the attached vertices must be a player owned settlment.
	bool special_condition = true;
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

	return (road_exists && !blocked && special_condition && !water_tiled);
}
bool Model::_can_build_city(int player, int v){
	if(player < 0 || player >= _num_players){ return false; }
	if(v < 0 || v >= (int)_vertex_array.size()){
		Logger::getLog("jordan.log").log(Logger::ERROR, "Model::can_build_settlement() index out of range v=%d/%d", v, (int)_vertex_array.size());
		return false;
	}
	vertex_face_t* vertex = &_vertex_array[v];
	return (vertex->type == vertex_face_t::SETTLEMENT && vertex->player == player);
}

bool Model::_can_build_settlement(int player, int v){
	if(player <0 || player >= _num_players){ return false; }
	if(v < 0 || v >= (int)_vertex_array.size()){
		Logger::getLog("jordan.log").log(Logger::ERROR, "Model::can_build_settlement() index out of range v=%d/%d", v, (int)_vertex_array.size());
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
			Logger::getLog("jordan.log").log(Logger::ERROR, "Model::can_build_settlement() %d/%d vertex index out of range", vertex->vertex(i), (int)_vertex_array.size());
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
			Logger::getLog("jordan.log").log(Logger::ERROR, "Model::can_build_settlement() %d/%d face index out of range", vertex->face(i), (int)_face_array.size());
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
		Logger::getLog("jordan.log").log(Logger::ERROR, "Model::find_tile_from_vertex() %d out of range %d-%d",
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
		Logger::getLog("jordan.log").log(Logger::ERROR, "Model::find_tile_from_face() %d out of range %d-%d",
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
@Paramter int num -- the key to the vertex in which we wan to retrieve.
@Return -- return nullptr on failure
*/
vertex_face_t* Model::get_vertex(int num){
	if(num < 0 || num >= (int)_vertex_array.size()){
		set_error(Model::MODEL_ERROR_INVALID_VERTEX);
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
		set_error(Model::MODEL_ERROR_INVALID_FACE);
		return nullptr;
	}
	return &_face_array[num];
}

std::vector<vertex_face_t>& Model::get_vertex_array(){
	return _vertex_array;
}
std::vector<vertex_face_t>& Model::get_face_array(){
	return _face_array;
}

/*
@Paramter int col -- The column in which our target tile exists
@Paramter int row -- The Row in which our target tile exists
@Return -- return nullptr on faliure
*/
Tiles* Model::get_tile(int col, int row){
	if(col < 0 || row < 0 || col >= _board_width || row >= _board_height){
		set_error(Model::MODEL_ERROR_INVALID_TILE);
		return nullptr;
	}
	return &_board[col + row*_board_width];
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
	return _find_tile_from_vertex(vertex, c,r);
}

/*
@Return: A point to a dev_cards_t object.
*/
const dev_cards_t* Model::get_dev_card(){	
	const dev_cards_t* rs = (const dev_cards_t*) &_dev_deck[_deck_pos++];
	draw_dev_card();
	return rs;
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
int Model::get_player_with_longest_road(){ return _player_holding_longest_road_card; }
int Model::get_turn_count(){ return _turn_count; }
int Model::get_roll_value(){ return _roll_value;}
int Model::get_num_dice(){ return _num_dice; }
int Model::get_num_dice_sides(){ return _num_dice_sides; }
int Model::get_current_player(){ return _current_player; }
int Model::get_num_players(){ return _num_players; }
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

	// reset the modelerror code
	_model_error = Model::MODEL_ERROR_NONE;	

	// How do I handle the state information held by players?
	return true;
}

/*
increment and modulos the _current_player
increment the _turn_count variable.
*/
void Model::end_turn(){
	_current_player = (_current_player + 1) % _num_players;
	++_turn_count;
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
	Logger::getLog("jordan.log").log(Logger::DEBUG, "Model::move_thief(col=%d,row=%d)", _thief_pos_x, _thief_pos_y);
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
	bool rs = false;
	switch(building) {
	case(building_t::CITY) : rs = build_city(player, pos); break;
	case(building_t::SETTLEMENT) : rs = build_settlement(player, pos); break;
	case(building_t::ROAD) : rs = build_road(player, pos); break;
	}

	if(rs == true){
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
	Logger::getLog("jordan.log").log(Logger::DEBUG, "Model::roll(dice=%d,num sides=%d) = %d",
												num_dice, num_sides,_roll_value);
	return _roll_value;
}



/*
TODO Create a player interface which will accomplish these tasks
*/
Player* Model::get_player(int player){
	if(player < 0 || player >= _num_players){
		set_error(Model::MODEL_ERROR_ERROR);
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
		if(p.dev_cards[i].visible == false || p.dev_cards[i].type == dev_cards_t::VICTORY){
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
	Logger::getLog("jordan.log").log(Logger::DEBUG, "Model::bank_exchange player=%d give=%d,%d,%d,%d,%d take=%d,%d,%d,%d,%d",
		player,
		give->res[0], give->res[1], give->res[2], give->res[3], give->res[4],
		take->res[0], take->res[1], take->res[2], take->res[3], take->res[4]
		);

	resource_t player_copy = _players[player].resources;
	resource_t bank_copy = _bank;
	bool good_flag = true;;
	Logger::getLog("jordan.log").log(Logger::DEBUG, "Model::bank_exchange old_bank=%d,%d,%d,%d,%d",
		_bank.res[0], _bank.res[1], _bank.res[2], _bank.res[3], _bank.res[4]);

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
		Logger::getLog("jordan.log").log(Logger::DEBUG, "Model::bank_exchange failed");
		return false;
	}	

	// perform the transaction
	_players[player].resources = player_copy;
	_bank = bank_copy;
	Logger::getLog("jordan.log").log(Logger::DEBUG, "Model::bank_exchange new_bank=%d,%d,%d,%d,%d",
		_bank.res[0],
		_bank.res[1],
		_bank.res[2],
		_bank.res[3],
		_bank.res[4]);
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
			if(row == _thief_pos_y && col == _thief_pos_x){ continue; }

			hex = &_board[col + row*_board_width];
			if(hex->active == 0){ continue; }
			if(hex->roll != roll){ continue; }

			// check all 6 vertices for settlements/cities owned by players
			for(int i = 0; i < 6; ++i){
				take.zero_out();
				if(get_vertex(hex->vertices[i]) == nullptr){ continue; }
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
				bank_exchange(_vertex_array[hex->vertices[i]].player, &give, &take);
				Logger::getLog("jordan.log").log(Logger::DEBUG, "Model::give_resoure_from_roll col=%d,row=%d,type=%d amount=%d,player=%d",
					col, row, res_type, amount, _vertex_array[hex->vertices[i]].player);
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

	dev_cards_t card = draw_dev_card();
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
	Logger::getLog("jordan.log").log(Logger::DEBUG, "Model::pay_for_item=%d", rs);
	return rs;
}

void Model::init_players(std::vector<Player>* players){
	Logger::getLog("jordan.log").log(Logger::DEBUG, "Model::init_players()");
	resource_t start_resources;
	start_resources.zero_out();

	for(int i = 0; i < (int)players->size(); ++i){
		(*players)[i].init(
			"Player", { rand() % 256, rand() % 256, rand() % 256, 255 },
			0, start_resources, 0,
			_config.buildings_cap
			);
	}
}
