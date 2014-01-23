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

 
Model::Model(int num_players){
	Logger::getLog("jordan.log").log(Logger::DEBUG, "Model Constructor(num_players=%d)", num_players);
	// seed the random number generator
	srand((unsigned int)time(NULL));

	// Get the number of levels and number of extensions
	// We need this to set the number of tiles,and resources we have for this game
	m_even_middle_row = false;
	if(num_players <= 4){
		m_num_extensions = 0;
		m_num_levels = 4;
	} else{
		m_num_extensions = config.num_of_extensions(num_players);		
		if((m_num_extensions + 5) % 2 == 0){
			m_even_middle_row = true; 
			m_num_levels = 1 + ( m_num_extensions + 5) / 2 ; // + 1 for water tiles
		} else{
			m_num_levels = 1 + 1+ (m_num_extensions + 5) / 2; //  +1 for water tiles
		}
	}
	Logger::getLog("Jordan.log").log(Logger::DEBUG, "Model Constructor: num_levels=%d,num_extensions=%d,even_middle_row=%d", m_num_levels,m_num_extensions,m_even_middle_row);

	// number of dice and num of sides per die
	num_dice = 2;
	num_dice_sides = 6;

	// Expand the number of tiles,resource, and cards if necessary
	config.apply_defaults();
	config.apply_extensions(m_num_extensions);
	for(int i = 0; i < resource_t::NUM_OF_RESOURCES; ++i){
		bank.res[i] = config.resource_cards_cap[i];
	}
	config.print_configuration_to_log();

	// set the model phase
	roll_value = 0;

	// Create all the players
	this->m_num_players = num_players;
	m_current_player = 0;
	this->m_players = new Player[num_players]();
	init_players(m_players,m_num_players);

	// set-up the game board
	establish_board_dimensions(num_players);
	m_board = new Tiles[m_board_size];
	fill_board(m_board, m_board_size, m_num_levels);

	// Setup the dev cards deck
	fill_deck(config.dev_cards_cap,dev_cards_t::NUM_OF_DEV_CARDS);

	// Setup the thief position
	thief_pos_x = 0;
	thief_pos_y = 0;

	// default no players own the longest road, or largest army cards
	player_holding_largest_army_card = -1;
	player_holding_longest_road_card = -1;

	turn_count = 0;
	set_error(Model::MODEL_ERROR_NONE);
}
Model::~Model(){
	Logger::getLog("jordan.log").log(Logger::DEBUG,"Model Destructor");
	delete[] m_board;
	delete[] m_players;
	m_dev_deck.clear();
	vertex_array.clear();
	face_array.clear();
}

void Model::establish_board_dimensions(int num_players){
	Logger::getLog("jordan.log").log(Logger::DEBUG, "Model::establish_board_dimensions(num_player=%d)", num_players);
	// Get the size of the board, and length of the longest row in the board
	if(m_even_middle_row == false){
		m_board_width = 2 * m_num_levels - 1;
		m_board_height = m_board_width;
		m_board_size = m_board_width*m_board_width;
		m_num_game_tiles = 3 * m_num_levels*m_num_levels - 3 * m_num_levels + 1;  // 6(n(n+1)/2) +1 for n >= 2, where n is the number of levels
		m_num_water_tiles = 6 * (m_num_levels - 1);
	} else{
		m_board_width = m_num_levels * 2;
		m_board_height = m_board_width + 1;
		m_board_size = m_board_width*m_board_height;
		m_num_game_tiles = 3*(m_num_levels*m_num_levels) + m_num_levels;
		m_num_water_tiles = 6*(m_num_levels) - 2;
	}
	Logger::getLog("Jordan.log").log(Logger::DEBUG, "Model::establish_board_dimensions m_board_width=%d,m_board_height=%d", m_board_width,m_board_height);
	Logger::getLog("Jordan.log").log(Logger::DEBUG, "Model::establish_board_dimensions m_board_size=%d", m_board_size);
	Logger::getLog("Jordan.log").log(Logger::DEBUG, "Model::establish_board_dimensions m_num_game_tiles=%d", m_num_game_tiles);
	Logger::getLog("Jordan.log").log(Logger::DEBUG, "Model::establish_board_dimensions m_num_water_tiles=%d", m_num_water_tiles);
}

void Model::init_players(Player* players, int num_players){
	Logger::getLog("jordan.log").log(Logger::DEBUG, "Model::init_players()");
	resource_t start_resources;
	start_resources.zero_out();
	for(int i = 0; i < num_players; ++i){
		players[i].init(
			"Player", {rand()%256,rand()%256,rand()%256,255},
			0,start_resources,0,
			config.buildings_cap
		);
	}
}

