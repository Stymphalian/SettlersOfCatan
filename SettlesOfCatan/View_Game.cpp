#include <vector>
#include <list>

#include <SDL.h>
#include "Logger.h"
#include "Util.h"
#include "Model.h"
#include "IView.h"
#include "View_Game.h"
#include "Player.h"
#include "lizz_lua.h"

std::string View_Game::view_game_model_error_strings[Model::NUM_model_error_codes_e] = {
	"",
	"Error",
	"Not enough bank resources.Not enough brick.",
	"Not enough bank resources.Not enough ore.",
	"Not enough bank resources.Not enough sheep",
	"Not enough bank resources.Not enough wheat.",
	"Not enough bank resources.Not enough wood",
	"Not enough bank resources.No more dev cards",
	"Not enough resources.Not enough brick.",
	"Not enough resources.Not enough ore.",
	"Not enough resources.Not enough sheep.",
	"Not enough resources.Not enough wheat.",
	"Not enough resources.Not enough wood.",
	"Not enough resources.Not enough buildings.",
	"Can't place thief.",
	"Can't place settlement.",
	"Can't place city.",
	"Can't place road.",
	"Can't play dev card.",
	"Inavlid tile.",
	"Invalid face.",
	"Invalid vertex."
};

View_Game::View_Game(Model& _model, SDL_Window& win, SDL_Renderer& ren)
: model(_model),IView(win,ren)
{
	Logger::getLog("jordan.log").log(Logger::DEBUG,"View_Game::View_Game() constructor");
	this->disp_w = UTIL_DISP_W;
	this->disp_h = UTIL_DISP_H;
	this->tile_w = UTIL_HEXTILE_W;
	this->tile_h = UTIL_HEXTILE_H;
	sprite_small[0] = UTIL_SPRITE_SMALL_W;
	sprite_small[1] = UTIL_SPRITE_SMALL_H;
	sprite_medium[0] = UTIL_SPRITE_MEDIUM_W;
	sprite_medium[1] = UTIL_SPRITE_MEDIUM_H;
	sprite_large[0] = UTIL_SPRITE_LARGE_W;
	sprite_large[1] = UTIL_SPRITE_LARGE_H;
	face_size[0] = UTIL_SPRITE_FACE_W;// 40;//20; // width
	face_size[1] = UTIL_SPRITE_FACE_H;// 20;//10; // height
	this->exit_flag = false;
	this->desired_fps = UTIL_FPS;
	this->total_frame_count = 0;
	this->state = state_e::NONE;
	this->change_state_flag = false;
	this->selected_pane = nullptr;
	this->debug = true;
	this->debug_tiles = false;
	this->debug_vertices = false;
	this->debug_faces = false;
	this->debug_data = 0;

	mouse.buttons = 0;
	mouse.x = 0;
	mouse.y = 0;

	// selected tile initialization
	selected_tile = nullptr;
	this->tile_col = 0;
	this->tile_row = 0;
	this->tilehex_pos = 0;
	this->draw_tile = false;
	// selectd vertex intialization
	this->selected_vertex = nullptr;
	this->vertex_hit_w = UTIL_VERTEX_HITBOX_W;
	this->vertex_hit_h = UTIL_VERTEX_HITBOX_H;
	this->draw_vertex = false;
	// selected face initialization
	this->selected_face = nullptr;
	this->face_hit_w = UTIL_VERTEX_HITBOX_W;
	this->face_hit_h = UTIL_VERTEX_HITBOX_H;
	this->draw_face = false;

	// setup the static panes to be used for this view
	int offset_x = 0;
	int offset_y = 0;
	int offset_z = 0;
	int padding = 0;
	getPixelPosFromTilePos(0,4, &offset_x, &offset_y);		
	this->top_pane.init(0, 0,offset_z,disp_w, offset_y);

	int board_x, board_y;
	getPixelPosFromTilePos(model.get_board_width() + 1, model.get_board_height() + 1, &board_x, &board_y);
	offset_x = 0;
	offset_y = top_pane.h ; 
	offset_z = 0;
	padding = vertex_hit_w*2; // 20 px internal padding( i.e. 10 on each side (top, bottom, left, right))
	this->mid_pane.init(offset_x,offset_y,offset_z,board_x-tile_w/2+ padding,board_y-tile_h/2 + padding,padding);

		//static sized bottom bar
	offset_x = 0;
	offset_y = disp_h - 240;
	offset_z = 1;
	this->bot_pane.init(offset_x, offset_y,offset_z, disp_w, 240 );

	// the misc pane to be used by whatever ...?
	offset_x = disp_w/8;
	offset_y = disp_h/8;
	offset_z = 2;
	this->misc_pane.init(offset_x, offset_y, offset_z,disp_w - (disp_w / 4), disp_h - (disp_h / 4));
	this->misc_pane.setVisible(false);

	// the message pane
	offset_x = disp_w/8;
	offset_y = this->mid_pane.y + this->mid_pane.h;
	offset_z = 3;
	this->message_pane.init(offset_x, offset_y, offset_z, disp_w - 2*(disp_w/8),18);
	this->message_pane.setMessage(nullptr);
	this->message_pane.setTimeout((unsigned)(this->desired_fps*1.5));
	this->message_pane.stop();

	// loading textures, music, fonts, and clips
	// all the new and mallocs are here.
	this->active = load();
	// setup the top pane
	this->active = this->active && setup_top_bar(top_pane);
	// setup the middle pane, which will hold the game board
	this->active = this->active &&  setup_board_pane(mid_pane);
	// setup the buttons
	this->active = this->active && setup_buttons(bot_pane);
	if(this->active == false){
		Logger::getLog("jordan.log").log(Logger::ERROR, "View_Game() Error loading view");
	}	
}

View_Game::~View_Game(){
	Logger::getLog("jordan.log").log(Logger::DEBUG,"View_Game::~View_Game() destructor");
	delete fps_timer;
	delete[] vertex_covered;
	delete[] face_covered;
	tiles.clear();
	vertices.clear();
	faces.clear();

	for(int i = 0; i < (int)player_buildings_spritesheet.size(); ++i){
		SDL_DestroyTexture(player_buildings_spritesheet[i]);
	}
	SDL_DestroyTexture(hextile_spritesheet);
	SDL_FreeSurface(hextile_surface);
	SDL_FreeSurface(hextile_face_surface);
	TTF_CloseFont(font_carbon_12);
	Mix_FreeMusic(music);
	Mix_FreeChunk(sound1);	
}

