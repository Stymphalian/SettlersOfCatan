#include <vector>
#include <list>

#include <SDL.h>
#include "Logger.h"
#include "Util.h"
#include "Model.h"
#include "IView.h"
#include "View_Game.h"
#include "Player.h"


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
	this->selected_pane = nullptr;

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

	// setup the three static panes to be used for this view
	int offset_x = 0;
	int offset_y = 0;
	int offset_z = 0;
	int padding = 0;
	getPixelPosFromTilePos(0,4, &offset_x, &offset_y);		
	this->top_pane.init(0, 0,offset_z,disp_w, offset_y);

	int board_x, board_y;
	getPixelPosFromTilePos(model.m_board_width + 1, model.m_board_height + 1, &board_x, &board_y);
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
	SDL_DestroyTexture(hextile_spritesheet);
	SDL_DestroyTexture(buildings_spritesheet);
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

	buildings_spritesheet = Util::load_texture("data/buildings_spritesheet.png", &ren);
	if(buildings_spritesheet == nullptr){
		logger.SDL_log(Logger::ERROR, "View_Game::load() load_texture data/buildings_spritesheet.png");
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
	vertex_covered = new Uint8[model.vertex_array.size()];
	// face covered
	face_covered = new Uint8[model.face_array.size()];

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

bool View_Game::setup_top_bar(pane_t& pane){
	return true;
}
bool View_Game::setup_board_pane(pane_t& pane){
	selected_tile = nullptr;
	selected_vertex = nullptr;
	selected_face = nullptr;
	tiles.reserve(model.m_board_size);
	vertices.reserve(model.vertex_array.size());
	faces.reserve(model.face_array.size());

	// setup all the tiles
	int x, y;
	for(int row = 0; row < model.m_board_height; ++row){
		for(int col = 0; col < model.m_board_width; ++col){			
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
	std::vector<vertex_face_t>::iterator it;
	for(it = model.vertex_array.begin(); it != model.vertex_array.end(); ++it){
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
	for(it = model.face_array.begin(); it != model.face_array.end(); ++it){
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
	if(new_state < 0 || new_state >= state_e::NUM_OF_state_e){ return; }
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
	} else if(e.type == SDL_KEYUP){

	}
}

void View_Game::handle_mouse_events(SDL_Event& e){
	// find the hex it intersects with	
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
}

void View_Game::handle_user_events(SDL_Event& e){
	if(e.type >= SDL_USEREVENT){		
		// check for the fps timer events
		if(e.user.type == TimerFactory::get().event_type()){			
			draw_flag = true;
			fps.update();
		}
	}
}

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
	Tiles& tile = model.m_board[0];
	return tile.get_adjacent(dir, tilex, tiley, new_tile_x, new_tile_y);		
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
	static int x,y;
	static Collision rel_mouse_hitbox;
	static bool ready = false;
	if(ready == false){
		ready = true;
		rel_mouse_hitbox.hook(&x, &y);
		rel_mouse_hitbox.add_rect(0, 0,0, 1, 1);
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
		x -= top_pane.x;
		y -= top_pane.y;
	} else if(mid_pane.hitbox.collides(mouse.hitbox)){
		// Handle mid-pane intersections
		selected_pane = &mid_pane;
		x -= mid_pane.x;
		y -= mid_pane.y;
		
		// see if we intersect with a tile
		int px, py;
		selected_tile = nullptr;		
		std::vector<Tile_intersect>::iterator it;		
		for(it = tiles.begin(); it != tiles.end(); ++it){	
			if(it->hitbox.collides(rel_mouse_hitbox)){
				selected_tile = &(*it);

				// check if we want an adjacent tile due to our mouse positioning
				// ! We need to account for the affects of padding in the pane				
				getPixelPosFromTilePos(it->col, it->row, &px, &py);
				px = x - px - mid_pane.padding / 2;
				py = y - py - mid_pane.padding / 2;
				tilehex_pos = getTilehexDir(px, py);
				if(tilehex_pos != -1){
					getTilePosFromDirection(tilehex_pos, it->col, it->row, &tile_col, &tile_row);
					if(model.get_tile(tile_col, tile_row) == nullptr ||
						model.get_tile(tile_col, tile_row)->active == 0)
					{
						//if(tile_col < 0 || tile_row < 0 || tile_col >= model.m_board_width || tile_row >= model.m_board_height){
						// tile does not exist on the game board, therefore
						// clear the selection and stop looking
						selected_tile = nullptr;
						tile_col = -1;
						tile_row = -1;
						break;
					} else{
						selected_tile = &tiles[tile_col + tile_row*model.m_board_width];
					}
				}

				// set the selectred col and row
				tile_col = selected_tile->col;
				tile_row = selected_tile->row;

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

	} else if(bot_pane.hitbox.collides(mouse.hitbox)){
		// Handle bot-pane intersections
		// update the model state
		selected_pane = &bot_pane;
		x -= bot_pane.x;
		y -= bot_pane.y;
		std::vector<Button*>::iterator it;
		for(it = button_list.begin(); it != button_list.end(); ++it){
			(*it)->hit_flag = ((*it)->hitbox.collides(rel_mouse_hitbox));
		}
	} else{
		// No Pane was selected
		selected_pane = nullptr;
		selected_vertex = nullptr;
		selected_face = nullptr;
	}
}

// screen offset in pixels
void View_Game::render_model(pane_t& pane){
	int screen_offset_x = pane.x+ pane.padding/2;
	int screen_offset_y = pane.y+ pane.padding/2;
	memset(vertex_covered, 0, model.vertex_array.size()*sizeof(Uint8));
	memset(face_covered, 0, model.face_array.size()*sizeof(Uint8));

	int height_overlap = (int)(tile_h*0.75);
	int c_offset = (int)(tile_w/2 - 5);
	int r_offset = (int)(tile_h/2 - 5);
	SDL_Color black = { 0, 0, 0,255 };
	bool good_draw_tile = false;
	int type, roll;
	int target;
	int c, r;
	int i;

	// render all the game tiles
	for(int row = 0; row < model.m_board_height; ++row){
		for(int col = 0; col < model.m_board_width; ++col){			
			getPixelPosFromTilePos(col, row, &c, &r);
			r += screen_offset_y;
			c += screen_offset_x;
			//r = row*height_overlap + screen_offset_y;
			//c = col*tile_w + (row % 2 == 1)*(tile_w / 2) + screen_offset_x;
			type = model.get_type_from_tile(col, row);
			roll = model.get_roll_from_tile(col, row);

			if(type != 0){
				Util::render_texture(&ren, hextile_spritesheet, c, r, &hextile_clips[type]);
				Util::render_text(&ren, font_carbon_12, c + c_offset, r + r_offset, black, "%d", roll);
			} else{
				SDL_Rect clip = { tile_w * 4, tile_h * 2, tile_w, tile_h };
				Util::render_texture(&ren, hextile_spritesheet, c, r, &clip);
			}

			if(draw_tile){
				good_draw_tile = true;
			}
		}
	}

	// render all the settlements 
	for(int row = 0; row < model.m_board_height; ++row){
		for(int col = 0; col < model.m_board_width; ++col){
			r = row*height_overlap + screen_offset_y;
			c = col*tile_w + (row % 2 == 1)*(tile_w / 2) + screen_offset_x;
			if(model.get_type_from_tile(col, row) == 0){ continue; }

			// render all the vertices
			for(i = 0; i < 6; ++i){
				target = model.get_vertices_from_tile(col, row)[i];
				if(target == -1 || vertex_covered[target] == 1){
					continue;
				}

				vertex_covered[target] = 1;
				int type = model.get_type_from_vertex(target);
				//type = vertex_face_t::CITY;
				if(type == vertex_face_t::NONE){ continue; }				

				int x = c + vertex_pos[i][0] - sprite_small[0]/2;
				int y = r + vertex_pos[i][1] - sprite_small[1]/2;
				Util::render_texture(&ren, buildings_spritesheet, x, y, &building_clips[type]);
			}

		}
	}


	// render all the roads/faces
	for(int row = 0; row < model.m_board_height; ++row){
		for(int col = 0; col < model.m_board_width; ++col){
			r = row*height_overlap + screen_offset_y;
			c = col*tile_w + (row % 2 == 1)*(tile_w / 2) + screen_offset_x;
			if(model.get_type_from_tile(col, row) == 0){
				continue;
			};

			// render all the faces
			for(i = 0; i < 6; ++i){
				target = model.get_faces_from_tile(col, row)[i];
				if(target == -1 || face_covered[target] == 1){
					continue;
				}

				face_covered[target] = 1;
				type = model.get_type_from_face(target);
				// TODO: the types for this are wrong. it is currently set to 
				// types of vertex_face_t, but out clipping rects use building_t
				// enumerations to indetify the clip.
				//type = building_t::ROAD;
				if(type == vertex_face_t::NONE){ continue; }
				int x = c + face_pos[i][0];
				int y = r + face_pos[i][1];

				Util::render_texture(&ren, buildings_spritesheet, x, y, &road_clips[i]);
			}
		}
	}

	if(true){
		// draw all the intersect rectangles
		do{
			//break;
			std::vector<Tile_intersect>::iterator it;
			for(it = tiles.begin(); it != tiles.end(); ++it){
				SDL_Rect rect = { it->x + pane.x, it->y + pane.y, it->w, it->h };
				Util::render_rectangle(&ren, &rect, Util::colour_white());
			}
		} while(false);

		// render all the intersect vertices
		do{
			std::vector< vertex_face_t_intersect>::iterator it;
			for(it = vertices.begin(); it != vertices.end(); ++it){
				SDL_Rect rect = { it->x + pane.x, it->y + pane.y, it->w, it->h };
				Util::render_rectangle(&ren, &rect, Util::colour_white());
			}
		} while(false);
		// render all the intersect faces
		do{
			std::vector< vertex_face_t_intersect>::iterator it;
			for(it = faces.begin(); it != faces.end(); ++it){
				SDL_Rect rect = { it->x + pane.x, it->y + pane.y, it->w, it->h };
				Util::render_rectangle(&ren, &rect, Util::colour_white());
			}
		} while(false);
	}


	// highlight the target hex
	if(draw_tile && good_draw_tile && selected_tile != nullptr){
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
	Player* player = model.get_player(model.m_current_player);
	if(player == nullptr){ return; }

	// who is the current player
	off_x = 0;
	off_y = 3 * 30 + 3 * 5 + 5;
	rect = {0,pane.y + off_y, 80, 18 };
	Util::render_rectangle(&ren, &rect, player->color);
	Util::render_text(&ren, font_carbon_12, pane.x + off_x + rect.w +5,pane.y + off_y, font_carbon_12_colour,
		"Player %d : %s Victory Points=%d", model.m_current_player,player->name.c_str(), player->victory_points);

	// how many resources doe the player have?
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
	off_x = 0;
	off_y += line_spacing;
	Util::render_text(&ren, font_carbon_12, pane.x + off_x, pane.y + off_y, font_carbon_12_colour,
		"Longest road %d segments by player %d ",model.longest_road(),model.player_holding_longest_road_card);
	off_x = 0;
	off_y += line_spacing;
	Util::render_text(&ren, font_carbon_12, pane.x + off_x, pane.y + off_y, font_carbon_12_colour,
		"Largest army %d soldiers owned by player %d",model.largest_army(), model.player_holding_largest_army_card);
}

void View_Game::render(){
	++total_frame_count;

	SDL_RenderClear(&ren);
	render_model(mid_pane);
	render_buttons(bot_pane);
	// draw the UI text for stuff...
	render_bottom_text(bot_pane);

	// draw the mouse position
	int mouse_x, mouse_y;
	Uint32 buttons;
	buttons = SDL_GetMouseState(&mouse_x, &mouse_y);
	Util::render_text(&ren, font_carbon_12, top_pane.x, top_pane.y, font_carbon_12_colour,
		"Mouse(%d,%d,%d)", mouse_x, mouse_y, buttons);
	// draw the fps 
	Util::render_text(&ren, font_carbon_12, top_pane.x, top_pane.y + 18, font_carbon_12_colour,
		"FPS:%d.%d", fps.fps, total_frame_count);

	// draw a rectangle aroudn the selected pane
	if(selected_pane != nullptr  && true){
		SDL_Rect r = { selected_pane->x, selected_pane->y, selected_pane->w, selected_pane->h };
		SDL_Color c = { 255,255,0, 255 };
		Util::render_rectangle(&ren, &r,c);
	}

	SDL_RenderPresent(&ren);
}