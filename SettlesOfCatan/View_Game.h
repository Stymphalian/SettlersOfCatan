#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_mixer.h>

class Timer;
class Tiles;
#include "Collision.h"
#include "Button.h"
#include "Model.h"
#include "model_structs.h"
#include "IView.h"
class IDialog;
class View_Game_Debug_Dialog;

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
	Mouse(){hitbox.hook(&x, &y);hitbox.add_rect(0, 0,0, 1, 1);}
	void update(){buttons = SDL_GetMouseState(&x, &y);}
};
class pane_t{
public:
	Collision hitbox;
	int x, y,z, w, h;
	int padding;
	
	pane_t(){
		x = y = z = w = h = padding =0;
		setVisible(false);
	}
	void init(int x, int y,int z, int w, int h, int padding = 0){
		this->x = x; this->y = y; this->z = z;
		this->w = w ;this->h  = h;
		this->padding = padding;
		hitbox.rects.clear();
		hitbox.hook(&this->x,&this->y,&this->z);
		hitbox.add_rect(0,0,0, this->w, this->h);
		setVisible(true);
		Logger::getLog().log(Logger::DEBUG, "pane_t(x=%d,y=%d,z=%d,w=%d,h=%d,internal_padding=%d)",
			this->x, this->y,this->z, this->w, this->h,this->padding);
	}
	bool isVisible(){ return visible; }
	void setVisible(bool value){
		visible = value;
		hitbox.active = value;
	}
private:
	bool visible;
};
class message_pane_t : public pane_t{
public:
	std::string message;
	unsigned timeout;
	unsigned current_tick;
	bool just_finished;
	
	message_pane_t() : pane_t(){
		this->message = "";
		this->timeout = 0;
		this->current_tick = 0;
		this->just_finished = false;
	}
	void tick(){
		if(current_tick == 0){
			this->just_finished = false;
		}else if(current_tick == 1){
			pane_t::setVisible(false);
			this->current_tick = 0;
			this->just_finished = true;
		} else{
			this->current_tick--;
		}
	}	
	void setTimeout(unsigned timeout){
		this->timeout = timeout;
		if(this->current_tick > timeout){this->current_tick = timeout;}
	}
	void reset(){ this->current_tick = this->timeout; pane_t::setVisible(true); }
	void stop(){ this->current_tick = 0; pane_t::setVisible(false); }
	void setMessage(const char* message){this->message = message;}
};

class Tile_intersect{
public:
	int x, y,z;
	int col, row;
	int w, h;
	Collision hitbox;

	void init(int x, int y,int z, int w, int h, int col, int row){
		this->x = x; this->y = y; this->z = z;
		this->w = w; this->h = h;
		this->col = col;this->row = row;			
		hitbox.hook(&this->x,&this->y,&this->z);
		hitbox.add_rect(0,0,0,this->w, this->h);
	}
	virtual ~Tile_intersect(){}
};

class vertex_face_t_intersect{
public:
	int x, y,z, w, h, num;
	Collision hitbox;
	void init(int x, int y,int z, int w, int h, int num){
		this->x = x; this->y = y; this->z = z;
		this->w = w; this->h = h;
		this->num = num;
		this->hitbox.hook(&this->x, &this->y,&this->z);
		this->hitbox.add_rect(0, 0,0, this->w, this->h);
	}
	virtual ~vertex_face_t_intersect(){};
};

class view_debug_t{
public:
	int panel_page;	
	bool board_tiles;
	bool board_vertices;
	bool board_faces;		
	bool selected_pane;
	bool item_numbering;
	bool selected_tile_vertices;;
	bool selected_tile_faces;;
	bool selected_vertex_vertices;
	bool selected_vertex_faces;
	bool selected_vertex_tiles;
	bool selected_face_vertices;
	bool selected_face_faces;
	bool selected_face_tiles;
	view_debug_t();
};

class View_Game : public IView{
public:
	static std::string view_game_model_error_strings[Model::NUM_model_error_codes_e];
	enum state_e {
		NONE,
		START,
		PLACE_SETTLEMENT_1,
		PLACE_ROAD_1,
		PLACE_SETTLEMENT_2,
		PLACE_ROAD_2,
		START_RESOURCES,
		NORMAL,
		TRADING,
		BUILD_SETTLEMENT,
		BUILD_CITY,
		BUILD_ROAD,
		PLAY_DEV_CARD,
		NUM_state_e
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
	state_e state;
	bool change_state_flag;

	// debug flags
	bool debug;
	view_debug_t _debug;
	
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
	// A debug dialog
	IDialog* dialog_in_focus;
	View_Game_Debug_Dialog* debug_dialog;
	
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
	void open_debug_dialog();
	void close_debug_dialog();
	void open_trade_dialog();
	void close_trade_dialog();
	void set_message_pane_text(const char* message);
private:
	// Variables  and resources
	SDL_Texture* hextile_spritesheet;	
	SDL_Surface* hextile_surface;
	SDL_Surface* hextile_face_surface;
	std::vector<SDL_Texture*> player_buildings_spritesheet;
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
	pane_t misc_pane;
	message_pane_t message_pane;
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
	Button enable_debug_button;
	Button misc_button;
	Button add_road_button;
	Button add_settlement_button;
	Button add_city_button;
	Button buy_dev_card_button;
	Button play_dev_card_button;
	Button trade_button;
	std::vector<Button*> button_list;

	// methods
	bool load();
	bool load_player_building_textures(const char* file);
	bool setup_top_bar(pane_t& pane);
	bool setup_board_pane(pane_t& pane);
	bool setup_buttons(pane_t& pane);
	void render_model(pane_t& pane);
	void render_model_board_tiles(pane_t& pane);
	void render_model_face_vertex_tiles(pane_t& pane);
	void render_model_selected(pane_t& pane);
	void render_model_debug(pane_t& pane);	
	void render_connecting_vertices(pane_t& pane, vertex_face_t* origin, int num);
	void render_connecting_faces(pane_t& pane,vertex_face_t* origin, int num);
	void render_connecting_tiles(pane_t& pane, vertex_face_t* origin, int num);
	void render_buttons(pane_t& pane);
	void render_bottom_text(pane_t& pane);
	void render_top_pane(pane_t& pane);
	void update_check_for_collisions();
	void update_top_pane(pane_t& pane, Collision& rel_mouse_hitbox);
	void update_mid_pane(pane_t& pane, Collision& rel_mouse_hitbox);
	void update_bot_pane(pane_t& pane, Collision& rel_mouse_hitbox);
	void getTilePosFromPixelPos(int px, int py, int* tile_x, int* tile_y);
	void getPixelPosFromTilePos(int tile_col, int tile_row, int* px, int* py);
	int getTilehexDir(int px, int py);
	int getHexFaceDir(int px, int py); // returns -1 for no selection.
	bool getTilePosFromDirection(int dir, int tilex, int tiley, int* new_tile_x, int* new_tile_y);	
	void get_mid_point_from_face(int face, int* x, int* y);

	// handling mouse input for the different states
	void handle_mouse_given_state(SDL_Event& ev,View_Game::state_e s);
	void View_Game::handle_keyboard_given_state(SDL_Event& ev, View_Game::state_e s);
};