bool View_Game::load(){
	Logger& logger = Logger::getLog("jordan.log");
	logger.log(logger.DEBUG, "View_Game::load() Loading all View_Game resources");

	hextile_spritesheet = Util::load_texture("data/hextiles_spritesheet.png",&ren);
	if(hextile_spritesheet == nullptr){
		logger.SDL_log(Logger::ERROR, "View_Game::load() load_texture data/hextiles_spritesheet.png");
		return false;
	}

	if(load_player_building_textures("data/buildings_spritesheet.bmp") == false) {
		logger.SDL_log(Logger::ERROR, "View_Game::load() load_player_building_textures(buildings_spritesheet.bmp)");
		return false;
	}	

	hextile_surface = Util::load_surface("data/hextile.png");
	if(hextile_surface == nullptr){
		return false;
	}

	hextile_face_surface = Util::load_surface("data/hextile_faces.png");
	if(hextile_face_surface == nullptr){
		return false;
	}

	font_carbon_12 = TTF_OpenFont("data/carbon.ttf", 12);
	font_carbon_12_colour = { 177, 177, 98, 255 };
	if(font_carbon_12 == nullptr){
		logger.TTF_log(Logger::ERROR, "View_Game::load() TTF_OpenFont data/carbon.ttf ");
		return false;
	}

	music = Mix_LoadMUS("data/music.mp3");
	if(music == nullptr){
		logger.Mix_log(Logger::ERROR, "View_Game::load() Mix_LoadMUX data/music.mp3");
		return false;
	}

	sound1 = Mix_LoadWAV("data/sound1.wav");
	if(sound1 == nullptr){
		logger.Mix_log(Logger::ERROR, "View_Game::load() Mix_LoadWAV data/sound1.wav");
		return false;
	}

	// Create the timer, but have it stopped to begin with
	fps_timer = TimerFactory::get().make(1000/desired_fps);
	fps_timer->stop();

	// vertex covered
	vertex_covered = new Uint8[model.get_num_vertices()];
	// face covered
	face_covered = new Uint8[model.get_num_faces()];

	// setting up the clips for the sprite sheets
	memset(hextile_clips, 0, sizeof(hextile_clips));
	hextile_clips[Tiles::SHEEP_TILE] = { 0, 0, tile_w, tile_h };
	hextile_clips[Tiles::BRICK_TILE] = { tile_w, 0, tile_w, tile_h };
	hextile_clips[Tiles::WOOD_TILE] = { tile_w * 2, 0, tile_w, tile_h };
	hextile_clips[Tiles::WHEAT_TILE] = { tile_w * 3, 0, tile_w, tile_h };
	hextile_clips[Tiles::ORE_TILE] = { tile_w * 4, 0, tile_w, tile_h };
	hextile_clips[Tiles::SHEEP_PORT] = { 0, tile_h, tile_w, tile_h };
	hextile_clips[Tiles::BRICK_PORT] = { tile_w, tile_h, tile_w, tile_h };
	hextile_clips[Tiles::WOOD_PORT] = { tile_w * 2, tile_h, tile_w, tile_h };
	hextile_clips[Tiles::WHEAT_PORT] = { tile_w * 3, tile_h, tile_w, tile_h };
	hextile_clips[Tiles::ORE_PORT] = { tile_w * 4, tile_h, tile_w, tile_h };
	hextile_clips[Tiles::TRADE_PORTS] = { 0, tile_h * 2, tile_w, tile_h };
	hextile_clips[Tiles::DESERT_TILE] = { tile_w * 1, tile_h * 2, tile_w, tile_h };
	hextile_clips[Tiles::WATER_TILE] = { tile_w * 2, tile_h * 2, tile_w, tile_h };
	
	// setting the clips for the building clips
	memset(building_clips, 0, sizeof(building_clips));
	building_clips[building_t::SETTLEMENT] = { 0, 0, sprite_small[0], sprite_small[1] };
	building_clips[building_t::CITY] = { sprite_small[0], 0, sprite_small[0], sprite_small[1] };
	building_clips[building_t::THIEF] = { sprite_small[0] * 2, 0, sprite_small[0], sprite_small[1] };

	// setting the clips for the road clips
	memset(road_clips, 0, sizeof(road_clips));
	road_clips[Tiles::HEXNORTHEAST] = { face_size[0] * 2, face_size[1] * 2, face_size[0], face_size[1] };
	road_clips[Tiles::HEXEAST] = { 0, sprite_small[1], sprite_small[0], sprite_small[1]};
	road_clips[Tiles::HEXSOUTHEAST] = { face_size[0] * 2, face_size[1] * 3, face_size[0], face_size[1] };
	road_clips[Tiles::HEXSOUTHWEST] = { face_size[0] , face_size[1] * 3, face_size[0], face_size[1] };
	road_clips[Tiles::HEXWEST] = { 0, sprite_small[1], sprite_small[0], sprite_small[1] };
	road_clips[Tiles::HEXNORTHWEST] = { face_size[0] , face_size[1] * 2, face_size[0], face_size[1] };

	// the offsets needed to place a vertex ( the point on a hextile)
	vertex_pos[0][0] = tile_w/2;
	vertex_pos[0][1] = 0;
	vertex_pos[1][0] = tile_w;
	vertex_pos[1][1] = tile_h/4;
	vertex_pos[2][0] = tile_w;
	vertex_pos[2][1] = (int)(tile_h *0.75);
	vertex_pos[3][0] = tile_w/2;
	vertex_pos[3][1] = tile_h;
	vertex_pos[4][0] = 0;
	vertex_pos[4][1] = (int)(tile_h*0.75);
	vertex_pos[5][0] = 0;
	vertex_pos[5][1] = tile_h/4;

	// the offsets needed to place a face 
	face_pos[0][0] = tile_w/2;
	face_pos[0][1] = 0;
	face_pos[1][0] = (int)(tile_w*0.75);
	face_pos[1][1] = tile_h/4;
	face_pos[2][0] = tile_w / 2;
	face_pos[2][1] = (int)(tile_h*0.75);
	face_pos[3][0] = 0;
	face_pos[3][1] = (int)(tile_h*0.75);
	face_pos[4][0] = -tile_w/4;
	face_pos[4][1] = tile_h/4;
	face_pos[5][0] = 0;
	face_pos[5][1] = 0;

	return true;
}
bool View_Game::load_player_building_textures(const char* file){
	if(!file){ return false; }
	Logger& logger = Logger::getLog("jordan.log");
	SDL_Surface* surface = nullptr;
	SDL_Palette* palette = nullptr;
	SDL_Color colors[256];

	do{
		// get the buildings spritesheet surface
		surface = Util::load_surface(file);
		if(surface == nullptr){
			logger.SDL_log(Logger::ERROR, "View_Game::load_player_building_textures load_surface() %s", file);
			break;
		}

		// Alloc a palette to use
		palette = SDL_AllocPalette(256);
		if(palette == nullptr){
			logger.SDL_log(Logger::ERROR, "View_Game::load_player_building_textures() SDL_AllocPalette(%d)", 256);
			break;
		}

		// set default colours;
		for(int i = 0; i < 256; ++i){
			colors[i] = surface->format->palette->colors[i];
		}

		// for every player create a texture for their building colours
		int rs = 0;
		bool good_flag = true;
		for(int i = 0; i < model.get_num_players(); ++i){
			// custome 0,0,0 colour for the player
			colors[0] = model.get_player(i)->color;

			// set the colors to the palette
			rs = SDL_SetPaletteColors(palette, colors, 0, 256);
			if(rs != 0){
				good_flag = false;
				logger.SDL_log(Logger::ERROR, "load_player_building_textures() SDL_SetPaletteColors for player %i", i);
				break;
			}

			// set the custom palette for the surface
			rs = SDL_SetSurfacePalette(surface, palette);
			if(rs != 0){
				good_flag = false;
				logger.SDL_log(Logger::ERROR, "load_player_building_textures() SDL_SetSurfaceTexture for player %i", i);
				break;
			}

			// set the transparent color key
			rs = SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(surface->format, 255, 0, 255));
			if(rs != 0){
				good_flag = false;
				logger.SDL_log(Logger::ERROR, "load_player_building_textures() SDL_SetColorKey for player %i", i);
				break;
			}

			// create the texture
			SDL_Texture* tex = SDL_CreateTextureFromSurface(&ren, surface);
			if(tex == nullptr){
				logger.SDL_log(Logger::ERROR, "load_player_building_textures() SDL_CreateTextureFromSurface() for player %d", i);
				good_flag = false;
				break;
			}

			// push the texture onto the list
			player_buildings_spritesheet.push_back(tex);
		}

		// did we finish without errors?
		if(good_flag == false){ break; }
		SDL_FreePalette(palette);
		SDL_FreeSurface(surface);
		return true;
	} while(0);

	// some error has occured, so do any cleanup
	logger.log(Logger::DEBUG, "load_player_building_textures() Error in loading player building textures");
	SDL_FreePalette(palette);
	SDL_FreeSurface(surface);
	for(int i = 0; i < (int)player_buildings_spritesheet.size(); ++i){
		SDL_DestroyTexture(player_buildings_spritesheet[i]);
	}
	return false;
}


