#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_mixer.h>


#include "Timer.h"
#include "Tiles.h"
#include "Collision.h"
#include "Button.h"
#include "Model.h"
#include "model_structs.h"
#include "IView.h"

/*
Concrete class for the Game View
*/
class fps_t{
public:
	int frame_count, current_time, fps; bool done;
	fps_t(){ frame_count = 0; current_time = 0; fps = 0; done = true; }
	void update(){
		int tick = SDL_GetTicks() / 1000;
		if((tick - current_time) >= 1){
			fps = frame_count; frame_count = 0; current_time = tick;
		}; ++frame_count;
	}
};
class Mouse{
public:
	int x, y;
	Uint8 buttons;
	Collision hitbox;
	Mouse(){hitbox.hook(&x, &y);hitbox.add_rect(0, 0, 1, 1);}
	void update(){buttons = SDL_GetMouseState(&x, &y);}
};
class pane_t{
public:
	Collision hitbox;
	int x, y, w, h;
	int padding;
	pane_t(){}
	void init(int x, int y, int w, int h, int padding = 0){
		this->x = x;this->y = y;
		this->w = w ;this->h  = h;
		this->padding = padding;
		hitbox.rects.clear();
		hitbox.hook(&this->x,&this->y);
		hitbox.add_rect(0,0, this->w, this->h);
		Logger::getLog("jordan.log").log(Logger::DEBUG, "pane_t(x=%d,y=%d,w=%d,h=%d,internal_padding=%d)",
			this->x, this->y, this->w, this->h,this->padding);
	}
};

class Tile_intersect{
public:
	int x, y;
	int col, row;
	int w, h;
	Collision hitbox;

	void init(int x, int y, int w, int h, int col, int row){
		this->x = x; this->y = y;
		this->w = w; this->h = h;
		this->col = col;this->row = row;			
		hitbox.hook(&this->x,&this->y);
		hitbox.add_rect(0,0,this->w, this->h);
	}
	virtual ~Tile_intersect(){}
};
class vertex_face_t_intersect{
public:
	int x, y, w, h, num;
	Collision hitbox;
	void init(int x, int y, int w, int h, int num){
		this->x = x; this->y = y;
		this->w = w; this->h = h;
		this->num = num;
		this->hitbox.hook(&this->x, &this->y);
		this->hitbox.add_rect(0, 0, this->w, this->h);
	}
	virtual ~vertex_face_t_intersect(){};
};


class View_Game : public IView{
	enum hexpos_e {HEXORIG, HEXNE,HEXSE,HEXSW,HEXNW};
	
public:
	enum state_e {
		NONE, PLACE_ROAD, PLACE_SETTLEMENT, PLACE_CITY, PLACE_THEIF,
		PLAYING_DEV_CARD, TRADING, NUM_OF_state_e
	};
	// variables
	Model& model;
	pane_t* selected_pane;
	Mouse mouse;

	int disp_w, disp_h;
	int tile_w, tile_h;
	int desired_fps;
	bool exit_flag;
	bool active;
	unsigned state;
	
	// selected tile
	Tile_intersect* selected_tile;
	std::vector<Tile_intersect> tiles;
	int tile_col, tile_row;
	int tilehex_pos;
	bool draw_tile;
	// selected vertex
	vertex_face_t_intersect* selected_vertex;
	std::vector<vertex_face_t_intersect> vertices;
	int vertex_hit_w;
	int vertex_hit_h;
	bool draw_vertex;
	// selected face
	vertex_face_t_intersect* selected_face;
	std::vector<vertex_face_t_intersect> faces;
	int face_hit_w;
	int face_hit_h;
	int draw_face;

	/*
	int selected_face;
	int tilehex_face_pos;
	bool draw_face;
	*/
	
	//constructor and destructor
	View_Game(Model& model, SDL_Window& win, SDL_Renderer& ren);
	virtual ~View_Game();

	//methods
	void set_state(state_e state);
	void on_start(SDL_Event& e);
	void on_switch(SDL_Event& e);
	void on_close(SDL_Event& e);
	void handle_keyboard_events(SDL_Event& ev);
	void handle_mouse_events(SDL_Event& ev);
	void handle_user_events(SDL_Event& ev);
	void update(SDL_Event& ev);
	void render();	
private:
	// resources
	SDL_Texture* hextile_spritesheet;
	SDL_Texture* buildings_spritesheet;
	SDL_Surface* hextile_surface;
	SDL_Surface* hextile_face_surface;
	Mix_Music* music;
	Mix_Chunk* sound1;
	Timer* fps_timer;
	TTF_Font* font_carbon_12;
	SDL_Color font_carbon_12_colour;
	SDL_Rect hextile_clips[Tiles::NUM_OF_TILES];
	SDL_Rect building_clips[building_t::NUM_OF_BUILDINGS];
	SDL_Rect road_clips[6];
	fps_t fps;
	Uint8 *vertex_covered;
	Uint8 *face_covered;
	pane_t top_pane, mid_pane, bot_pane;
	int total_frame_count;
	int face_size[2];
	int sprite_small[2]; // w and h of small sprites
	int sprite_medium[2]; // w and h of medium sprites
	int sprite_large[2]; // w and h of large sprites
	int vertex_pos[6][2];
	int face_pos[6][2];
	
	// buttons
	Button exit_button;
	Button end_turn_button;
	Button roll_button;
	Button add_road_button;
	Button add_settlement_button;
	Button add_city_button;
	Button buy_dev_card_button;
	Button play_dev_card_button;
	Button trade_button;
	std::vector<Button*> button_list;

	// methods
	bool load();
	bool setup_top_bar(pane_t& pane);
	bool setup_board_pane(pane_t& pane);
	bool setup_buttons(pane_t& pane);
	void render_model(pane_t& pane);
	void render_buttons(pane_t& pane);
	void render_bottom_text(pane_t& pane);
	void getTilePosFromPixelPos(int px, int py, int* tile_x, int* tile_y);
	void getPixelPosFromTilePos(int tile_col, int tile_row, int* px, int* py);
	int getTilehexDir(int px, int py);
	int getHexFaceDir(int px, int py); // returns -1 for no selection.
	bool getTilePosFromDirection(int dir, int tilex, int tiley, int* new_tile_x, int* new_tile_y);	
	void get_mid_point_from_face(int face, int* x, int* y);
	//bool rect_intersect(SDL_Rect* A, SDL_Rect* B, SDL_Rect* result);
	//bool rect_intersect(int ax, int ay, int aw, int ah,int bx, int by, int bw, int bh,int* rs_x, int* rs_y, int* rs_w, int* rs_h);
};