bool Model::fill_board(Tiles* board, int size,int num_levels){
	if(!board){ return false; }

	// mark which tiles are active
	Logger& logger = Logger::getLog("jordan.log");
	logger.log(Logger::DEBUG, "Model.fill_board(board,size=%d,num_level=%d)", size, num_levels);
	int offset = 0;
	int num_tiles = 0;
	int width = m_board_width;	
	int height = m_board_height;
	int active_count = 0;
	for(int row = 0; row < height; ++row){		
		num_tiles = numtiles_from_row(row, num_levels,m_num_extensions);
		offset = num_offset_tiles_from_row(row, num_levels,m_num_extensions);
		logger.log(Logger::DEBUG, "Model::fill_board() offset=%d,num_tiles=%d",offset,num_tiles);
		
		// mark tiles as active
		for(int col = 0; col < num_tiles; ++col){
			++active_count;
			board[col + offset + row*width].active = 1;
			board[col + offset + row*width].reset_vertices_faces();			
		}
	}
	logger.log(Logger::DEBUG, "Model.fill_board() active_count = %d", active_count);

	// for each active tile choose a tile to place
	// get an ordering of all the tiles
	logger.log(logger.DEBUG, "Model.fill_board() Setting up the tile ordering. num_game_tiles=%d", m_num_game_tiles);
	logger.log(logger.DEBUG, "Model.fill_board() Setting up the water_tile ordering. num_water_tiles=%d", m_num_water_tiles);
	logger.log(logger.DEBUG, "Model.fill_board() Setting up the roll ordering. number of roll tokens=%d", m_num_game_tiles - m_num_water_tiles);
	int* tile_order = new int[m_num_game_tiles];
	int* water_tile_order = new int[m_num_water_tiles];
	int* roll_order = new int[m_num_game_tiles - m_num_water_tiles];
	int tile_order_size = 0;
	int roll_order_size = 0;
	int water_tile_order_size = 0;
	
	// normal game tiles
	for(int i = 0; i < Tiles::NUM_OF_TILES; ++i){
		for(int j = 0; j < config.num_tiles[i]; ++j){
			if(Tiles::is_water_tile(i)){
				if(water_tile_order_size >= m_num_water_tiles){ continue; }
				water_tile_order[water_tile_order_size++] = i;
			} else{
				if(tile_order_size >= m_num_game_tiles){ continue; }
				tile_order[tile_order_size++] = i;
			}
		}
	}

	// add the additional water tiles
	while(water_tile_order_size < m_num_water_tiles){
		water_tile_order[water_tile_order_size++] = Tiles::WATER_TILE;
	}

	// Random order for the roll values on each tile
	logger.log(logger.DEBUG, "Model.fill_board() Random ordering of tile roll probability, board_tiles=%d,water_tiles=%d", m_num_game_tiles ,m_num_water_tiles);	
	while(roll_order_size < (m_num_game_tiles - m_num_water_tiles)){
		for(int i = num_dice; i <= num_dice*num_dice_sides; ++i){
			if(roll_order_size >= (m_num_game_tiles - m_num_water_tiles)){ break; }
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
	for(int row = 0; row < height; ++row){
		for(int col = 0; col < width; ++col){
			if(board[col + row*width].active == 0){ continue; }
			count++;
			
			hex = &board[col + row*width];
			hex->active = 1;
			hex->ring_level = get_ring_level(row, col, m_num_levels);
			hex->type = Tiles::WATER_TILE; // use a default tile, just in case.

			// determine the type and roll value of the tile
			
			if(hex->ring_level == m_num_levels-1){				
				// place water tiles on the outer ring
				if(water_tile_pos >= water_tile_order_size){ 
					logger.log(Logger::ERROR, "Model.fill_board() More water tiles than expected. Capacity for only %d tiles.", water_tile_order_size);
					continue;
				}
				hex->type = water_tile_order[water_tile_pos++];					
				hex->roll = 0;
			} else{
				// place normal tiles
				if(tile_pos >= tile_order_size) { 
					logger.log(Logger::ERROR, "Model.fill_board() More tiles than expected. Capacity for only %d tiles.", tile_order_size);
					break;
				}
				if(roll_pos >= roll_order_size) {
					logger.log(Logger::ERROR, "Model.fill_board() More roll tokens than expected. Capacity for only %d tokens.", roll_order_size);
					break;
				}
				hex->type = tile_order[tile_pos++];

				// the desert tile should not use a roll value
				if(hex->type == Tiles::DESERT_TILE){
					hex->roll = 0;
				} else {
					hex->roll = roll_order[roll_pos++];
				}
			}
			logger.log(Logger::DEBUG, "Tile [%d] col=%d,row=%d ring_level=%d,type=%d,roll=%d", count,col, row, hex->ring_level,hex->type,hex->roll);

			// assign the vertices to the tile
			Tiles *other=nullptr,*other2=nullptr;
			int x, y;
			for(int i = 0; i < 6; ++i){
				// the weird ternary expression is for the special case where we wrap around the clock
				// get the least clock wise neighbour
				hex->get_adjacent((i==0)?5:i-1,col, row, &x, &y);
				other2 = (x < 0 || y < 0 || x >= width || y >= width)
					? nullptr : &board[x + y*width];
		
				// get the most clockwise neighbour
				hex->get_adjacent(i, col, row, &x, &y);
				other = nullptr;
				other = (x < 0 || y < 0 || x >= width || y >= width)
					? nullptr : &board[x + y*width];
											
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
				} 
				else
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
				other = (x < 0 || y < 0 || x >= width || y >= height)
							? nullptr : &board[x + y*width];
			
				// check the neighbouring tile to see if a face node is already assigned
				if(other!= nullptr &&
					other->active == 1 &&
					other->faces[(i + 3) % 6] != -1)
				{					
					// face already exists, therefore use that one
					hex->faces[i] = other->faces[(i + 3) % 6];
				} 
				else
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

	Logger::getLog("jordan.log").log(Logger::DEBUG, "sizeof(vector)%d",sizeof(std::vector<int>));
	logger.log(Logger::DEBUG, "Model.fill_board() face_array.size() = %d", face_array.size());
	logger.log(Logger::DEBUG, "Model.fill_board() vertex_array.size() = %d", vertex_array.size());
	logger.log(Logger::DEBUG, "Model.fill_board() tiles_pos=%d,water_tiles_pos=%d,roll_pos=%d", tile_pos, water_tile_pos, roll_pos);
	logger.log(logger.DEBUG, "Model.fill_board() Assignment of face and vertex nodes");
	count = 0;
	for(int row = 0; row < m_board_height; ++row){
		for(int col = 0; col < m_board_width; ++col){
			if(board[col + row*width].active == 1){
				count++;
				Tiles& hex = board[col + row*width];
				logger.log(logger.DEBUG, "[%d] row=%d,col=%d face={%d,%d,%d,%d,%d,%d} vertices={%d,%d,%d,%d,%d,%d}",
							count,row, col,
							hex.faces[0], hex.faces[1], hex.faces[2], 
							hex.faces[3], hex.faces[4], hex.faces[5],
							hex.vertices[0], hex.vertices[1], hex.vertices[2],
							hex.vertices[3], hex.vertices[4], hex.vertices[5]
							);
			}
		}
	}

	// fill the vertex_face_t in vertex_arry and face_array
	// with the pointer to their connected tiles, vertices and faces
	fill_vertex_face_arrays();

	// Cleanup
	delete[] tile_order;
	delete[] roll_order;
	delete[] water_tile_order;
	return true;
}



bool Model::fill_deck(int* default_cards, int size){
	if(!default_cards){ return false; }
	Logger& logger = Logger::getLog("Jordan.log");
	logger.log(logger.DEBUG,"Model::fill_deck(default_cards,size=%d)", size);

	// find the number of cards in the deck
	int deck_size = 0;
	for(int i = 0; i < size; ++i){
		deck_size += default_cards[i];
	}
	logger.log(logger.DEBUG, "Model::fill_deck() deck_size=%d",deck_size);

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
		m_dev_deck.push_back(dev_card);
		logger.log(logger.DEBUG, "Model::fill_deck() [%d] title=%s", m_dev_deck.size(), dev_card.title().c_str());
		logger.log(logger.DEBUG, "Model::fill_deck() [%d] message=%s", m_dev_deck.size(), dev_card.message().c_str());
	}
	logger.log(logger.DEBUG, "Model::fill_deck() m_dev_deck.size()=%d", m_dev_deck.size());

	// cleanup
	delete[] random_cards;
	delete[] victory_titles;
	return true;
}

int Model::get_ring_level(int row, int col,int levels){
	int dy, numtiles, offset, border_len, center, l, t, x=0;	

	if( m_even_middle_row == true){
		int level = level_from_row(row, levels, m_num_extensions);
		numtiles = numtiles_from_row(row, levels, m_num_extensions);
		offset = num_offset_tiles_from_row(row, levels, m_num_extensions);		
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
			dy = level_from_row(row, levels, m_num_extensions);
			numtiles = numtiles_from_row(row, levels, m_num_extensions);
			offset = num_offset_tiles_from_row(row, levels, m_num_extensions);
			border_len = abs(numtiles - levels);
			center = offset + numtiles / 2;
			l = abs(center - offset);
			t = l - border_len;
			if(numtiles % 2 == 0 && col >= center){
				l--; t--;
			}
			x = abs(center - col);

			if(x < t){
				return dy-1;
			} else { return dy + x - t-1; }
		}
		return -1;		
	} else{
		// special case
		if(row == levels - 1){ return abs(levels - 1 - col); }
		dy = abs(levels - 1 - row);
		numtiles = numtiles_from_row(row, levels, m_num_extensions);
		offset = num_offset_tiles_from_row(row, levels, m_num_extensions);
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
int Model::num_offset_tiles_from_row(int row, int level,int num_extensions){	
	int offset = level_from_row(row, level,num_extensions) / 2;
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
int Model::level_from_row(int row, int levels,int num_extensions){
	if((num_extensions != 0) && (5 + num_extensions) % 2 == 0){
		return abs(row - levels);
	} else{
		return abs(levels - 1 - row);
	}
}

// Tells you how many tiles there are on this row given the number of levels there are
int Model::numtiles_from_row(int row, int level,int num_extensions){
	if((num_extensions != 0) && (5 + num_extensions) % 2 == 0){
		return 2 * level - level_from_row(row, level, num_extensions);
	} else{		
		return 2 * level - 1 - level_from_row(row, level, num_extensions);
	}
}

resource_t Model::get_building_cost(building_t::buildings card){
	if(card < 0 || card >= building_t::NUM_OF_BUILDINGS){
		set_error(Model::MODEL_ERROR_ERROR);
		return config.building_costs[0];
	}
	return config.building_costs[card];
}

int Model::roll(int num_dice,int num_sides=6){
	roll_value = 0;
	for(int i = 0; i < num_dice; ++i){
		roll_value += rand() % num_sides + 1;
	}
	Logger::getLog("jordan.log").log(Logger::DEBUG, "Model::roll(dice=%d,num sides=%d) = %d",
												num_dice, num_sides,roll_value);
	return roll_value;
}

int Model::num_possiblities_dice(int sum, int num_dice, int num_sides){
	int limit = (sum - num_dice) / num_sides;
	int count = 0;
	int temp = 0;
	for(int k = 0; k < limit; ++k){
		temp *= (k % 2 == 0) ? 1 : -1;
		temp *= M_math::get().combination(num_dice,k);
		temp *= M_math::get().combination(sum - num_sides*k - 1, num_dice - 1);
		count += temp;
	}
	return count;
}

void Model::end_turn(){
	m_current_player =( m_current_player + 1) % m_num_players;	
	++turn_count;
}


void Model::play_dev_card(int player, dev_cards_t* card){
	if(!card){ return; }
	card->visible = true;
	
	switch(card->type){
		case(dev_cards_t::SOLDIER):{
			m_players[player].num_soldiers++;
			if( player_holding_largest_army_card  == -1 &&
				m_players[player].num_soldiers >= 3 ) 
			{
				player_holding_largest_army_card = player;
			}else if( m_players[player].num_soldiers >
						m_players[player_holding_largest_army_card].num_soldiers) 
			{
				transfer_largest_army_card(player_holding_largest_army_card,player);
			}			
		}break;
		case(dev_cards_t::VICTORY):{
			
		}break;
		case(dev_cards_t::MONOPOLY):{

		}break;
		case(dev_cards_t::YEAR_PLENTY):{

		}break;
		case(dev_cards_t::ROAD_BUILDING):{

		}break;
		default: {}
	}
}

void Model::give_resources_from_roll(int roll){
	Tiles* hex = nullptr;
	int amount;
	int res_type;
	resource_t give;
	resource_t take;
	give.zero_out();

	for(int row = 0; row < m_board_height; ++row){
		for(int col = 0; col < m_board_width; ++col){
			// do not give resources if the tile contains a thief
			if(row == thief_pos_y && col == thief_pos_x){ continue; }

			hex = &m_board[col + row*m_board_width];
			if(hex->active == 0){ continue; }
			if(hex->roll != roll){ continue; }

			// check all 6 vertices for settlements/cities owned by players
			for(int i = 0; i < 6; ++i){
				take.zero_out();
				if(get_vertex(hex->vertices[i]) == nullptr){ continue; }
				if(vertex_array[hex->vertices[i]].type == vertex_face_t::NONE){ continue; }					

				amount = 0;
				res_type = 0;
				if(vertex_array[hex->vertices[i]].type == vertex_face_t::CITY){
					amount = 2;
				} else if(vertex_array[hex->vertices[i]].type == vertex_face_t::SETTLEMENT) {
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
				bank_exchange(vertex_array[hex->vertices[i]].player, &give, &take);				
				Logger::getLog("jordan.log").log(Logger::DEBUG, "Model::give_resoure_from_roll col=%d,row=%d,type=%d amount=%d,player=%d",
					col, row, res_type, amount, vertex_array[hex->vertices[i]].player);
			}
			// we have now checked all the veritces for this tile
		}
	}
	return;
}


bool Model::buy_dev_card(int player){
	if(player < 0 || player >= m_num_players){return false;}
	if(m_dev_deck.empty()){ return false; }
	// pay for the card
	resource_t price = get_building_cost(building_t::DEV_CARD);
	if(pay_for_item(player, &price) == false){
		return false;
	}

	dev_cards_t card =  draw_dev_card();
	m_players[player].dev_cards.push_back(card);
	return true;
}

dev_cards_t Model::draw_dev_card(){
	if(m_dev_deck.empty()){
		set_error(Model::MODEL_ERROR_BANK_DEV_CARD);
		dev_cards_t rs;
		return rs;
	} else{
		dev_cards_t rs = m_dev_deck.back();
		m_dev_deck.pop_back();
		Logger::getLog("jordan.log").log(Logger::DEBUG, "Model::draw_dev_card %d remaining. title=%s,message=%s",
			m_dev_deck.size(), rs.title().c_str(), rs.message().c_str());
		return rs;
	}
}

bool Model::build_building(int player,building_t::buildings building,int pos){
	bool rs = false;
	switch(building) {
		case(building_t::CITY) :rs=add_city(player,pos);break;
		case(building_t::SETTLEMENT) : rs=add_settlement(player, pos); break;
		case(building_t::ROAD) : rs=add_road(player, pos); break;
	}

	if(rs == true){
		// don't need to pay if you are placing the first couple of buildings.
		if((int)get_player(player)->buildings.size()-1 < 2 &&
			(building == building_t::SETTLEMENT || building == building_t::CITY)){		
			return true;
		}		
		if((int)get_player(player)->roads.size()-1 < 2 && building == building_t::ROAD){
			return true;
		}
		
		// pay the necessary resources
		resource_t price = get_building_cost(building);
		pay_for_item(player, &price);
		return true;
	}
	return false;
}


bool Model::add_city(int player, int pos){	
	if(pos < 0 || pos >= (int)vertex_array.size() ||
		player < 0 || player >= m_num_players)
	{
		Logger::getLog("jordan.log").log(Logger::ERROR, "Model::add_city() Out of range (player=%d/%d),(pos=%d/%d)", player, m_num_players, pos, (int)vertex_array.size());
		set_error(Model::MODEL_ERROR_PLACE_SETTLEMENT);
		return false;
	}
	if(vertex_array[pos].type != vertex_face_t::SETTLEMENT ||
		vertex_array[pos].type == vertex_face_t::CITY ||
		vertex_array[pos].player != player)
	{
		Logger::getLog("jordan.log").log(Logger::DEBUG, "Model::add_city() Unsuitable Vertex type=%d,player=%d", (int)vertex_array[pos].type, vertex_array[pos].player);
		set_error(Model::MODEL_ERROR_PLACE_CITY);
		return false;
	}
	if(m_players[player].building_cap[building_t::CITY] <= 0){ 
		Logger::getLog("jordan.log").log(Logger::DEBUG, "Model::add_city() player=%d Not enough buildings\n",player);
		set_error(Model::MODEL_ERROR_PLAYER_NOT_ENOUGH_BUILDINGS);
		return false;	
	}
	
	// set the tile as a city now.
	vertex_array[pos].type = vertex_face_t::CITY;
	m_players[player].building_cap[building_t::CITY]--;
	m_players[player].building_cap[building_t::SETTLEMENT]++;
//	m_players[player].buildings.push_back(pos);	 // don't need to push back because it is already there.
	Logger::getLog("jordan.log").log(Logger::DEBUG, "Model::add_city() player=%d,col=%d,row=%d,vertex=%d", player, vertex_array[pos].vert.tiles[0][0], vertex_array[pos].vert.tiles[0][1], pos);
	return true;
}
bool Model::add_settlement(int player, int pos){	
	if(pos < 0 || pos >= (int)vertex_array.size() ||
		player < 0 || player >= m_num_players)
	{
		Logger::getLog("jordan.log").log(Logger::ERROR, "Model::add_settlement() Out of range (player=%d/%d),(pos=%d/%d)",player,m_num_players,pos,(int)vertex_array.size());
		set_error(Model::MODEL_ERROR_PLACE_SETTLEMENT);
		return false;
	}
	if(vertex_array[pos].type == vertex_face_t::SETTLEMENT ||
		vertex_array[pos].type == vertex_face_t::CITY)
	{
		Logger::getLog("jordan.log").log(Logger::DEBUG, "Model::add_settlement() Vertex is already occupied  by %d", (int)vertex_array[pos].type);
		set_error(Model::MODEL_ERROR_PLACE_SETTLEMENT);
		return false;
	}
	if(m_players[player].building_cap[building_t::SETTLEMENT] <= 0){ 
		Logger::getLog("jordan.log").log(Logger::DEBUG, "Model::add_settlement() Player %d does not have enough buildings",player);
		set_error(Model::MODEL_ERROR_PLAYER_NOT_ENOUGH_BUILDINGS);
		return false;
	}
	
	// find the tile associate with this vertex
	int col, row;
	Tiles* tile = find_tile_from_vertex(pos,&col,&row);
	if(tile == nullptr){ 
		Logger::getLog("jordan.log").log(Logger::ERROR, "Model::add_settlement() Tile doesn't exist(col=%d,row=%d)", col, row);
		set_error(Model::MODEL_ERROR_PLACE_SETTLEMENT);
		return false;
	}

	// check for a connecting road, and space around the hex
	if(can_build_settlement(player, row, col, pos)){
		vertex_array[pos].type = vertex_face_t::SETTLEMENT;
		vertex_array[pos].player = player;
		m_players[player].building_cap[building_t::SETTLEMENT]--;
		m_players[player].buildings.push_back(pos);
		Logger::getLog("jordan.log").log(Logger::DEBUG, "Model::add_settlement player=%d,col=%d,row=%d,vertex=%d",player,col,row,pos );
		return true;
	}
	Logger::getLog("jordan.log").log(Logger::DEBUG, "Model::add_settlement() Unable to build settlement");
	set_error(Model::MODEL_ERROR_PLACE_SETTLEMENT);
	return false;
}
bool Model::add_road(int player, int pos){
	if(pos < 0 || pos >= (int)face_array.size() ||
		player <0 || player >= m_num_players)
	{
		Logger::getLog("jordan.log").log(Logger::ERROR, "Model::add_road() Out of bounds( player =%d/%d,pos =%d/%d)", player,m_num_players, pos,(int)face_array.size());
		set_error(Model::MODEL_ERROR_PLACE_ROAD);
		return false;
	}
	if(face_array[pos].type != vertex_face_t::NONE)
	{ 
		Logger::getLog("jordan.log").log(Logger::DEBUG, "Model::add_road() Face is already filled with an object %d", face_array[pos].type);
		set_error(Model::MODEL_ERROR_PLACE_ROAD);
		return false;
	}
	if(m_players[player].building_cap[building_t::ROAD] <= 0){
		set_error(Model::MODEL_ERROR_PLAYER_NOT_ENOUGH_BUILDINGS);
		Logger::getLog("jordan.log").log(Logger::DEBUG, "Model::add_road()  Can't build road. Player %d doesn't have enough builidings", player);
		return false;
	}
	
	// check for a connecting road
	int col, row;
	Tiles* tile = find_tile_from_face(pos, &col, &row);;
	if(tile == nullptr){ 
		set_error(Model::MODEL_ERROR_PLACE_ROAD);
		Logger::getLog("jordan.log").log(Logger::ERROR, "Model::add_road()  Tile can't be found (col=%d,row=%d)", col, row);
		return false;
	}

	// if a road exists, then we can build a road
	if(can_build_road(player,row,col,pos)){
		face_array[pos].type = vertex_face_t::ROAD;
		face_array[pos].player = player;
		m_players[player].building_cap[building_t::ROAD]--;
		m_players[player].roads.push_back(pos);
		Logger::getLog("jordan.log").log(Logger::DEBUG, "Model::add_road player=%d,col=%d,row=%d,face=%d", player, col, row, pos);
		return true;
	}
	Logger::getLog("jordan.log").log(Logger::DEBUG, "Model::add_road()  Unable to build road.");
	set_error(Model::MODEL_ERROR_PLACE_ROAD);
	return false;
}


bool Model::fill_vertex_face_arrays(){
	Logger& logger = Logger::getLog("jordan.log");
	logger.log(Logger::DEBUG, "Model::fill_vertex_face_arrays() filling the face and vertex arrays");
	Tiles* hex;
	Tiles* other = 0;
	vertex_face_t* t;	
	int x, y;
	for(int row=0; row < m_board_height; ++row){
		for(int col=0; col < m_board_width; ++col){
			hex = &m_board[col + row*m_board_width];
			if(hex->active == 0){ continue; }

			// handle vertices
			for(int v=0; v < 6; ++v){
				if(hex->vertices[v] >= (int) vertex_array.size() || hex->vertices[v] == -1){
					logger.log(Logger::DEBUG, "Model::fill_vertex_face_array() vertex_array index %d out of bounds %d", hex->vertices[v], vertex_array.size());
					continue;
				}
				t = &vertex_array[hex->vertices[v]];
				if(t->is_assigned == true){ continue; }
				t->is_assigned = true;

				// assign the hex tiles
					// assign the current tile to index 0
				t->vert.tiles[0][0] = col;
				t->vert.tiles[0][1] = row;
					// assign the most clockwise tile to index 1
				hex->get_adjacent(v, col, row, &x, &y);
				if(get_tile(x,y) == nullptr || get_tile(x,y)->active == false ){
					x = y =-1;
				}
				t->vert.tiles[1][0] = x;
				t->vert.tiles[1][1] = y;
					// assign the least clockwise tile to index 2
				hex->get_adjacent((v+5)%6, col, row, &x, &y);
				if(get_tile(x, y) == nullptr || get_tile(x, y)->active == false){
					x = y = -1;
				}
				t->vert.tiles[2][0] = x;
				t->vert.tiles[2][1] = y;


				// assign the vertices and faces
				t->vert.vertices[0] = hex->vertices[(v + 1) % 6];
				t->vert.vertices[1] = hex->vertices[(v + 5) % 6];
				t->vert.faces[0] = hex->faces[v];
				t->vert.faces[1] = hex->faces[(v + 5) % 6];

				if(t->vert.tiles[1][0] != -1){
					// using the most clockwise tile to find the vertices and faces
					other = &m_board[t->vert.tiles[1][0] + t->vert.tiles[1][1] * m_board_width];
					t->vert.vertices[2] = other->vertices[(v + 5) % 6];
					t->vert.faces[2] = other->faces[(v + 4) % 6];
				} else if( t->vert.tiles[2][0] != -1){
					// using the least clockwise tile to find the vertices and faces
					other = &m_board[t->vert.tiles[2][0] + t->vert.tiles[2][1] * m_board_width];
					t->vert.vertices[2] = other->vertices[(v + 1) % 6];
					t->vert.faces[2] = other->faces[(v + 2) % 6];
				} else{
					// the vertex exists only if both the neighbour tiles exists
					t->vert.vertices[2] = -1; //no vertex, 
					t->vert.faces[2] = -1; // no face
				}
			}

			// handling the faces
			for(int f = 0; f < 6; ++f){
				if(hex->faces[f] >= (int) face_array.size() || hex->faces[f] == -1){
					logger.log(Logger::DEBUG, "Model::fill_vertex_face_array() face_array index %d out of bounds %d", hex->faces[f], face_array.size());
					continue;
				}
				t = &face_array[hex->faces[f]];
				if(t->is_assigned){ continue; }
				t->is_assigned = true;

				// assign the attching tiles
				t->face.tiles[0][0] = col;
				t->face.tiles[0][1] = row;
					// get the tile sharing the current face
				hex->get_adjacent(f, col, row, &x, &y);
				if(get_tile(x, y) == nullptr || get_tile(x, y)->active == false){
					x = y = -1;
				}
				t->face.tiles[1][0] = x;
				t->face.tiles[1][1] = y;

				// assign the vertices 
				t->face.vertices[0] = hex->vertices[f];
				t->face.vertices[1] = hex->vertices[(f + 1) % 6];


				// we want the leftmost hextile to be in tiles[0]				
				int vpos = f;
				if(f >= Tiles::HEXSOUTHWEST  && t->face.tiles[1][0] != -1){
					// so we want to swap them
					int temp_x = t->face.tiles[0][0];
					int temp_y = t->face.tiles[0][1];
					t->face.tiles[0][0] = t->face.tiles[1][0];
					t->face.tiles[0][1] = t->face.tiles[1][1];
					t->face.tiles[1][0] = temp_x;
					t->face.tiles[1][1] = temp_y;
					vpos = (f + 3) % 6; // directly opposite face on the current tile
				}

				// assign the attaching faces
				t->face.faces[0] = hex->faces[(vpos+1)%6]; // most clockwise inner face
				t->face.faces[1] = hex->faces[(vpos + 5) % 6]; // least clockwise inner face
				
				if(t->face.tiles[1][0] == -1){
					// if no neightbour is attached to the current face, 
					// let us check other adjacent tiles to see if faces
					// are attached
					t->face.faces[2] = t->face.faces[3] = -1; // just set to -1 to begin with

					// check if the most clockwise neighbour exists
					hex->get_adjacent((vpos + 1) % 6, col, row, &x, &y);
					if(x >= 0 && y >= 0 && x < m_board_width && y < m_board_width){
						other = &m_board[x + y*m_board_width];
						t->face.faces[2] = other->faces[(vpos + 5) % 6];
					}

					// check if the least clockwise neighbour exists
					hex->get_adjacent((vpos + 5) % 6, col, row, &x, &y);
					if(x >= 0 && y >= 0 && x < m_board_width && y < m_board_width){
						other = &m_board[x + y*m_board_width];
						t->face.faces[3] = other->faces[(vpos+1)%6];
					}					
				} else {
					// get the outer faces from the adjacent tile
					other = &m_board[t->face.tiles[1][0] + t->face.tiles[1][1] * m_board_width];
					t->face.faces[2] = other->faces[(vpos + 2) % 6];
					t->face.faces[3] = other->faces[(vpos + 4) % 6];
				}
			}
		

		}
	}


	
	int count;
	std::vector<vertex_face_t>::iterator it;
	logger.log(Logger::DEBUG, "vertex array %d", vertex_array.size());
	count = 0;
	for(it = vertex_array.begin(); it != vertex_array.end(); ++it){
		vertex_face_t* t = &(*it);
		logger.log(Logger::DEBUG, "vertex[%d] type=%d player=%d faces {%d,%d,%d} vertices {%d,%d,%d} tiles{ (%d,%d), (%d,%d), (%d,%d)}",
			count,t->type, t->player,
			t->vert.faces[0], t->vert.faces[1], t->vert.faces[2],
			t->vert.vertices[0], t->vert.vertices[1], t->vert.vertices[2],
			t->vert.tiles[0][0], t->vert.tiles[0][1],
			t->vert.tiles[1][0], t->vert.tiles[1][1],
			t->vert.tiles[2][0], t->vert.tiles[2][1]
			);
		count++;

	}

	logger.log(Logger::DEBUG, "face array %d", face_array.size());
	count = 0;
	for(it = face_array.begin(); it != face_array.end(); ++it){
		vertex_face_t* t = &(*it);
		logger.log(Logger::DEBUG, "face[%d] type=%d player=%d faces {%d,%d,%d,%d} vertices {%d,%d} tiles{ (%d,%d), (%d,%d)}",
			count,t->type,t->player,
			t->face.faces[0], t->face.faces[1], t->face.faces[2], t->face.faces[3],
			t->face.vertices[0], t->face.vertices[1],
			t->face.tiles[0][0], t->face.tiles[0][1],
			t->face.tiles[1][0], t->face.tiles[1][1]
			);
		count++;
	}
	return true;
}

Tiles* Model::find_tile_from_vertex(int vertex, int* c, int* r){
	if(vertex < 0 || (unsigned) vertex >= vertex_array.size()){
		Logger::getLog("jordan.log").log(Logger::ERROR, "Model::find_tile_from_vertex() %d out of range %d-%d",
			vertex, 0, vertex_array.size());
		set_error(Model::MODEL_ERROR_INVALID_VERTEX);
		return nullptr;
	}
	vertex_face_t* v = &vertex_array[vertex];
	if(v->vert.tiles[0][0] == -1){ return nullptr; }
	if(c != nullptr){ *c = v->vert.tiles[0][0];}
	if(r != nullptr){ *r = v->vert.tiles[0][1]; }
	return &m_board[v->vert.tiles[0][0] + v->vert.tiles[0][1] * m_board_width];
}
Tiles* Model::find_tile_from_face(int face, int* c, int* r){
	if(face < 0 || (unsigned) face >= face_array.size()){
		Logger::getLog("jordan.log").log(Logger::ERROR, "Model::find_tile_from_face() %d out of range %d-%d",
			face, 0, face_array.size());
		set_error(Model::MODEL_ERROR_INVALID_FACE);
		return nullptr;
	}
	vertex_face_t* f = &face_array[face];
	if(f->face.tiles[0][0] == -1){ return nullptr; }
	if(c != nullptr){ *c = f->face.tiles[0][0]; }
	if(r != nullptr){ *r = f->face.tiles[0][1]; }
	return &m_board[f->face.tiles[0][0] + f->face.tiles[0][1] * m_board_width];
}


bool Model::can_build_road(int player,int row, int col, int pos){
	if(get_player(player) == nullptr){ return false; }
	if(pos < 0 || pos >= (int)face_array.size()){
		Logger::getLog("jordan.log").log(Logger::ERROR, "Model::can_build_road() pos is out of range %d/%d", pos, (int)face_array.size());
		return false;
	}
	Tiles* tile = get_tile(col, row);
	if(tile == nullptr){ return false; }
	//vertex_face_t* face = &face_array[tile->faces[pos]];
	vertex_face_t* face = &face_array[pos];
	bool road_exists;
	bool blocked;
	bool water_tiled;

	// check the two faces beside to make sure one is a land tile
	water_tiled = true;
	for(int i = 0; i < 2; ++i){
		Tiles* t = get_tile(face->face.tiles[i][0], face->face.tiles[i][1]);
		if( t == nullptr){ continue; }
		if(!t->is_water_tile(t->type)){ water_tiled = false; break; }
	}


	// check every face for a road that belong to the player
	vertex_face_t* edge = nullptr;
	road_exists = false;
	for(int i = 0; i < 4; ++i){
		if(face->face.faces[i] == -1){ continue; }
		if(face->face.faces[i] < 0 || face->face.faces[i] >= (int)face_array.size()){
			Logger::getLog("jordan.log").log(Logger::ERROR, "Model::can_build_road() %d/%d index out of range.", face->face.faces[i], (int)face_array.size());
			continue;
		}
		edge = &face_array[face->face.faces[i]];

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
		if(face->face.vertices[i] == -1){ continue; }
		if(face->face.vertices[i] < 0 || face->face.vertices[i] >= (int)vertex_array.size()){
			Logger::getLog("jordan.log").log(Logger::ERROR, "Model::can_build_road() %d/%d vertex index out of range", face->face.vertices[i], (int)vertex_array.size());
			continue;
		}
		vert = &vertex_array[face->face.vertices[i]];

		if((vert->type == vertex_face_t::CITY || vert->type == vertex_face_t::SETTLEMENT)
			&& vert->player != player )
		{
			blocked = true;
			break;
		}
	}	

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
				vertex_face_t* v1 = get_vertex(face->face.vertices[vindex]);
				if(v1 == nullptr){continue;}

				if(v1->type == vertex_face_t::SETTLEMENT && v1->player == player){
					bool is_okay = true;
					// check that this settlment doesn't already have an attached road
					for(int i = 0; i < 3; ++i){
						if(get_face(v1->vert.faces[i]) == nullptr){ continue; }
						if(get_face(v1->vert.faces[i])->type != vertex_face_t::NONE){
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
bool Model::can_build_settlement(int player, int row, int col, int v){
	if(get_player(player) == nullptr){ return false; }
	if(v < 0 || v >= (int)vertex_array.size()){
		Logger::getLog("jordan.log").log(Logger::ERROR, "Model::can_build_settlement() index out of range v=%d/%d",v, (int)vertex_array.size());
		return false;
	}
	Tiles* tile = get_tile(col, row);
	if(tile == nullptr){ return false; }
	vertex_face_t* vertex = &vertex_array[v];
	bool has_space = false;
	bool is_connected = false;
	bool water_tiled = true;

	// check the three tiles beside to make sure one is a land tile
	water_tiled = true;
	for(int i = 0; i < 3; ++i){
		Tiles* t = get_tile(vertex->vert.tiles[i][0], vertex->vert.tiles[i][1]);
		if(t == nullptr){ continue; }
		if(!t->is_water_tile(t->type)){ water_tiled = false; break; }
	}


	// check every vertex for for an existing settlement/city 
	vertex_face_t* point = nullptr;
	has_space = true;
	for(int i = 0; i < 3; ++i){
		if(vertex->vert.vertices[i] == -1){ continue; }
		if(vertex->vert.vertices[i] < 0 || vertex->vert.vertices[i] >= (int)vertex_array.size()){
			Logger::getLog("jordan.log").log(Logger::ERROR, "Model::can_build_settlement() %d/%d vertex index out of range", vertex->vert.vertices[i], (int)vertex_array.size());
			continue;
		}
		point = &vertex_array[vertex->vert.vertices[i]];
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
		if(vertex->vert.faces[i] == -1){ continue; }
		if(vertex->vert.faces[i] < 0 || vertex->vert.faces[i] >= (int)face_array.size()){
			Logger::getLog("jordan.log").log(Logger::ERROR, "Model::can_build_settlement() %d/%d face index out of range", vertex->vert.faces[i], (int)face_array.size());
			continue;
		}

		edge = &face_array[vertex->face.faces[i]];
		if(edge->type == vertex_face_t::ROAD && edge->player == player){
			is_connected = true;
			break;
		}
	}

	// Handle the speical case in which the player is placing their 
	// first few settlments and roads.
	if(get_player(player) != nullptr){
		if(get_player(player)->buildings.size() < 2){
			is_connected = true;
		}
	}

	return (has_space && is_connected && water_tiled ==false);
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

bool Model::bank_exchange(int player, resource_t* give, resource_t* take){
	if(!give || !take){ return false; }
	if(player < 0 || player >= m_num_players){ return false; }
	Logger::getLog("jordan.log").log(Logger::DEBUG, "Model::bank_exchange player=%d give=%d,%d,%d,%d,%d take=%d,%d,%d,%d,%d",
		player,
		give->res[0], give->res[1], give->res[2], give->res[3], give->res[4],
		take->res[0], take->res[1], take->res[2], take->res[3], take->res[4]
		);

	resource_t player_copy = m_players[player].resources;
	resource_t bank_copy = bank;
	bool good_flag = true;;
	Logger::getLog("jordan.log").log(Logger::DEBUG, "Model::bank_exchange old_bank=%d,%d,%d,%d,%d",
		bank.res[0], bank.res[1], bank.res[2], bank.res[3], bank.res[4]);

	for(int i = 0; i < resource_t::NUM_OF_RESOURCES; ++i){
		if(
			(bank_copy.res[i] - take->res[i] < 0) ||
			(bank_copy.res[i] + give->res[i] > config.resource_cards_cap[i])
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
	m_players[player].resources = player_copy;
	bank = bank_copy;
	Logger::getLog("jordan.log").log(Logger::DEBUG, "Model::bank_exchange new_bank=%d,%d,%d,%d,%d",
		bank.res[0], bank.res[1], bank.res[2], bank.res[3], bank.res[4]);
	return true;
}

bool Model::move_thief(int col, int row){
	// check that the thief isn't placed on a water tile
	if(col < 0 || row < 0 || col >= m_board_width || row >= m_board_width ||
		Tiles::is_water_tile(m_board[col + row*m_board_width].type))
	{
		set_error(Model::MODEL_ERROR_PLACE_THIEF);
		return false;
	}
	
	thief_pos_x = col;
	thief_pos_y = row;
	Logger::getLog("jordan.log").log(Logger::DEBUG, "Model::move_thief(col=%d,row=%d)",thief_pos_x,thief_pos_y);
	return true;
}

void Model::transfer_largest_army_card(int original_player, int new_player){
	if(original_player < 0 || original_player >= m_num_players){ return; }
	if(new_player < 0 || new_player >= m_num_players){ return; }
	player_holding_largest_army_card = new_player;
}
void Model::transfer_longest_road_card(int original_player, int new_player){
	if(original_player < 0 || original_player >= m_num_players){ return; }
	if(new_player < 0 || new_player >= m_num_players){ return; }
	player_holding_longest_road_card = new_player;
}

Tiles* Model::get_tile(int col, int row){
	if(col < 0 || row < 0 || col >= m_board_width || row >= m_board_height){ 
		set_error(Model::MODEL_ERROR_INVALID_TILE);
		return nullptr;
	}
	return &m_board[col + row*m_board_width];
}
int Model::get_type_from_tile(int col, int row){
	if(col < 0 || row < 0 || col >= m_board_width || row >= m_board_height){ 
		set_error(Model::MODEL_ERROR_INVALID_TILE);
		return Tiles::NONE_TILE;
	}
	return m_board[col + row*m_board_width].type;
}
int Model::get_roll_from_tile(int col, int row){
	if(col < 0 || row < 0 || col >= m_board_width || row >= m_board_height){
		set_error(Model::MODEL_ERROR_INVALID_TILE);
		return -1;
	}
	return m_board[col + row*m_board_width].roll;
}
int* Model::get_vertices_from_tile(int col, int row, int* size){
	if(col < 0 || row < 0 || col >= m_board_width || row >= m_board_height){
		set_error(Model::MODEL_ERROR_INVALID_TILE);
		return nullptr;
	}
	if(size != nullptr){ *size = 6; }
	return m_board[col + row*m_board_width].vertices;
}
int* Model::get_faces_from_tile(int col, int row, int* size){
	if(col < 0 || row < 0 || col >= m_board_width || row >= m_board_height){
		set_error(Model::MODEL_ERROR_INVALID_TILE); 
		return nullptr;
	}
	if(size != nullptr){ *size = 6; }
	return m_board[col + row*m_board_width].faces;
}
int Model::get_type_from_face(int face){
	if(face < 0 || face >= (int)face_array.size()){
		set_error(Model::MODEL_ERROR_INVALID_FACE);
		return vertex_face_t::NONE;
	}
	return face_array[face].type;
}
int Model::get_type_from_vertex(int vertex){
	if(vertex < 0 || vertex >= (int)vertex_array.size()) { 
		set_error(Model::MODEL_ERROR_INVALID_VERTEX);
		return vertex_face_t::NONE;
	}
	return vertex_array[vertex].type;
}
int Model::get_player_from_face(int face){
	if(face < 0 || face >= (int)face_array.size()){
		set_error(Model::MODEL_ERROR_INVALID_FACE); 
		return -1;
	}
	return face_array[face].player;
}
int Model::get_player_from_vertex(int vertex){
	if(vertex < 0 || vertex >= (int)vertex_array.size()) { 
		set_error(Model::MODEL_ERROR_INVALID_VERTEX);
		return -1;
	}
	return vertex_array[vertex].player;
}

Player* Model::get_player(int player){
	if(player < 0 || player >= m_num_players){ return nullptr; }
	return &m_players[player];
}

int Model::largest_army(){
	if(player_holding_largest_army_card == -1){ return 0; }
	else{
		return m_players[player_holding_largest_army_card].num_soldiers;
	}
}
int Model::longest_road(){	
	if(player_holding_longest_road_card == -1){ return 0; }
	else{
		// TODO: This is not working right now. INCORRECT
		return m_players[player_holding_longest_road_card].roads.size();
	}
}

int Model::num_victory_points_for_player(int player){
	if(player < 0 || player >= m_num_players){ return -1; }
	Player& p = m_players[player];

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
		if(this->vertex_array[p.buildings[i]].type == vertex_face_t::CITY){
			points += 2;
		} else{
			points++;
		}
	}

	if(player == this->player_holding_largest_army_card){ points+=2;}
	if(player == this->player_holding_longest_road_card){ points+=2; }
	return points;
}



vertex_face_t* Model::get_vertex(int num){
	if(num < 0 || num >= (int)vertex_array.size()){ 
		set_error(Model::MODEL_ERROR_INVALID_VERTEX);
		return nullptr;
	}
	return &vertex_array[num];
}
vertex_face_t* Model::get_face(int num){
	if(num < 0 || num >= (int)face_array.size()){
		set_error(Model::MODEL_ERROR_INVALID_FACE);
		return nullptr;
	}
	return &face_array[num];
}

void Model::set_error(Model::model_error_codes_e code){
	model_error = code;
}
int Model::get_error(){
	return model_error;
}