bool View_Game::setup_top_bar(pane_t& pane){
	return true;
}
bool View_Game::setup_board_pane(pane_t& pane){
	selected_tile = nullptr;
	selected_vertex = nullptr;
	selected_face = nullptr;
	tiles.reserve(model.get_board_height()*model.get_board_width());
	vertices.reserve(model.get_num_vertices());
	faces.reserve(model.get_num_faces());

	// setup all the tiles
	int x, y;
	for(int row = 0; row < model.get_board_height(); ++row){
		for(int col = 0; col < model.get_board_width(); ++col){			
			Tile_intersect temp; 
			getPixelPosFromTilePos(col, row, &x, &y);
			
			tiles.push_back(temp);
			tiles[tiles.size()-1].init(x + pane.padding/2, y + pane.padding/2,0, tile_w, tile_h, col, row);
		}
	}
	Logger::getLog("jordan.log").log(Logger::DEBUG, "View_Game::setup_board_pane() tiles.size() = %d", tiles.size());

	// setup all the vertices
	int pos =0;
	int tile_x, tile_y;
	std::vector<vertex_face_t>& vertex_array = model.get_vertex_array();
	std::vector<vertex_face_t>::iterator it;

	for(it = vertex_array.begin(); it != vertex_array.end(); ++it){
		x = y = 0;
		tile_x = tile_y = 0;
		vertex_face_t_intersect temp;
		vertices.push_back(temp);

		int col = it->vert.tiles[0][0];
		int row = it->vert.tiles[0][1];
		int dir = model.get_tile(col, row)->get_direction_of_vertex(pos);
		if(dir == -1){ continue; }
		// vertex_pos gives us the x,y point of the vertex of direction dir.
		x = vertex_pos[dir][0] - vertex_hit_w/2 + pane.padding/2;
		y = vertex_pos[dir][1] - vertex_hit_h/2 + pane.padding/2;

		// get the pixel pos of the given tile.
		getPixelPosFromTilePos(col, row, &tile_x, &tile_y);
		vertices[vertices.size() - 1].init(tile_x+x,tile_y+y,0,vertex_hit_w, vertex_hit_h,pos);
		pos++;
	}
	Logger::getLog("jordan.log").log(Logger::DEBUG, "View_Game::setup_board_pane() vertices.size() = %d", vertices.size());

	// setup all the face intersects
	pos = 0;
	tile_x = tile_y = 0;
	std::vector<vertex_face_t>& face_array = model.get_face_array();
	for(it = face_array.begin(); it != face_array.end(); ++it){
		x = y = 0;
		tile_x = tile_y = 0;
		vertex_face_t_intersect temp;
		faces.push_back(temp);

		// set the x and y point of the face inteserct block
		int col = it->face.tiles[0][0];
		int row = it->face.tiles[0][1];
		int dir = model.get_tile(col, row)->get_direction_of_face(pos);
		if(dir == -1){ continue; }
		int mid_x, mid_y;
		get_mid_point_from_face(dir,&mid_x,&mid_y);
		x = mid_x - face_hit_w / 2  + pane.padding / 2;
		y = mid_y - face_hit_h / 2 + pane.padding / 2;

		// get the pixel pos of the given tile
		getPixelPosFromTilePos(col, row, &tile_x, &tile_y);
		faces[faces.size() - 1].init(tile_x + x, tile_y + y,0, face_hit_w, face_hit_h,pos);
		pos++;
	}
	Logger::getLog("jordan.log").log(Logger::DEBUG, "View_Game::setup_board_pane() faces.size() = %d", faces.size());


	// print where the tile_intersect tiles are placed
	do{
		std::vector<Tile_intersect>::iterator it;
		for(it = tiles.begin(); it != tiles.end(); ++it){
			Logger::getLog("jordan.log").log(Logger::DEBUG, "tile_intersect= x=%d,y=%d,w=%d,h=%d,col=%d,row=%d,hitbox_x=%d,hitbox_y=%d",
				it->x, it->y, it->w, it->h, it->col, it->row,
				it->hitbox.getx(), it->hitbox.gety()
				);
		}
	} while(false);

	// print out where all vertex are placed
	do{
		std::vector<vertex_face_t_intersect>::iterator it;
		for(it = vertices.begin(); it != vertices.end(); ++it){
			Logger::getLog("jordan.log").log(Logger::DEBUG, "vertex_t_intersect= x=%d,y=%d,w=%d,h=%d,vertex_num=%d,hitbox_x=%d,hitbox_y=%d",
				it->x, it->y, it->w, it->h, it->num,
				it->hitbox.getx(), it->hitbox.gety()
				);
		}
	} while(false);
	// print out where all the face intersect tiles are placed
	do{
		std::vector<vertex_face_t_intersect>::iterator it;
		for(it = faces.begin(); it != faces.end(); ++it){
			Logger::getLog("jordan.log").log(Logger::DEBUG, "face_t_intersect= x=%d,y=%d,w=%d,h=%d,face_num=%d,hitbox_x=%d,hitbox_y=%d",
				it->x, it->y, it->w, it->h, it->num,
				it->hitbox.getx(), it->hitbox.gety()
				);
		}

	} while(false);

	return true;
}

bool View_Game::setup_buttons(pane_t& pane){
	int off_x, off_y;
	int vert_pad = 3;
	int horiz_pad = 3;
	int but_w = 80;
	int but_h = 32;

	// initialize the buttons, and determine their positions.
	
	off_x = horiz_pad;
	off_y = vert_pad;
	// the first row of buttons
	exit_button.init("EXIT",off_x, off_y,0, but_w, but_h);
	off_x += exit_button.w + horiz_pad;
	end_turn_button.init("End Turn", off_x, off_y,0, but_w, but_h);
	off_x += end_turn_button.w + horiz_pad;
	roll_button.init("Roll", off_x, off_y,0, but_w, but_h);
	off_x += roll_button.w + horiz_pad;
	enable_debug_button.init("Enable Debug", off_x, off_y, 0, but_w, but_h);
	off_x += enable_debug_button.w + horiz_pad;
	misc_button.init("Button", off_x, off_y, 0, but_w, but_h);
	off_x += misc_button.w + horiz_pad;
	
	off_x = horiz_pad;
	off_y += but_h + vert_pad;
	// second row of buttons
	add_road_button.init("Build Road", off_x, off_y,0, but_w, but_h);
	off_x += add_road_button.w + horiz_pad;
	add_settlement_button.init("Build Settlement", off_x, off_y,0, but_w, but_h);
	off_x += add_settlement_button.w + horiz_pad;
	add_city_button.init("Build City", off_x, off_y,0, but_w, but_h);
	off_x += add_city_button.w + horiz_pad;
	buy_dev_card_button.init("Buy Dev Card", off_x, off_y,0, but_w, but_h);
	off_x += buy_dev_card_button.w + horiz_pad;
	
	off_x = horiz_pad;
	off_y += but_h + vert_pad;
	// the third row of buttons
	play_dev_card_button.init("Play Dev Card", off_x , off_y,0, but_w, but_h);
	off_x += play_dev_card_button.w + horiz_pad;
	trade_button.init("Trade", off_x, off_y,0, but_w, but_h);
	off_x += trade_button.w + horiz_pad;

	// set the actions for all the button
	exit_button.set_action(exit_button_action);
	end_turn_button.set_action(end_turn_action);
	roll_button.set_action(roll_action);
	enable_debug_button.set_action(enable_debug_action);
	misc_button.set_action(empty_action);
	add_road_button.set_action(add_road_action);
	add_settlement_button.set_action(add_settlement_action);
	add_city_button.set_action(add_city_action);
	buy_dev_card_button.set_action(buy_dev_card_action);
	play_dev_card_button.set_action(play_dev_card_action);
	trade_button.set_action(trade_action);

	// add to the button list
	button_list.push_back(&exit_button);
	button_list.push_back(&end_turn_button);
	button_list.push_back(&roll_button);
	button_list.push_back(&enable_debug_button);
	button_list.push_back(&misc_button);
	button_list.push_back(&add_road_button);
	button_list.push_back(&add_settlement_button);
	button_list.push_back(&add_city_button);
	button_list.push_back(&buy_dev_card_button);
	button_list.push_back(&play_dev_card_button);
	button_list.push_back(&trade_button);

	// set the padding for each button
	std::vector<Button*>::iterator it;
	for(it = button_list.begin(); it != button_list.end(); ++it){
		(*it)->set_pad(horiz_pad, vert_pad);
	}
	return true;
}

