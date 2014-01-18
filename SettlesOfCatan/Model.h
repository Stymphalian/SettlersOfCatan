#pragma once
#include <vector>
#include <list>
#include "Tiles.h"
#include "model_structs.h"

class Player;

class Model{
public:
	// enum phase_e {NONE,ROLL,THIEF,TRADE,BUILD,NUM_PHASES};
	// Variables
	int m_num_levels; // how many rings are there
	int m_num_extensions; // how many extensions were used
	int num_dice;
	int num_dice_sides;

	resource_t bank;
	int roll_value;
	int m_num_players;
	int m_current_player;
	Player* m_players;
		
	Tiles* m_board;
	int m_board_size; // width*height
	int m_board_width; //also the len of the longest row ( middle row)
	int m_board_height;  // the number of rows on the game board
	bool m_even_middle_row;
	int m_num_game_tiles; // how many game tiles are there, including the water tiles
	int m_num_water_tiles; // the number of water tiles, include ports
	std::vector<vertex_face_t> vertex_array;
	std::vector<vertex_face_t> face_array;

	std::vector<dev_cards_t> m_dev_deck; // a list holding the deck of dev cards

	int thief_pos_x;
	int thief_pos_y;
	int player_holding_largest_army_card;
	int player_holding_longest_road_card;
	int largest_army();
	int longest_road();

	int turn_count;

	// Constructors and Destructors
	Model(int num_players);
	virtual ~Model();

	// Methods
	void end_turn();

	Tiles* get_tile(int col, int row);
	int get_type_from_face(int face);
	int get_type_from_vertex(int vertex);
	int get_player_from_face(int face);
	int get_player_from_vertex(int vertex);
	int get_type_from_tile(int col, int row);
	int get_roll_from_tile(int col, int row);
	int* get_vertices_from_tile(int col, int row,int* size=nullptr);
	int* get_faces_from_tile(int col, int row,int* size=nullptr);
	Player* get_player(int player);

	resource_t get_building_cost(building_t::buildings card);
	void build_building(int player, building_t::buildings building, int pos);
	bool can_build_road(int player, int row, int col, int face);
	bool can_build_settlement(int player, int row, int col, int vertex);
	int get_ring_level(int row, int col, int nlevels);
	
	int roll(int num_dice, int num_sides);
	void give_resources_from_roll(int roll);
	bool move_thief(int new_col, int new_row);

	bool buy_dev_card(int player);
	void play_dev_card(int player, dev_cards_t* card);
	void transfer_largest_army_card(int original_player, int new_player);
	void transfer_longest_road_card(int original_player, int new_player);
		
	bool pay_for_item(int player, resource_t* price);
	bool bank_exchange(int player, resource_t* give, resource_t* take);
	bool bank_exchange(resource_t* give, resource_t* take);
private:
	// private member variables
	configuration_t config; // hold configuration data, such as how many tiles and cards we should have
	
	// methods
	void establish_board_dimensions(int num_players);
	void init_players(Player* players, int num_players);
	bool fill_board(Tiles* board, int size,int num_levels);
	bool fill_deck(int* default_dev_cards_cap,int size);

	dev_cards_t draw_dev_card();
	bool add_city(int player, int pos);
	bool add_settlement(int player, int pos);
	bool add_road(int player, int pos);
	Tiles* find_tile_from_vertex(int vertex,int* c, int* r);
	Tiles* find_tile_from_face(int face,int* c, int* r);
	
	int level_from_row(int row,int levels,int num_extensions);
	int numtiles_from_row(int row,int levels,int num_extensions);
	int num_offset_tiles_from_row(int row, int levels,int num_extensions);
	void apply_tile_resource_extensions(int num_extensions);
	int num_possiblities_dice(int sum, int num_dice, int num_sides);
	bool fill_vertex_face_arrays();
};


/*
int edge(int* graph, int n, int u, int v){
	return graph[u + v*n];
}

void dfs(int* graph, int n,int origin){
	if(!graph){ return; }

	// variables and initaliazation
	int v = 0; // current node
	std::vector<int> stack;
	int* visited = (int*) malloc(sizeof(int)* n);
	memset(visited, 0, sizeof(int)*n);

	// push origin onto the stack
	stack.insert(stack.begin(),origin);

	while(stack.empty() == false){
		// pop off the first node from the stack
		v = stack.front();
		stack.erase(stack.begin());
		
		// mark v as visited
		visited[v] = 1;

		// Process the node

		// for every edge e inbound to v
		for(int w = 0; w < n; ++w){
			if(w == v) { continue; }
			if(edge(graph, n, v, w) == 0){ continue; }

			// edge exists
			if(visited[w] == 1){
				// node is already visited ( backedge)
			} else{
				// node is not yet visited ( discovery edge )
				// push onto the stack
				stack.insert(stack.begin(), w);
			}
		}
	}
}

void bfs(int* graph, int n,int origin){
	if(!graph){ return; }

	//variables
	std::vector<int> queue;
	int v = 0;
	int* visited = (int*)malloc(sizeof(int)*n);
	memset(visited, 0, sizeof(int)*n);

	// push origin onto the queue
	queue.push_back(origin);

	while(queue.empty() == false){
		// pop off the queue
		v = queue.front();
		queue.erase(queue.begin());
		visited[v] = 1;

		// process the vertex

		// for every edge e inbound on current
		for(int w = 0; w < n; ++w){
			if(w == v){ continue; }
			if(edge(graph, n, w, v) == 0){ continue; }

			if(visited[w] == 1){
				// already visited
			} else{
				// back edge or cross edge
				// enqueue onto the queue
				queue.push_back(w);
			}
		}// end for

	}
}

*/