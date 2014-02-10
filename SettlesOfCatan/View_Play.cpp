
#include <SDL.h>
#include "View_Play.h"
#include "IDialog.h"
#include "Timer.h"
#include "Util.h"
#include "Configuration.h"
#include "Logger.h"
#include "Collision.h"
#include "TextField.h"
#include "DropDown.h"


View_Play::View_Play(SDL_Window& win, SDL_Renderer& ren): IView(win,ren){
	srand((unsigned)(time(NULL)));
	disp_w = Configuration::DISP_W;
	disp_h = Configuration::DISP_H;
	fps_timer = TimerFactory::get().make(1000 /Configuration::FPS);
	fps_timer->stop();
	button = 0;
	mouse_x = 0;
	mouse_y = 0;

	mouse_intersect.hook(&mouse_x, &mouse_y);
	mouse_intersect.add_rect(0, 0, 0, 1, 1);
	// MY STUFF START
	int offsets2[8][2] = {
		{ 1, -2 }, { 2, -1 }, { 2, 1 }, { 1, 2 },
		{ -1, 2 }, { -2, 1 }, { -2, -1 }, { -1, -2 }
	};
	for(int i = 0; i < 8; ++i){
		offsets[i][0] = offsets2[i][0];
		offsets[i][1] = offsets2[i][1];
	}

	target = 0;
	target_x = 0;
	target_y = 0;
	board_size = 8;
	_board = new int[board_size*board_size];		

	graph_size = board_size*board_size;
	graph = new int[graph_size*graph_size];
	std::memset(_board, 0,board_size*board_size*sizeof(int));
	std::memset(graph, 0, graph_size*graph_size*sizeof(int));
	graph_that_shit();
//	solve();

	// MY STUFF END
	font_carbon_12 = TTF_OpenFont(Util::data_resource("carbon.ttf").c_str(), 12);
	font_carbon_12_colour = { 177, 177, 98, 255 };
	if(font_carbon_12 == nullptr){
		Logger::getLog().TTF_log(Logger::ERROR, "View_Game::load() TTF_OpenFont data/carbon.ttf ");
	}
}

View_Play::~View_Play(){
	Logger::getLog().log(Logger::DEBUG, "View_Play destructor()");
	delete fps_timer;
	SDL_FreeSurface(surface);
	SDL_DestroyTexture(texture);
	TTF_CloseFont(font_carbon_12);
	delete[] _board;
	delete[] graph;
}

void View_Play::on_start(SDL_Event& ev){
	fps_timer->start();	
}

void View_Play::on_switch(SDL_Event& ev){
	fps_timer->stop();
}

void View_Play::on_close(SDL_Event& ev){
	fps_timer->stop();
}

void View_Play::handle_keyboard_events(SDL_Event& ev){
	
	const Uint8 *keyboard = SDL_GetKeyboardState(NULL);
	if(ev.type == SDL_KEYDOWN){
		if(keyboard[SDL_SCANCODE_ESCAPE]){
			SDL_Event ev;
			SDL_zero(ev);
			ev.type = SDL_QUIT;
			ev.quit.type = SDL_QUIT;
			ev.quit.timestamp = SDL_GetTicks();
			SDL_PushEvent(&ev);
			return;
		}

		// MY STUFF START				
		if(keyboard[SDL_SCANCODE_1]){
			int dir = (keyboard[SDL_SCANCODE_LSHIFT]) ? -1 : 1;
			target += dir;
		}

		if( keyboard[SDL_SCANCODE_W] ){ target_y--;}
		if( keyboard[SDL_SCANCODE_D] ){ target_x++;}
		if( keyboard[SDL_SCANCODE_S] ){ target_y++;}
		if( keyboard[SDL_SCANCODE_A] ){ target_x--;}
		if(target_x < 0){ target_x = 0; }
		if(target_x >= board_size){ target_x = board_size-1; }
		if(target_y < 0){ target_y = 0; }
		if(target_y >= board_size){ target_y = board_size - 1; }
		// MY STUFF END

	} else if(ev.type == SDL_KEYUP){

	}
}
void View_Play::handle_mouse_events(SDL_Event& ev){

	button = SDL_GetMouseState(&mouse_x,&mouse_y);
	if(ev.type == SDL_MOUSEBUTTONDOWN){

	} else if(ev.type == SDL_MOUSEBUTTONUP){

	} else if(ev.type == SDL_MOUSEMOTION){

	}
}

void View_Play::handle_user_events(SDL_Event& ev){
	if(ev.type >= SDL_USEREVENT){
		// handle a frame per 1/30 the of a second
		if(ev.user.type == TimerFactory::get().event_type()){
			draw_flag = true;
		} else if(ev.user.type == Util::get().get_userev("dialog_close_event")){
			
		} else{
		}
	}
}