void View_Game::set_state(state_e new_state){
	if(new_state < 0 || new_state >= state_e::NUM_state_e){ return; }
	state = new_state;
}

void View_Game::on_start(SDL_Event& e){	
	if(e.type >= SDL_USEREVENT){
		Logger::getLog("jordan.log").log(Logger::DEBUG, "View_Game::on_start()");
		fps_timer->start();
	}
}
void View_Game::on_switch(SDL_Event& e){
	if(e.type >= SDL_USEREVENT){
		Logger::getLog("jordan.log").log(Logger::DEBUG, "View_Game::on_switch()");
		fps_timer->stop();
	}	
}
void View_Game::on_close(SDL_Event& e){
	if(e.type >= SDL_USEREVENT){
		Logger::getLog("jordan.log").log(Logger::DEBUG, "View_Game::on_switch()");
		fps_timer->stop();
	}
}

void View_Game::handle_keyboard_events(SDL_Event& e){
	// handle the key board given the state.
	handle_keyboard_given_state(e,state);

	//generic handling of the keyboard
	if(e.type == SDL_KEYDOWN){
		if(e.key.keysym.scancode == SDL_SCANCODE_ESCAPE){
			SDL_Event ev;
			SDL_zero(ev);
			ev.type = SDL_QUIT;
			ev.quit.type = SDL_QUIT;
			ev.quit.timestamp = SDL_GetTicks();
			SDL_PushEvent(&ev);
			return;
		}

		int x, y;
		bool move_mouse = false;
		SDL_GetMouseState(&x, &y);
		const Uint8* keyboard = SDL_GetKeyboardState(NULL);
		if(keyboard[SDL_SCANCODE_W]){ y--; move_mouse = true; }
		if(keyboard[SDL_SCANCODE_S]){ y++; move_mouse = true; }
		if(keyboard[SDL_SCANCODE_D]){ x++; move_mouse = true; }
		if(keyboard[SDL_SCANCODE_A]){ x--; move_mouse = true; }
		if(move_mouse){
			SDL_WarpMouseInWindow(&win, x, y);
		}
	
		// cycle through the error codes
		if(keyboard[SDL_SCANCODE_1] ){			
			int dir = (keyboard[SDL_SCANCODE_LSHIFT]) ? -1 : 1;
			int code = (model.get_error() + dir);
			if(code < 0){code = Model::NUM_model_error_codes_e + code;
			} else{code %= Model::NUM_model_error_codes_e;}
			model.set_error((Model::model_error_codes_e)code);

			// set the message pane to show the error
			message_pane.setMessage(View_Game::view_game_model_error_strings[model.get_error()].c_str());
			message_pane.reset();
		}

		// cycle through the debug data codes
		if(keyboard[SDL_SCANCODE_2]){
			int dir = (keyboard[SDL_SCANCODE_LSHIFT]) ? -1 : 1;
			int value = this->debug_data + dir;
			if(value < 0){ this->debug_data = 3 + value; }
			else{ this->debug_data = (this->debug_data + dir) % 3; }
		}

		if(keyboard[SDL_SCANCODE_3]){
			enable_debug_button.action(*this, model);
		}
		if(keyboard[SDL_SCANCODE_4]){
			set_state(View_Game::START);
		}
		if(keyboard[SDL_SCANCODE_5]){
			debug_tiles = (debug_tiles) ? false : true;
		}

		if(keyboard[SDL_SCANCODE_6]){
			debug_vertices = (debug_vertices) ? false : true;
		}
		if(keyboard[SDL_SCANCODE_7]){
			debug_faces = (debug_faces) ? false : true;
		}

	} else if(e.type == SDL_KEYUP){

	}
}

void View_Game::handle_mouse_events(SDL_Event& e){

	// we must determine what hex tiles/vertex/face we may have intersected with.
	// TODO: This can be simplified if we place objects into pane objects
	// which each have their own mouse,keyboard, and update methods
	mouse.buttons = SDL_GetMouseState(&mouse.x, &mouse.y);
	if(e.type == SDL_MOUSEBUTTONDOWN){
		std::vector<Button*>::iterator it;
		for(it = button_list.begin(); it != button_list.end(); ++it){
			if((*it)->hit_flag){
				(*it)->action(*this, model);
			}
		}
		draw_tile = true;
		draw_vertex = true;
		draw_face = true;
	} else if(e.type == SDL_MOUSEBUTTONUP){
		draw_tile = false;
		draw_vertex = false;
		draw_face = false;
	} else if(e.type == SDL_MOUSEMOTION){

	}

	// handle mouse input depengin on the specific state
	handle_mouse_given_state(e,state);
}

void View_Game::handle_user_events(SDL_Event& e){
	if(e.type < SDL_USEREVENT){return;}
	// check for the timer events
	if(e.user.type == TimerFactory::get().event_type()){
		// check for the fps timer
		if((Timer*)e.user.data1 == fps_timer){
			draw_flag = true;
			fps.update();

			// time-out any messages on the screen.
			message_pane.tick();
			if(message_pane.just_finished){
				model.set_error(Model::MODEL_ERROR_NONE);
				message_pane.setMessage(nullptr);
			}
		} else{
			// do nothing
		}
	} else{
		// do nothing
	}
}


/*
Almost Obsolete.
Given a pixelx and pixel y determine the tile col and row;
it assumes that x=0,y=0 is start of the rect for tile col=0, row=0.
*/
void View_Game::getTilePosFromPixelPos(int px, int py, int* tile_x, int* tile_y){
	//*tile_y = ((int)(py*1.25) / tile_h);
	//*tile_y = ((int)(py / 0.875) / tile_h);
	int offset = (int)(tile_h*0.75);
	int y_min = py / offset;
	*tile_y = (py > (y_min + 1)*(offset)+10) ? y_min + 1 : y_min;

	if((*tile_y) % 2 == 1){
		*tile_x = (px - tile_w / 2) / tile_w;
	} else {
		*tile_x = px / tile_w;
	}
}
/*
returns the pixel position of the tile given the col and row
the pixel position is relative to a x=0,y=0 origin point for tile col=0, row = 0
*/
void View_Game::getPixelPosFromTilePos(int tile_col, int tile_row, int* px, int* py){
	*px = (int)(tile_col*tile_w + (tile_row % 2 == 1)*(tile_w / 2));
	*py = (int)(tile_row*(tile_h*0.75));
}

/*
Conditions:
	0 <= px <= tile_w
	0 <= py <= tile_h
	hextile_surface exists
	@return -1 if none selected
*/
int View_Game::getTilehexDir(int px, int py){
	Uint32 pixel = Util::get_pixel32(hextile_surface, px,py);
	Uint8 r, g, b;
	SDL_GetRGB(pixel, hextile_surface->format, &r, &g, &b);
	if(r == 255 && g == 0 && b == 0){return Tiles::HEXNORTHWEST;
	} else if(r == 0 && g == 255 && b == 0){return Tiles::HEXNORTHEAST;
	} else if(r == 0 && g == 0 && b == 255){return Tiles::HEXSOUTHEAST;
	} else if(r == 255 && g == 255 && b == 0){return Tiles::HEXSOUTHWEST;
	} else{return -1;}
}

