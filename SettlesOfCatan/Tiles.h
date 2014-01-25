#pragma once
class Tiles{
public:
	enum directions{
		HEXNORTHEAST = 0, HEXEAST, HEXSOUTHEAST,
		HEXSOUTHWEST, HEXWEST, HEXNORTHWEST
	};
	enum tiles
	{
		NONE_TILE, SHEEP_TILE, BRICK_TILE, WOOD_TILE,
		WHEAT_TILE, ORE_TILE, DESERT_TILE,
		WATER_TILE, SHEEP_PORT, BRICK_PORT, WOOD_PORT,
		WHEAT_PORT, ORE_PORT, TRADE_PORTS, NUM_OF_TILES
	};
	// statics
	static bool is_water_tile(int type);
	static bool is_land_tile(int type);
	static bool get_adjacent(int dir, int x, int y, int* dx, int* dy);
	static int hex_dx(int dir, bool odd_row);
	static int hex_dy(int dir);

	// variables
	int active;
	int type;
	int roll;
	int vertices[6]; // clockwise clock
	int faces[6]; // use hex face direction
	int ring_level;

	// constructor & desctructor
	Tiles();
	virtual ~Tiles();

	// methods
	void init(int type, int roll);
	void reset_vertices_faces();
	int get_direction_of_vertex(int vertex_num);
	int get_direction_of_face(int face_num);	
};

