/*

HOME TO OLD CODE FORM A VARIETY OF CLASSES
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>

#include "Logger.h"
#include "util.h"
#include "Timer.h"
#include "Button.h"
#include "SpriteFactory.h"
#include "Sprite.h"
#include "Model.h"

typedef struct bag_t {
	unsigned w;
	unsigned h;
	unsigned tilesize;
	bool exit_flag;
}bag_t;



bool init();
void shutdown();
bool intersect(SDL_Rect* A, SDL_Rect* B, SDL_Rect* rs);
void render_model(Model& model, SDL_Renderer* ren, SDL_Texture* hex, SDL_Rect* clips, TTF_Font* font_12);

int main(int argc, char** argv){
	if(init() == false){ return EXIT_FAILURE; }

	//variables
	Logger& logger = Logger::getLog("jordan.log");
	SDL_Window* win = 0;
	SDL_Renderer* ren = 0;
	SDL_Texture* background = 0;
	SDL_Texture* foreground = 0;
	SDL_Texture* hex = 0;
	TTF_Font* font_12 = 0;
	Timer* fps_timer = 0;
	Button* button = 0;
	Ball* ball = 0;
	Box* box1 = 0;
	Box* box2 = 0;
	// Load in a Model
	Model model(4);

	Mix_Music* music = 0;
	Mix_Chunk* sound1 = 0;
	Mix_Chunk* sound2 = 0;
	bag_t bag = {UTIL_DISP_W,UTIL_DISP_H,32,false};
	int box_target = 0;
	struct fps_t fps = {UTIL_FPS,0,0,0};

	// Create a new window
	win = SDL_CreateWindow("Jordan", 100, 100,bag.w,bag.h,SDL_WINDOW_SHOWN);
	if (win == nullptr){ 
		logger.SDL_log(logger.ERROR,"SDL_CreateWindow");
		return 2;
	}

	// Create a Renderer
	ren = SDL_CreateRenderer(win, -1,
		SDL_RENDERER_ACCELERATED |
		SDL_RENDERER_PRESENTVSYNC);
	if (ren == nullptr) { 
		logger.SDL_log(logger.ERROR,"SDL_CreateRenderer");
		return 3;
	}

	//Loading Textures for display
	foreground = load_texture("data/blue_tile.png", ren);
	background = load_texture("data/red_tile.png", ren);
	hex = load_texture("data/hextiles_spritesheet.png", ren);
	if (foreground == 0 || background == 0){
		return 4;
	}
	// Loading a font
	font_12 = TTF_OpenFont("data/carbon.ttf", 12);
	if(font_12 == nullptr){
		logger.SDL_log(logger.ERROR, "TTF_OpenFont");
		return 4;
	}

	// Register timers and stuff
	fps_timer = TimerFactory::get().make(1000/UTIL_FPS);

	// Create a button to display
	button = new Button(ren, 50, 50, 64, 48);
	ball = (Ball*)SpriteFactory::get(ren).make("ball");
	box1 = (Box*)SpriteFactory::get(ren).make("box");
	box2 = (Box*)SpriteFactory::get(ren).make("box");

	// Load the music
	music = Mix_LoadMUS("data/music.mp3");
	if(music == 0){
		logger.Mix_log(logger.ERROR, "Mix_LoadMUS");
		return 6;
	}
	// Load the sound effects
	sound1 = Mix_LoadWAV("data/sound1.wav");
	sound2 = Mix_LoadWAV("data/sound2.wav");
	if(sound1 == nullptr ){
		logger.Mix_log(logger.ERROR, "Mix_LoadWAV(sound1)");
		return 6;
	}
	if(sound2 == nullptr){
		logger.Mix_log(logger.ERROR, "Mix_LoadWAV(sound2)");
		return 6;
	}


	// Main loop
	SDL_Event e;
	unsigned long count = 0;
	bool draw_flag = false;
	int draw_times = 0;
	fps = { 0, 0, 0 };
	SDL_Rect clips[Tiles::NUM_OF_TILES];
	memset(clips, 0, sizeof(SDL_Rect)*Tiles::NUM_OF_TILES);
	int tilesize = 40;
	int x = 0, y = 0;
	clips[Tiles::SHEEP_TILE] = { 0, 0, tilesize, tilesize };
	clips[Tiles::BRICK_TILE] = { tilesize, 0, tilesize, tilesize };
	clips[Tiles::WOOD_TILE] = { tilesize * 2, 0, tilesize, tilesize };
	clips[Tiles::WHEAT_TILE] = { tilesize * 3, 0, tilesize, tilesize };
	clips[Tiles::ORE_TILE] = { tilesize * 4, 0, tilesize, tilesize };
	clips[Tiles::SHEEP_PORT] = { 0, tilesize, tilesize, tilesize };
	clips[Tiles::BRICK_PORT] = { tilesize, tilesize, tilesize, tilesize };
	clips[Tiles::WOOD_PORT] = { tilesize * 2, tilesize, tilesize, tilesize };
	clips[Tiles::WHEAT_PORT] = { tilesize * 3, tilesize, tilesize, tilesize };
	clips[Tiles::ORE_PORT] = { tilesize * 4, tilesize, tilesize, tilesize };
	clips[Tiles::TRADE_PORTS] = { 0, tilesize * 2, tilesize, tilesize };
	clips[Tiles::DESERT_TILE] = { tilesize * 1, tilesize * 2, tilesize, tilesize };
	clips[Tiles::WATER_TILE] = { tilesize * 2, tilesize * 2, tilesize, tilesize };

	while(!bag.exit_flag){
		if(SDL_WaitEvent(&e) == 0){
			logger.log(logger.DEBUG, "SDL_WaitEvent");
		} else{ // HANDLE EVENTS
			// Quit Event
			if(e.type == SDL_QUIT){
				bag.exit_flag = true;
			}
			// Handle Keyboard Events
			if(e.type == SDL_KEYDOWN){
				if(e.key.keysym.scancode == SDL_SCANCODE_ESCAPE){
					bag.exit_flag = true;
				}
				// ball->update(&e);
				if(box_target == 0){
					box1->update(&e);
				} else if(box_target == 1){
					box2->update(&e);
				}

				// Handle the playing the effects
				if(e.key.keysym.scancode == SDL_SCANCODE_1){
					if(Mix_PlayChannel(-1, sound1, 0) == -1){
						logger.Mix_log(logger.ERROR, "Mix_PlayChannel(sound1)");
					}
				}
				if(e.key.keysym.scancode == SDL_SCANCODE_2){
					if(Mix_PlayChannel(-1, sound2, 0) == -1){
						logger.Mix_log(logger.ERROR, "Mix_PlayChannel(sound2)");
					}
				}

				// Handle playing, pauseing and stopping the music
				if(e.key.keysym.scancode == SDL_SCANCODE_R){
					if(Mix_PlayingMusic() == 0){
						// NOT CURRENTLY PLAYING
						if(Mix_PlayMusic(music, -1) == -1){
							logger.Mix_log(logger.ERROR, "Mix_PlayMusic");
						}
					} else{
						// PLAYING
						if(Mix_PausedMusic() == 1){
							// Resume the music
							Mix_ResumeMusic();
						} else{
							// Pause the music
							Mix_PauseMusic();
						}
					}
				}
				// Stop the music
				if(e.key.keysym.scancode == SDL_SCANCODE_T){
					Mix_HaltMusic();
				}

			} else if(e.type == SDL_KEYUP){

			}
			// Handle Mouse Events
			if(e.type == SDL_MOUSEBUTTONDOWN ||
				e.type == SDL_MOUSEBUTTONUP ||
				e.type == SDL_MOUSEMOTION)
			{
				button->handle_event(&e);
				if(e.type == SDL_MOUSEBUTTONDOWN){
					if(e.button.button ==SDL_BUTTON_LEFT){
						box_target = (box_target + 1) % 2;
					}
				}
				if(e.type == SDL_MOUSEMOTION){
					if(box_target == 0){
						box1->update(&e);
					} else {
						box2->update(&e);
					}
				}
			}

			// Handle User Events
			if(e.type == SDL_USEREVENT){
				// Check for timer events
				if(e.user.type == TimerFactory::get().event_type()){
					draw_flag = true;
					ball->tick();
					fps.update();
				}
			}
		}

		// render the scene
		if(draw_flag){
			draw_flag = false;
			++count;
			SDL_RenderClear(ren);

			// render all the background tiles
			SDL_RenderClear(ren);
			for(unsigned row = 0; row < bag.h; ++row){
				for(unsigned col = 0; col < bag.w; ++col){
					int c = col*bag.tilesize;
					int r = row*bag.tilesize;
					render_texture(ren, background, c, r);
				}
			}

			// draw the objects
			render_texture(ren, foreground, 25, 25);
			button->draw(ren);
			ball->draw(ren);

			// Draw boxes and any potential intersection rectangle
			bool a_intersect_b = false;
			do{
				SDL_Rect rs;
				SDL_Rect A = { box1->x, box1->y, box1->hitbox.w, box1->hitbox.h };
				SDL_Rect B = { box2->x, box2->y, box2->hitbox.w, box2->hitbox.h };
				box1->draw(ren, 255, 0, 0);
				box2->draw(ren, 0, 255, 0);
				if((a_intersect_b = intersect(&A, &B, &rs))){
					Uint8 r, g, b, a;
					SDL_GetRenderDrawColor(ren, &r, &g, &b, &a);

					SDL_SetRenderDrawColor(ren, 0, 0, 255, 255);
					SDL_RenderDrawRect(ren, &rs);
					SDL_SetRenderDrawColor(ren, r, g, b, a);
				}
			} while(0);

			render_model(model,ren,hex,clips,font_12);
			
			// draw the text
			SDL_Color color = { 177, 177, 98 };
			// mouse position
			int mouse_x, mouse_y;
			Uint32 buttons;
			buttons = SDL_GetMouseState(&mouse_x, &mouse_y);
			render_text(ren, font_12, 0, 0, color, "Mouse(%d.%d.%d)",mouse_x,mouse_y,buttons);
			// Frame count, and FPS
			render_text(ren, font_12, 0, 18, color, "FPS:%d,%d",fps.fps,count);
			
			// Render to the window
			SDL_RenderPresent(ren);
		}
	}

	// Cleanup
	delete fps_timer;
	delete button;
	delete ball;
	delete box1;
	delete box2;
	Mix_FreeChunk(sound1);
	Mix_FreeChunk(sound2);
	Mix_FreeMusic(music);
	SDL_DestroyTexture(foreground);
	SDL_DestroyTexture(background);
	SDL_DestroyTexture(hex);
	TTF_CloseFont(font_12);
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);
	
	// close SDL and stuff
	shutdown();
	return 0;
}


bool init(){
	Logger& logger = Logger::getLog("jordan.log");

	// initialize SDL for using modules
	if(SDL_Init(SDL_INIT_EVERYTHING)){
		logger.SDL_log(logger.ERROR, "SDL_INIT_EVERYTHING");
		return false;
	}
	// initialize PNG SDL_Image
	if((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)  != IMG_INIT_PNG){
		logger.IMG_log(logger.ERROR, "IMG_INIT(IMG_INIT_PNG)");
		return false;
	}
	// initialize JPG SDL_Image
	if( (IMG_Init(IMG_INIT_JPG) & IMG_INIT_JPG) != IMG_INIT_JPG){
		logger.IMG_log(logger.ERROR, "IMG_INIT(IMG_INIT_JPG)");
		return false;
	}
	// Initialize the TTF font module
	if(TTF_Init() != 0){
		logger.TTF_log(logger.ERROR, "TTF_Init");
		return false;
	}
	// Initialize the Mixer for MP3 usage
	int flags =MIX_INIT_MP3;
	if((Mix_Init(flags) & flags) != flags){
		logger.Mix_log(logger.ERROR, "MIX_INIT_MP3");
		return false;
	}
	if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096) == -1){
		logger.Mix_log(logger.ERROR, "Mix_OpenAudio");
		return false;
	}
	return true;
}

void shutdown(){
	Mix_CloseAudio();
	Mix_Quit();
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

bool intersect(SDL_Rect* A, SDL_Rect* B, SDL_Rect* result){
	// Invalid params
	if(A == nullptr || B == nullptr){ return false;}
	bool x_flag=true, y_flag=true;

	// Check for horizontal intersection
	if(A->x + A->w < B->x || A->x > B->x + B->w){
		x_flag = false;
	}
	// Check for vertical interseiction
	if(A->y + A->h <  B->y || A->y > B->y + B->h){
		y_flag = false;
	}

	// There is only an intersection if both horizontal and vertical intersect
	if(x_flag && y_flag){
		result->x = (B->x > A->x) ? B->x : A->x;
		result->y = (B->y > A->y) ? B->y : A->y;
		result->w = (B->x + B->w < A->x + A->w) ? B->x + B->w : A->x + A->w;
		result->w -= result->x;
		result->h = (B->y + B->h < A->y + A->h) ? B->y + B->h : A->y+ A->h;
		result->h -= result->y;
	}

	return (x_flag && y_flag);
}


void render_model(Model& model,SDL_Renderer* ren, SDL_Texture* hex, SDL_Rect* clips,TTF_Font* font_12){
	if(1){
		int tilesize = 40;
		int height_overlap = (int)(tilesize*0.75);
		int c_offset = (int)(tilesize*0.4);
		int r_offset = (int)(tilesize*0.35);
		SDL_Color color = { 255, 255, 255 };
		int c, r;
		int type, roll;

		for(int row = 0; row < model.m_board_width; ++row){
			for(int col = 0; col < model.m_board_width; ++col){
				r = row*height_overlap;
				c = col*tilesize + (row % 2 == 1)*(tilesize / 2);
				type = model.m_board[col + row*model.m_board_width].type;
				roll = model.m_board[col + row*model.m_board_width].roll;

				if(type == 0){
					// don't render the tile
					//render_texture(ren, background, c, r);
				} else{
					render_texture(ren, hex, c, r, &clips[type]);
					//render_text(ren, font_12, c + c_offset, r + r_offset, color, "%d",roll);
				}
			}
		}
	}
}









Tiles* Model::find_tile_from_vertex(int vertex, int* c, int* r){
for(int row = 0; row < m_board_width; ++row){
for(int col = 0; col < m_board_width; ++col){
Tiles* hex = &m_board[col + row*m_board_width];
if(hex->active == 0) { continue; }

for(int i = 0; i < 6; ++i){
if(i == hex->vertices[i]){
if(c != nullptr){ *c = col; }
if(r != nullptr){ *r = row; }
return hex;
}
}
}
}
return nullptr;
}
Tiles* Model::find_tile_from_face(int face, int* c, int* r){
for(int row = 0; row < m_board_width; ++row){
for(int col = 0; col < m_board_width; ++col){
Tiles* hex = &m_board[col + row*m_board_width];
if(hex->active == 0) { continue; }

for(int i = 0; i < 6; ++i){
if(i == hex->faces[i]){
if(c != nullptr){ *c = col; }
if(r != nullptr){ *r = row; }
return hex;
}
}
}
}
return nullptr;
}


bool Model::can_build_road(int player,int row, int col, int pos){
Tiles* tile = &m_board[col + row*m_board_width];
int x, y;
bool road_exists = false;
bool blocked = false;
Tiles* before_tile = nullptr;
Tiles* after_tile = nullptr;
// get the before tile
tile->get_adjacent((pos == 0) ? 5 : pos - 1, col, row, &x, &y);
if(x < 0 || y <0 || x >= m_board_width || y >= m_board_width){
before_tile = nullptr;
} else{
before_tile = &m_board[x + y*m_board_width];
}

// get the after tile
tile->get_adjacent((pos + 1) % 6, col, row, &x, &y);
if(x < 0 || y <0 || x >= m_board_width || y >= m_board_width){
after_tile = nullptr;
} else{
after_tile = &m_board[x + y*m_board_width];
}

// check the faces in the before_tiles,and after tiles for a road owned by the player
road_exists = false;
int check_indicies[4] = { 1, 2, 4, 5 };
Tiles*  tile_to_use[6] = { before_tile, before_tile, after_tile, after_tile, 0, 0 };
for(int i = 0; i < 4; ++i){
if(tile_to_use[i] == nullptr){ continue; }
int t = face_array[tile_to_use[i]->faces[(pos + check_indicies[i]) % 6]].type;
int p = face_array[tile_to_use[i]->faces[(pos + check_indicies[i]) % 6]].player;
if(t == vertex_face_t::ROAD && p == player){
road_exists = true;
break;
}
}

// check that a city does not exist at a vertex point between
tile_to_use[0] = before_tile;
tile_to_use[1] = before_tile;
tile_to_use[2] = before_tile;
tile_to_use[3] = after_tile;
tile_to_use[4] = after_tile;
tile_to_use[5] = after_tile;
blocked = false;
for(int i = 0; i < 6; ++i){
if(tile_to_use[i] == nullptr){ continue; }
int t = vertex_array[tile_to_use[i]->vertices[(pos + i) % 6]].type;
int p = vertex_array[tile_to_use[i]->vertices[(pos + i) % 6]].player;

if((t == vertex_face_t::CITY || t == vertex_face_t::SETTLEMENT) &&
p != player)
{
blocked = true;
break;
}
}

return (road_exists && blocked == false);
}
bool Model::can_build_settlement(int player, int row, int col, int v){
int x, y;
bool has_space = false;
bool is_connected = false;
Tiles* tile = &m_board[col + row*m_board_width];
Tiles* tile1 = nullptr;

// get tile 1
tile->get_adjacent(v % 6, col, row, &x, &y);
if(x < 0 || y < 0 || x >= m_board_width || y >= m_board_width){
tile1 = nullptr;
} else{
tile1 = &m_board[x + y*m_board_width];
}

// check for space to put the city
has_space = true;
int offset[3] = { 0, 5, 5 };
Tiles* target[3] = { tile, tile, tile1 };
for(int i = 0; i < 3; ++i){
if(target[i] == nullptr){ continue; }
// city or settlement doesn't exist
int t = vertex_array[target[i]->vertices[(v + offset[i])%6]].type;
if(t == vertex_face_t::CITY || t == vertex_face_t::SETTLEMENT){
has_space = false;
return false;
break;
}
}

// look for a connected road to the vertex
// this algorithm assumes that a road could only exist in that
// position if it is connected by another road.
// We are ignoring the case where the road happens to be broken by
// another player's settelement
is_connected = false;
offset[0] = 0;
offset[1] = 5;
offset[2] = 4;
for(int i = 0; i < 3; ++i){
if(target[i] == nullptr){ continue; }
// check for a road
int t = vertex_array[target[i]->faces[(v + offset[i]) % 6]].type;
int p = vertex_array[target[i]->faces[(v + offset[i]) % 6]].player;

if(t == vertex_face_t::ROAD && p == player){
is_connected = true;
break;
}
}

return (has_space && is_connected);
}


bool View_Game::run_wait(){
Logger& logger = Logger::getLog("jordan.log");
SDL_Event e;

fps_timer->start();
draw_flag = false;
while(exit_flag == false){
// wait for an event
if(SDL_WaitEvent(&e)==0){
logger.SDL_log(Logger::ERROR, "SDL_WaitEvent");
continue;
}

if(e.type == SDL_QUIT){
exit_flag = true;
} else if(is_keyboard_event(&e)){
handle_keyboard_events(&e);
} else if(is_mouse_event(&e)){
handle_mouse_events(&e);
} else if(e.type == SDL_USEREVENT){
handle_user_events(&e);
update();
}

if(draw_flag){
draw_flag = false;
++total_frame_count;
render();
}
}
return false;
}







vertex_pos[0][0] = tile_w/4;
vertex_pos[0][1] = -tile_h/4;
vertex_pos[1][0] = (int)(tile_w*0.75);
vertex_pos[1][1] = 0;
vertex_pos[2][0] = (int)(tile_w*0.75);
vertex_pos[2][1] = tile_h / 2;
vertex_pos[3][0] = tile_w/4;
vertex_pos[3][1] = (int)(tile_h*0.75);
vertex_pos[4][0] = -tile_w/4;
vertex_pos[4][1] = tile_h/2;
vertex_pos[5][0] = -tile_w/4;
vertex_pos[5][1] = 0;




*/