/*
 0 <= px <= tile_w;
 0 <= py <= tile_h;
 hextile_face_surface exists
 @returns -1 if none selected
*/
int View_Game::getHexFaceDir(int px, int py){
	Uint32 pixel = Util::get_pixel32(hextile_face_surface, px, py);
	Uint8 r, g, b;
	SDL_GetRGB(pixel,hextile_face_surface->format, &r, &g, &b);
	if(r == 255 && g == 0 && b == 0){return Tiles::HEXSOUTHEAST;
	} else if(r == 0 && g == 255 && b ==0 ){return Tiles::HEXSOUTHWEST;
	} else if(r == 0 && g == 255 && b == 255){return Tiles::HEXEAST;	
	} else if(r == 0 && g == 0 && b == 255){return Tiles::HEXNORTHWEST;
	} else if(r == 255 && g == 255 && b == 0){return Tiles::HEXNORTHEAST;
	} else if(r == 255 && g == 0 && b == 255){return Tiles::HEXWEST;
	} else { return -1; }
}
bool View_Game::getTilePosFromDirection(int dir, int tilex, int tiley, int* new_tile_x, int* new_tile_y){
	if(!new_tile_x || !new_tile_y){ return false; }
	return Tiles::get_adjacent(dir, tilex, tiley, new_tile_x, new_tile_y);		
}
void View_Game::get_mid_point_from_face(int face, int* x, int* y){
	if(!x || !y){ return; }
	switch(face){
	case(Tiles::HEXNORTHEAST):{ *x= (int)(tile_w*0.75); *y = tile_h/8;}break;
	case(Tiles::HEXEAST):{ *x= tile_w; *y = tile_h/2;}break;
	case(Tiles::HEXSOUTHEAST):{ *x= (int)(tile_w*0.75); *y = tile_h - tile_h/8;}break;
	case(Tiles::HEXSOUTHWEST):{ *x= tile_w/4; *y = tile_h- tile_h/8;}break;
	case(Tiles::HEXWEST):{ *x= 0; *y = tile_h/2;}break;
	case(Tiles::HEXNORTHWEST) : { *x = tile_w / 4; *y = tile_h / 8; }break;
	}
}

// Update all the objects with information from the keyboard and mouse;
void View_Game::update(SDL_Event& e){
	update_check_for_collisions();
	// animations and such	
}
void View_Game::update_check_for_collisions(){
	static int x, y;
	static Collision rel_mouse_hitbox;
	static bool ready = false;
	if(ready == false){
		ready = true;
		rel_mouse_hitbox.hook(&x, &y);
		rel_mouse_hitbox.add_rect(0, 0, 0, 1, 1);
	}
	x = mouse.x;
	y = mouse.y;

	//	Logger::getLog("jordan.log").log(Logger::DEBUG, "mouse.hitbox(%d,%d)", mouse.hitbox.getx(), mouse.hitbox.gety());
	// TODO: Need a better way to tell which button is being hit
	std::vector<Button*>::iterator it;
	for(it = button_list.begin(); it != button_list.end(); ++it){
		(*it)->hit_flag = false;
	}
	selected_tile = nullptr;
	selected_vertex = nullptr;
	selected_face = nullptr;

	// check for collisions
	if(top_pane.hitbox.collides(mouse.hitbox)){
		// Handle top-pane instructions
		selected_pane = &top_pane;
		x -= selected_pane->x;
		y -= selected_pane->y;
		update_top_pane(top_pane, rel_mouse_hitbox);
	} else if(mid_pane.hitbox.collides(mouse.hitbox)){
		// Handle mid-pane intersections
		selected_pane = &mid_pane;
		x -= selected_pane->x;
		y -= selected_pane->y;
		update_mid_pane(mid_pane, rel_mouse_hitbox);
	} else if(bot_pane.hitbox.collides(mouse.hitbox)){
		// Handle bot-pane intersections
		// update the model state
		selected_pane = &bot_pane;
		x -= selected_pane->x;
		y -= selected_pane->y;
		update_bot_pane(bot_pane, rel_mouse_hitbox);
	} else{
		// No Pane was selected
		selected_pane = nullptr;
		selected_vertex = nullptr;
		selected_face = nullptr;
		selected_tile = nullptr;
	}
}

void View_Game::update_top_pane(pane_t& pane, Collision& rel_mouse_hitbox){
	// do nothing?
}
void View_Game::update_mid_pane(pane_t& pane, Collision& rel_mouse_hitbox){
	// see if we intersect with a tile
	int px, py;
	this->selected_tile = nullptr;
	std::vector<Tile_intersect>::iterator it;
	for(it = tiles.begin(); it != tiles.end(); ++it){
		if(it->hitbox.collides(rel_mouse_hitbox)){
			this->selected_tile = &(*it);

			// check if we want an adjacent tile due to our mouse positioning
			// ! We need to account for the affects of padding in the pane				
			getPixelPosFromTilePos(it->col, it->row, &px, &py);
			px = rel_mouse_hitbox.getx() - px - mid_pane.padding / 2;
			py = rel_mouse_hitbox.gety() - py - mid_pane.padding / 2;
			tilehex_pos = getTilehexDir(px, py);
			if(this->tilehex_pos != -1){
				getTilePosFromDirection(tilehex_pos, it->col, it->row, &this->tile_col, &this->tile_row);
				if(model.get_tile(this->tile_col, this->tile_row) == nullptr ||
					model.get_tile(this->tile_col, this->tile_row)->active == 0)
				{
					// tile does not exist on the game board, therefore
					// clear the selection and stop looking
					this->selected_tile = nullptr;
					this->tile_col = -1;
					this->tile_row = -1;
					break;
				} else{
					this->selected_tile = &tiles[this->tile_col + this->tile_row*model.get_board_width()];
				}
			}

			// set the selectred col and row
			this->tile_col = selected_tile->col;
			this->tile_row = selected_tile->row;
			break;
		}
	}

	// see if we intersect with one of the vertices
	do{
		selected_vertex = nullptr;
		std::vector<vertex_face_t_intersect>::iterator it;
		for(it = vertices.begin(); it != vertices.end(); ++it){
			if(it->hitbox.collides(rel_mouse_hitbox)){
				selected_vertex = &(*it);
			}
		}
	} while(false);

	// see if we intersect with one of the face intersects
	do{
		selected_face = nullptr;
		std::vector<vertex_face_t_intersect>::iterator it;
		for(it = faces.begin(); it != faces.end(); ++it){
			if(it->hitbox.collides(rel_mouse_hitbox)){
				selected_face = &(*it);
			}
		}
	} while(false);
}

void View_Game::update_bot_pane(pane_t& pane, Collision& rel_mouse_hitbox){
	std::vector<Button*>::iterator it;
	for(it = button_list.begin(); it != button_list.end(); ++it){
		(*it)->hit_flag = ((*it)->hitbox.collides(rel_mouse_hitbox));
	}
}


// screen offset in pixels
void View_Game::render_model(pane_t& pane){
	render_model_board_tiles(pane);
	render_model_face_vertex_tiles(pane);
	render_model_debug(pane);
	render_model_selected(pane);
}
void View_Game::render_model_board_tiles(pane_t& pane){
	SDL_Color black = { 0, 0, 0, 255 };
	int screen_offset_x = pane.x + pane.padding / 2;
	int screen_offset_y = pane.y + pane.padding / 2;
	int c_offset = tile_w / 2 - 5;
	int r_offset = tile_h / 2 - 5;
	int c, r;
	int type, roll;

	// render all the game tiles
	for(int row = 0; row < model.get_board_height(); ++row){
		for(int col = 0; col < model.get_board_width(); ++col){
			getPixelPosFromTilePos(col, row, &c, &r);
			r += screen_offset_y;
			c += screen_offset_x;
			type = model.get_tile(col, row)->type;
			roll = model.get_tile(col, row)->type;
		

			if(type != 0){
				Util::render_texture(&ren, hextile_spritesheet, c, r, &hextile_clips[type]);
				// only render the roll if the tile is not a water tile or desert tile
				if(!(model.get_tile(0, 0)->is_water_tile(type) ||type == Tiles::DESERT_TILE) )
				{
					Util::render_text(&ren, font_carbon_12, c + c_offset, r + r_offset, black, "%d", roll);
				}
				
			} else{
				// render a debug rectangle instead
				if(debug){
					SDL_Rect clip = { tile_w * 4, tile_h * 2, tile_w, tile_h };
					Util::render_texture(&ren, hextile_spritesheet, c, r, &clip);
				}
			}
		}
	}

}
void View_Game::render_model_face_vertex_tiles(pane_t& pane){
	memset(vertex_covered, 0, model.get_num_vertices()*sizeof(Uint8));
	memset(face_covered, 0, model.get_num_faces()*sizeof(Uint8));
	int screen_offset_x = pane.x + pane.padding / 2;
	int screen_offset_y = pane.y + pane.padding / 2;
	int height_overlap = (int)(tile_h*0.75);
	int target, type;
	int r, c;

	// render all the roads/faces and Vertices(settlements/cities)
	for(int row = 0; row < model.get_board_height(); ++row){
		for(int col = 0; col < model.get_board_width(); ++col){
			getPixelPosFromTilePos(col, row, &c, &r);
			r += screen_offset_y;
			c += screen_offset_x;
			if(model.get_tile(col, row)->type == 0){
				continue;
			};

			// render all the faces
			for(int i = 0; i < 6; ++i){
				target = model.get_tile(col,row)->faces[i];
				if(target == -1 || face_covered[target] == 1){
					continue;
				}

				face_covered[target] = 1;
				type = model.get_face(target)->type;
				//type = model.get_type_from_face(target);
				// TODO: the types for this are wrong. it is currently set to 
				// types of vertex_face_t, but out clipping rects use building_t
				// enumerations to indetify the clip.
				//type = building_t::ROAD;
				if(type == vertex_face_t::NONE){ continue; }
				int x = c + face_pos[i][0];
				int y = r + face_pos[i][1];

				Util::render_texture(&ren, player_buildings_spritesheet[model.get_face(target)->player], x, y, &road_clips[i]);
			}

			// render all the vertices
			for(int i = 0; i < 6; ++i){
				target = model.get_tile(col, row)->vertices[i];
				//target = model.get_vertices_from_tile(col, row)[i];
				if(target == -1 || vertex_covered[target] == 1){
					continue;
				}

				vertex_covered[target] = 1;
				int type = model.get_vertex(target)->type;
				//int type = model.get_type_from_vertex(target);
				//type = vertex_face_t::CITY;
				if(type == vertex_face_t::NONE){ continue; }

				int x = c + vertex_pos[i][0] - sprite_small[0] / 2;
				int y = r + vertex_pos[i][1] - sprite_small[1] / 2;
				Util::render_texture(&ren, player_buildings_spritesheet[model.get_vertex(target)->player], x, y, &building_clips[type]);
			}

		}
	}
}