void View_Play::update(SDL_Event& ev){
	// do something for every user event
}


int View_Play::bvalue(int* board, int n, int col, int row){
	if(col < 0 || col >= n || row < 0 || row >= n){ return -1; }
	return board[col + row*n];
}
bool View_Play::_solve(int* board, int n, int col, int row, int* count){
	static unsigned long long counter = 0;	
	printf("%llu             \r", counter++);

	if(*count >= n*n){ return true; }
	board[col + row*n] = *count +1;
	*count = *count +1;

	int c = 0;
	int r = 0;
	int adjacent = 0;
	for(int i = 0; i < 8; ++i){
		c = col + offsets[i][0];
		r = col + offsets[i][1];
		if(bvalue(board, n, c, r) != 0) { continue; }
		if(_solve(board, n,c,r, count)){
			// we done, we have found a valid solution
			return true;
		}		
	}
	
	// no valid transition was found from this position
	board[col + row*n] = 0;
	*count = *count - 1;
	return false;
}
void View_Play::solve(){
	Uint32 start = SDL_GetTicks();
	int count = 0;
	for(int row = 0; row < board_size; ++row){
		for(int col = 0; col < board_size; ++col){
			std::memset(_board, 0, sizeof(int)*board_size*board_size);
			count = 0;

			// if returns true then we have a valid solution
			if(_solve(_board, board_size, col, row, &count)){
				goto out_of_loop;
			}
		}
	}	

out_of_loop:	
	printf("solve time taken = %0.5f", ((float)(SDL_GetTicks()) / start) /1000);
	return;
}
void View_Play::graph_that_shit(){	
	int c = 0;
	int r = 0;
	int graph_c = 0;
	int graph_r = 0;
	for(int row = 0; row < board_size; ++row){
		for(int col = 0; col < board_size; ++col){
			graph_c = col + row*board_size;
			
			for(int i = 0; i < 8; ++i){
				c = col + offsets[i][0];
				r = row + offsets[i][1];
				if(c < 0 || c >= board_size || r < 0 || r >= board_size){ continue; }	

				graph_r = c + r*board_size;
				graph[graph_c + graph_r*graph_size] = 1;
				graph[graph_r + graph_c*graph_size] = 1;
			}
		}
	}

}

void View_Play::render(){		
	SDL_RenderClear(&ren);
	
	Util::render_text(&ren, font_carbon_12, 5, 5, font_carbon_12_colour,
		"board size = %dx%d    target=%d",board_size, board_size, target);

	SDL_Rect rect = { 0, 0, 0, 0 };
	SDL_Color color = { 90, 120, 30, 180 };
	SDL_Color color2 = { 20, 120, 140, 180 };
	int start_y = 40;
	int padding = 0;
	int x_offset = padding;	
	int y_offset = start_y;
	int tile_w = 40;
	int tile_h = 40;

	// render special rectangles around connected tiles
	int c = 0;
	int r = 0;
	int graph_c =  target_x  + target_y*board_size;
	int graph_r = 0;

	/*
	for(int i = 0; i < 8; ++i){
		c = target_x + offsets[i][0];
		r = target_y + offsets[i][1];

		if(c < 0 || c >= board_size || r < 0 || r >= board_size){ continue; }
		graph_r = c + r*board_size;
		if(graph[graph_c + graph_r*graph_size] != 1){ continue; }
		if(graph[graph_r + graph_c*graph_size] != 1){ continue; }

		rect = {
			c*(tile_w + 2),
			r*(tile_h + 2) + start_y,
			tile_w,
			tile_h
		};
		Util::render_fill_rectangle(&ren, &rect, color);
	}
	*/
	
	for(int row = 0; row < board_size; ++row){
		for(int col = 0; col < board_size; ++col){
			rect = { x_offset, y_offset, tile_w, tile_h };
			Util::render_rectangle(&ren, &rect, color);

			/*
			if(target_x == col &&  target_y == row){
				Util::render_fill_rectangle(&ren, &rect, color2);
			}
			*/
			graph_c = col + row*board_size;
			for(int i = 0; i < 8; ++i){
				c = col + offsets[i][0];
				r = row + offsets[i][1];
				if(c < 0 || c >= board_size || r < 0 || r >= board_size){ continue; }
				graph_r = c + r*board_size;

				int end_x = c*(tile_w + padding) + tile_w / 2;
				int end_y = r*(tile_h + padding) + tile_h / 2 + start_y;
				Util::render_line(&ren, color2,
					rect.x + rect.w / 2,
					rect.y + rect.h / 2 ,
					end_x,
					end_y);				
			}


			x_offset += tile_w + padding;
		}
		x_offset = padding;
		y_offset += tile_h + padding;
	}

	
	SDL_RenderPresent(&ren);
}