void View_Game::render_model_selected(pane_t& pane){
	int screen_offset_x = pane.x + pane.padding / 2;
	int screen_offset_y = pane.y + pane.padding / 2;
	int c, r;

	// highlight the target hex
	if(draw_tile && selected_tile != nullptr){
		getPixelPosFromTilePos(tile_col, tile_row, &c, &r);
		r += screen_offset_y;
		c += screen_offset_x;
		SDL_Rect rect = { c, r, tile_w, tile_h };
		Util::render_rectangle(&ren, &rect, Util::colour_blue());
	}
	// highlight the selected vetex
	if(draw_vertex && selected_vertex != nullptr){
		SDL_Rect rect = { selected_vertex->x + pane.x,
			selected_vertex->y + pane.y,
			selected_vertex->w,
			selected_vertex->h };
		Util::render_rectangle(&ren, &rect, Util::colour_blue());
	}
	// highlight the seleceted face
	if(draw_face && selected_face != nullptr){
		SDL_Rect rect = { selected_face->x + pane.x,
			selected_face->y + pane.y,
			selected_face->w,
			selected_face->h };
		Util::render_rectangle(&ren, &rect, Util::colour_blue());
	}
}

void View_Game::render_model_debug(pane_t& pane){
	if(debug){
		// draw all the intersect rectangles
		if( debug_tiles){
			//break;
			std::vector<Tile_intersect>::iterator it;
			for(it = tiles.begin(); it != tiles.end(); ++it){
				SDL_Rect rect = { it->x + pane.x, it->y + pane.y, it->w, it->h };
				Util::render_rectangle(&ren, &rect, Util::colour_white());
			}
		}

		// render all the intersect vertices
		if(debug_vertices){
			std::vector< vertex_face_t_intersect>::iterator it;
			for(it = vertices.begin(); it != vertices.end(); ++it){
				SDL_Rect rect = { it->x + pane.x, it->y + pane.y, it->w, it->h };
				Util::render_rectangle(&ren, &rect, Util::colour_white());
			}
		}
		// render all the intersect faces
		if(debug_faces){
			std::vector< vertex_face_t_intersect>::iterator it;
			for(it = faces.begin(); it != faces.end(); ++it){
				SDL_Rect rect = { it->x + pane.x, it->y + pane.y, it->w, it->h };
				Util::render_rectangle(&ren, &rect, Util::colour_white());
			}
		}
	}
}

void View_Game::render_buttons(pane_t& pane){
	std::vector<Button*>::iterator it;
	SDL_Rect rect;
	for(it = button_list.begin(); it != button_list.end(); ++it){
		rect = { (*it)->x + pane.x,
			(*it)->y + pane.y,
			(*it)->w,
			(*it)->h };
		if((*it)->hit_flag){
			Util::render_rectangle(&ren, &rect, Util::colour_blue());
		} else{
			Util::render_rectangle(&ren, &rect, Util::colour_green());
		}
		Util::render_text(&ren, font_carbon_12, rect.x, rect.y, font_carbon_12_colour, "%s", (*it)->text.c_str());
	}
}
void View_Game::render_bottom_text(pane_t& pane){
	int off_x, off_y;
	int line_spacing = 18;
	SDL_Rect rect = { 0, 0, 0, 0 };
	Player* player = model.get_player(model.get_current_player());
	if(player == nullptr){ return; }

	// who is the current player
	off_x = 0;
	off_y = 3 * 30 + 3 * 5 + 5;
	rect = {0,pane.y + off_y, 80, 18 };
	Util::render_rectangle(&ren, &rect, player->color);
	Util::render_text(&ren, font_carbon_12, pane.x + off_x + rect.w +5,pane.y + off_y, font_carbon_12_colour,
		"Player %d : %s Victory Points=%d", model.get_current_player(),player->name.c_str(),
		model.num_victory_points_for_player(model.get_current_player()));

	// how many resources does the player have?
	off_x = 0;
	off_y += line_spacing;
	Util::render_text(&ren, font_carbon_12, pane.x + off_x, pane.y + off_y, font_carbon_12_colour,
		"Resources: wheat=%d sheep=%d brick=%d ore=%d wood=%d",
		player->resources.res[resource_t::WHEAT],
		player->resources.res[resource_t::SHEEP],
		player->resources.res[resource_t::BRICK],
		player->resources.res[resource_t::ORE],
		player->resources.res[resource_t::WOOD]
		);

	// show the number of buildings the player has left
	off_x = 0;
	off_y += line_spacing;
	Util::render_text(&ren, font_carbon_12, pane.x + off_x, pane.y + off_y, font_carbon_12_colour,
		"Buildings: roads=%d settlements=%d city=%d",
		player->building_cap[building_t::ROAD],
		player->building_cap[building_t::SETTLEMENT],
		player->building_cap[building_t::CITY]
		);

	// The active dev cards
	off_x = 0;
	off_y += line_spacing;
	int num_dev_cards[dev_cards_t::NUM_OF_DEV_CARDS][2];
	memset(num_dev_cards,0,dev_cards_t::NUM_OF_DEV_CARDS*sizeof(int)*2);
	std::vector<dev_cards_t>::iterator it;
	for(it = player->dev_cards.begin(); it != player->dev_cards.end(); ++it){
		if(it->visible){
			num_dev_cards[it->type][0]++;
		} else{
			num_dev_cards[it->type][0]++;
		}
	}	
	Util::render_text(&ren, font_carbon_12, pane.x + off_x, pane.y + off_y, font_carbon_12_colour,
		"Visible/Hidden Dev Cards: Soldier=%d/%d, Victory=%d/%d",
		num_dev_cards[dev_cards_t::SOLDIER][0], num_dev_cards[dev_cards_t::SOLDIER][1],
		num_dev_cards[dev_cards_t::VICTORY][0], num_dev_cards[dev_cards_t::VICTORY][1]
		);
	off_x = 160;
	off_y += line_spacing;
	Util::render_text(&ren, font_carbon_12, pane.x + off_x, pane.y + off_y, font_carbon_12_colour,
		"Monopoly=%d/%d, Year Plenty=%d/%d, Road Building=%d/%d",
		num_dev_cards[dev_cards_t::MONOPOLY][0], num_dev_cards[dev_cards_t::MONOPOLY][1],
		num_dev_cards[dev_cards_t::YEAR_PLENTY][0], num_dev_cards[dev_cards_t::YEAR_PLENTY][1],
		num_dev_cards[dev_cards_t::ROAD_BUILDING][0], num_dev_cards[dev_cards_t::ROAD_BUILDING][1]
		);

	// Who has the Largest army and longest road cards
	// TODO: longest road  and largest army.
	off_x = 0;
	off_y += line_spacing;
	Util::render_text(&ren, font_carbon_12, pane.x + off_x, pane.y + off_y, font_carbon_12_colour,
		"Longest road %d segments by player %d ",0,model.get_player_with_longest_road());
	off_x = 0;
	off_y += line_spacing;
	Util::render_text(&ren, font_carbon_12, pane.x + off_x, pane.y + off_y, font_carbon_12_colour,
		"Largest army %d soldiers owned by player %d",0, model.get_player_with_largest_army());
}

void View_Game::render_top_pane(pane_t& pane){
	// draw the mouse position
	if(debug){
		int mouse_x, mouse_y;
		Uint32 buttons;
		buttons = SDL_GetMouseState(&mouse_x, &mouse_y);
		Util::render_text(&ren, font_carbon_12, top_pane.x, top_pane.y, font_carbon_12_colour,
			"Mouse(%d,%d,%d)  Debug Data=%d State=%d", mouse_x, mouse_y, buttons, this->debug_data,(int)state);
		// draw the fps 
		Util::render_text(&ren, font_carbon_12, top_pane.x, top_pane.y + 18, font_carbon_12_colour,
			"FPS:%d.%d", fps.fps, total_frame_count);


		if(selected_tile != nullptr && this->debug_data == 0){
			Util::render_text(&ren, font_carbon_12, top_pane.x, top_pane.y + 36, font_carbon_12_colour,
				"Tile col=%d row=%d x=%d y=%d w=%d h=%d",
				selected_tile->col, selected_tile->row,
				selected_tile->x, selected_tile->y,
				selected_tile->w, selected_tile->h
				);


			Tiles* t = model.get_tile(selected_tile->col, selected_tile->row);

			if(t == nullptr){
				Util::render_text(&ren, font_carbon_12, top_pane.x, top_pane.y + 48, font_carbon_12_colour, "Model Tile = nullptr");
			} else{
				Util::render_text(&ren, font_carbon_12, top_pane.x, top_pane.y + 48, font_carbon_12_colour,
					"Model Tile active=%d type=%d roll=%d ringleve=%d",
					t->active, t->type, t->roll, t->ring_level);

				Util::render_text(&ren, font_carbon_12, top_pane.x, top_pane.y + 64, font_carbon_12_colour,
					"vertices { %d %d %d %d %d %d } faces { %d %d %d %d %d %d }",
					t->vertices[0], t->vertices[1], t->vertices[2], t->vertices[3], t->vertices[4], t->vertices[5],
					t->faces[0], t->faces[1], t->faces[2], t->faces[3], t->faces[4], t->faces[5]
					);
			}


		}
		if(selected_vertex != nullptr && this->debug_data == 1){
			Util::render_text(&ren, font_carbon_12, top_pane.x, top_pane.y + 36, font_carbon_12_colour,
				"Vertex [num=%d] : x=%d y=%d z=%d w=%d h=%d",
				selected_vertex->num,
				selected_vertex->x, selected_vertex->y, selected_vertex->z,
				selected_vertex->w, selected_vertex->h
				);

			vertex_face_t* vertex = model.get_vertex(selected_vertex->num);
			if(vertex == nullptr){
				Util::render_text(&ren, font_carbon_12, top_pane.x, top_pane.y + 48, font_carbon_12_colour, "Model Vertex = nullptr");
			} else{
				Util::render_text(&ren, font_carbon_12, top_pane.x, top_pane.y + 48, font_carbon_12_colour,
					"Model Vertex: %d,%d,%d  faces { %d,%d,%d } vertex { %d,%d,%d }",
					vertex->type, vertex->player, vertex->is_assigned,
					vertex->vert.faces[0], vertex->vert.faces[1], vertex->vert.faces[2],
					vertex->vert.vertices[0], vertex->vert.vertices[1], vertex->vert.vertices[2]
					);
				Util::render_text(&ren, font_carbon_12, top_pane.x + 50, top_pane.y + 64, font_carbon_12_colour,
					"tiles = { (%d, %d), (%d, %d), (%d, %d) }",
					vertex->vert.tiles[0][0], vertex->vert.tiles[0][1],
					vertex->vert.tiles[1][0], vertex->vert.tiles[1][1],
					vertex->vert.tiles[2][0], vertex->vert.tiles[2][1]
					);
			}
		}
		if(selected_face != nullptr && this->debug_data == 2){
			Util::render_text(&ren, font_carbon_12, top_pane.x, top_pane.y + 36, font_carbon_12_colour,
				"Face [num=%d]: x=%d y=%d z=%d w=%d h=%d",
				selected_face->num,
				selected_face->x, selected_face->y, selected_face->z,
				selected_face->w, selected_face->h
				);

			vertex_face_t* face = model.get_face(selected_face->num);
			if(face == nullptr){
				Util::render_text(&ren, font_carbon_12, top_pane.x, top_pane.y + 48, font_carbon_12_colour, "Model Face = nullptr");
			} else{
				Util::render_text(&ren, font_carbon_12, top_pane.x, top_pane.y + 48, font_carbon_12_colour,
					"Model Face: %d,%d,%d, faces { %d,%d,%d,%d } vertex { %d,%d } ",
					face->type, face->player, face->is_assigned,
					face->face.faces[0], face->face.faces[1], face->face.faces[2], face->face.faces[3],
					face->face.vertices[0], face->face.vertices[1]
					);

				Util::render_text(&ren, font_carbon_12, top_pane.x + 50, top_pane.y + 64, font_carbon_12_colour,
					"tiles { (%d,%d) (%d,%d) }",
					face->face.tiles[0][0], face->face.tiles[0][1],
					face->face.tiles[1][0], face->face.tiles[1][1]
					);
			}
		}

		// draw a rectangle around the selected pane
		if(selected_pane != nullptr && true){
			SDL_Rect r = { selected_pane->x, selected_pane->y, selected_pane->w, selected_pane->h };
			SDL_Color c = { 255, 255, 0, 255 };
			Util::render_rectangle(&ren, &r, c);
		}
	}
}

void View_Game::render(){
	++total_frame_count;

	SDL_RenderClear(&ren);
	render_top_pane(top_pane);
	render_model(mid_pane);
	render_buttons(bot_pane);
	// draw the UI text for stuff...
	render_bottom_text(bot_pane);
		
	// draw the message box
	if(message_pane.isVisible()){
		SDL_Rect r = {
			message_pane.x, message_pane.y,
			message_pane.w, message_pane.h
		};
		// clear to black
		SDL_Color c = { 0,0,0, 255 };
		Util::render_fill_rectangle(&ren, &r, c);
		// draw the border
		c = { 255, 255, 0, 255 };
		Util::render_rectangle(&ren, &r, c);
		// draw the error text
		Util::render_text(&ren, font_carbon_12, message_pane.x, message_pane.y,
			font_carbon_12_colour, "%s",
			(message_pane.message == nullptr) ? "" : message_pane.message
			);
	}

	// draw the misc pane
	if(misc_pane.isVisible()){
		SDL_Rect r = {
			misc_pane.x, misc_pane.y,
			misc_pane.w, misc_pane.h
		};
		SDL_Color c = { 255, 255, 0, 255 };
		Util::render_rectangle(&ren, &r, c);
	}

	SDL_RenderPresent(&ren);
}

void  View_Game::handle_mouse_given_state(SDL_Event& ev,View_Game::state_e s){
	Logger& logger = Logger::getLog("jordan.log");

	if(ev.type == SDL_MOUSEBUTTONDOWN){
		if(s == View_Game::NONE)
		{
			//do nothing
			//logger.log(Logger::DEBUG, "View_Game::handle_mouse_given_state(%d) None State", (int)s);
		}
		else if(s == View_Game::START)
		{
			static bool once = false;
			if(once == false){
				once = true;
				logger.log(Logger::DEBUG, "View_Game::handle_mouse_given_state(%d) Start", (int)s);
				// do roll assignment. for now we just assume we start
				// from player one and then keep going.
				set_state(View_Game::PLACE_SETTLEMENT_1);
			} else{
				set_state(View_Game::NORMAL);
			}
		}
		else if(s == View_Game::PLACE_SETTLEMENT_1)
		{
			logger.log(Logger::DEBUG, "View_Game::handle_mouse_given_state(%d) Place Settlement 1 for player %d", (int)s,model.get_current_player());
			if(selected_vertex != nullptr){
				bool rs = model.build_building(
											building_t::SETTLEMENT,											
											selected_vertex->num,
											model.get_current_player() );
				if(rs){
					set_state(View_Game::PLACE_ROAD_1);
				}
			}
		}
		else if(s == View_Game::PLACE_ROAD_1)
		{
			logger.log(Logger::DEBUG, "View_Game::handle_mouse_given_state(%d) Place Roads 1 for player %d", (int)s,model.get_current_player());
			static int counter = 0;
			if(selected_face != nullptr){
				bool rs = model.build_building(
											building_t::ROAD,											
											selected_face->num,
											model.get_current_player() );
				if(rs){
					counter++;
				
					// switch either to the next placing of settlements, or 
					// to the next player of placing settlements.
					if(counter >= model.get_num_players()){
						set_state(View_Game::PLACE_SETTLEMENT_2);
						model.set_current_player(model.get_current_player());
						//model.m_current_player = model.m_current_player;
					} else{
						set_state(View_Game::PLACE_SETTLEMENT_1);
						model.set_current_player((model.get_current_player() + 1) % model.get_num_players());
						//model.m_current_player = (model.m_current_player + 1) % model.m_num_players;
					}				
				} // end if(rs)				
			}	
		}
		else if(s == View_Game::PLACE_SETTLEMENT_2)
		{
			logger.log(Logger::DEBUG, "View_Game::handle_mouse_given_state(%d) Place Settlement 2 for player %d", (int)s, model.get_current_player());
			if(selected_vertex != nullptr){
				bool rs = model.build_building(
											building_t::SETTLEMENT,							
											selected_vertex->num,
											model.get_current_player() );
				if(rs) { set_state(View_Game::PLACE_ROAD_2); }
			}
		}
		else if(s == View_Game::PLACE_ROAD_2)
		{
			logger.log(Logger::DEBUG, "View_Game::handle_mouse_given_state(%d) Place Road 2 for player %d", (int)s, model.get_current_player());
			static int counter = 0;
			if(selected_face != nullptr){
				bool rs = model.build_building(
											building_t::ROAD,											
											selected_face->num,
											model.get_current_player() );
				if(rs) { 
					counter++;
					// switch either to the next placing of settlements, or 
					// to the next player of placing settlements.
					if(counter >= model.get_num_players()){
						set_state(View_Game::START_RESOURCES);
						//model.m_current_player = 0;
						model.set_current_player(0);
					} else{
						set_state(View_Game::PLACE_SETTLEMENT_2);
						model.set_current_player(model.get_current_player() - 1);
						//model.m_current_player--;
						if(model.get_current_player() < 0){
							model.set_current_player(model.get_num_players());
							//model.m_current_player = model.m_num_players + model.m_current_player;
						}
					}
				}// end if rs()

			}
		}
		else if(s == View_Game::START_RESOURCES)
		{
			logger.log(Logger::DEBUG, "View_Game::handle_mouse_given_state(%d) Start Resources and Placing the Thief",(int)s);			
			// place the thief
			if(selected_tile != nullptr){
				// give resources.
				for(int i = model.get_num_dice(); i < model.get_num_dice()*model.get_num_dice_sides(); ++i){
					model.give_resources_from_roll(i);
				}

				// place the thief.
				model.place_thief(selected_tile->col, selected_tile->row);
				set_state(View_Game::NORMAL);
			}			
		}
		else if(s == View_Game::NORMAL)
		{
			// do nothing.
		}
		else if(s == View_Game::TRADING)
		{
			logger.log(Logger::DEBUG, "View_Game::handle_mouse_given_state(%d) Trading", (int)s);
			set_state(View_Game::NORMAL);
		}
		else if(s == View_Game::BUILD_SETTLEMENT)
		{
			logger.log(Logger::DEBUG, "View_Game::handle_mouse_given_state(%d) Building settlement", (int)s);
			set_state(View_Game::NORMAL);
		}
		else if(s == View_Game::BUILD_CITY)
		{
			logger.log(Logger::DEBUG, "View_Game::handle_mouse_given_state(%d) Building City", (int)s);
			set_state(View_Game::NORMAL);
		}
		else if(s == View_Game::BUILD_ROAD)
		{
			logger.log(Logger::DEBUG, "View_Game::handle_mouse_given_state(%d) Building Road", (int)s);
			set_state(View_Game::NORMAL);
		}
		else if(s == View_Game::PLAY_DEV_CARD)
		{
			logger.log(Logger::DEBUG, "View_Game::handle_mouse_given_state(%d) Playing Dev Card", (int)s);
			set_state(View_Game::NORMAL);
		}
	}else  if(ev.type == SDL_MOUSEBUTTONUP){
		if(s == View_Game::NONE){}
		else if(s == View_Game::START){}
		else if(s == View_Game::PLACE_SETTLEMENT_1){}
		else if(s == View_Game::PLACE_ROAD_1){}
		else if(s == View_Game::PLACE_SETTLEMENT_2){}
		else if(s == View_Game::PLACE_ROAD_2){}
		else if(s == View_Game::START_RESOURCES){}
		else if(s == View_Game::NORMAL){}
		else if(s == View_Game::TRADING){}
		else if(s == View_Game::BUILD_SETTLEMENT){}
		else if(s == View_Game::BUILD_CITY){}
		else if(s == View_Game::BUILD_ROAD){}
		else if(s == View_Game::PLAY_DEV_CARD){}
	} else if(ev.type == SDL_MOUSEMOTION){
		if(s == View_Game::NONE){}
		else if(s == View_Game::START){}
		else if(s == View_Game::PLACE_SETTLEMENT_1){}
		else if(s == View_Game::PLACE_ROAD_1){}
		else if(s == View_Game::PLACE_SETTLEMENT_2){}
		else if(s == View_Game::PLACE_ROAD_2){}
		else if(s == View_Game::START_RESOURCES){}
		else if(s == View_Game::NORMAL){}
		else if(s == View_Game::TRADING){}
		else if(s == View_Game::BUILD_SETTLEMENT){}
		else if(s == View_Game::BUILD_CITY){}
		else if(s == View_Game::BUILD_ROAD){}
		else if(s == View_Game::PLAY_DEV_CARD){}	
	}

	if(model.get_error() != Model::MODEL_ERROR_NONE){
		message_pane.setMessage(View_Game::view_game_model_error_strings[model.get_error()].c_str());
		message_pane.reset();
	}
}

void View_Game::handle_keyboard_given_state(SDL_Event& ev,View_Game::state_e s){
	if(s == View_Game::NONE){}
	else if(s == View_Game::START){}
	else if(s == View_Game::PLACE_SETTLEMENT_1){}
	else if(s == View_Game::PLACE_ROAD_1){}
	else if(s == View_Game::PLACE_SETTLEMENT_2){}
	else if(s == View_Game::PLACE_ROAD_2){}
	else if(s == View_Game::START_RESOURCES){}
	else if(s == View_Game::NORMAL){}
	else if(s == View_Game::TRADING){}
	else if(s == View_Game::BUILD_SETTLEMENT){}
	else if(s == View_Game::BUILD_CITY){}
	else if(s == View_Game::BUILD_ROAD){}
	else if(s == View_Game::PLAY_DEV_CARD){}
	else if(s == View_Game::NUM_state_e){}
}