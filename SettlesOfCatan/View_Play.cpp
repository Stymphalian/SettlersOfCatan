
#include <algorithm>
#include <cassert>

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <string>
#include <list>
#include <iostream>
#include <random>

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
#include "Coords.h"
#include "mouse.h"
#include "Pane.h"

#include "ViewPort.h"
#include "Selectable.h"
#include "MouseListener.h"

SelectableViewport::SelectableViewport(Viewport* port)
: SelectablePane()
{
	this->port = port;
	_selected = false;
}
SelectableViewport::~SelectableViewport(){}


Book::Book(int flag)
: SelectablePane(), Mouseable(), IMouseListener(),
viewport(&_viewport)
//viewport(&scrollbar)
{
	focused_colour = { 20, 20, 20, 120 };
	selected_colour = { 80, 20, 10, 140 };
	selected_shit = nullptr;
	sub_book = nullptr;
	if(flag == 0){
		num_panes = 5;
		pages[0].coord().init(0, 0, 0, 30, 30);
		pages[1].coord().init(65, 0, 1, 40, 40);
		pages[2].coord().init(51, 123, 2, 50, 60);
		pages[3].coord().init(72, 70, 3, 70, 70);
		pages[4].coord().init(120, 128, 4, 80, 80);

		pages[0].coord().set_relative_x(pages[0].coord().w());
		pages[1].coord().set_relative_y(pages[1].coord().h());

		pages[0].setvisible(true);
		pages[1].setvisible(true);
		pages[2].setvisible(true);
		pages[3].setvisible(true);
		pages[4].setvisible(true);
		
		add_pane(&pages[1]);
		add_pane(&pages[3]);		
		add_pane(&pages[4]);
		add_pane(&pages[0]);
		add_pane(&pages[2]);
				
		pages[0].subscribe(this);
		pages[1].subscribe(this);
		pages[2].subscribe(this);
		pages[3].subscribe(this);
		pages[4].subscribe(this);

		sub_book = new Book(1);
		//sub_book->coord().init(0,0, 0, 400, 400);		
		sub_book->coord().init(150,150, 0, 200, 200);		
		sub_book->coord().set_relative_w(0.5f);
		sub_book->coord().set_relative_h(0.5f);
		sub_book->setvisible(true);		
		sub_book->subscribe(this);
		add_pane(sub_book);
		
		//_viewport.set_target(sub_book, 1, 1);
		//_viewport.set_camera_coords(0, 0, 100, 100);
		//_viewport.set_viewport_coords(25, 25,100,100);
		//_viewport.coord().set_relative_w(1.0f);
		//_viewport.coord().set_relative_h(1.0f);		
		//_viewport.viewport_coord().set_relative_w(1.0f);
		//_viewport.viewport_coord().set_relative_h(1.0f);		
		//_viewport.coord().set_parent(&this->coord());
		//_viewport.viewport_coord().set_parent(&this->coord());
		//_viewport.setvisible(true);
		//viewport.coord().init(25,25,0, 90, 90);
		//viewport.setvisible(true);
		//viewport.subscribe(this);
		//add_pane(&viewport);

		scrollbar.set_target(sub_book, 1, 1);
		scrollbar.set_camera_coords(0, 0, 100, 100);
		scrollbar.set_viewport_coords(25, 25, 100, 100);
		//vscrollbar.coord().set_relative_w(1.0f);
		//vscrollbar.coord().set_relative_h(1.0f);
		//vscrollbar.viewport_coord().set_relative_w(1.0f);
		//vscrollbar.viewport_coord().set_relative_h(1.0f);
		scrollbar.coord().set_parent(&this->coord());
		scrollbar.viewport_coord().set_parent(&this->coord());		
		//scrollbar.add_vert_scrollbar();
		//scrollbar.add_horiz_scrollbar();		
		scrollbar.show_vertical_scrollbar();
		//scrollbar.hide_vertical_scrollbar();
		scrollbar.show_horizontal_scrollbar();
		//scrollbar.hide_horizontal_scrollbar();
		scrollbar.setvisible(true);

		viewport.port = &scrollbar;
		viewport.coord().init(25,25,0, 100, 100);
		viewport.setvisible(true);
		viewport.subscribe(this);
		add_pane(&viewport);

	} else if (flag == 1){		
		num_panes = 1;		
		pages[0].coord().init(0, 0, 0, 50, 50);
		pages[0].coord().set_relative_w(0.1f);
		pages[0].coord().set_relative_h(0.1f);		
		pages[0].setvisible(true);

		add_pane(&pages[0]);
		pages[0].subscribe(this);		
		
		sub_book = new Book(2);
		sub_book->coord().init(30, 30,0,0,0);
		sub_book->coord().set_relative_w(0.25f);
		sub_book->coord().set_relative_h(0.255f);
		sub_book->setvisible(true);
		add_pane(sub_book);
		sub_book->subscribe(this);		
		
		_viewport.set_target(sub_book, 1, 1);
		_viewport.set_camera_coords(0, 0, 30, 30);
		_viewport.set_viewport_coords(5, 55, 30, 30);		
		_viewport.setvisible(true);
		_viewport.coord().set_parent(&this->coord());
		_viewport.viewport_coord().set_parent(&this->coord());

		viewport.coord().init(5,55,0,30,30);
		viewport.setvisible(true);
		viewport.subscribe(this);
		add_pane(&viewport);
		
	} else{
		num_panes = 0;
		pages[0].coord().init(0, 0, 0, 5, 5);
		pages[0].coord().set_relative_w(0.25f);
		pages[0].coord().set_relative_h(0.25f);
		pages[0].setvisible(true);

		add_pane(&pages[0]);
		pages[0].subscribe(this);		
	}
}
Book::~Book(){
	delete sub_book;
}

bool Book::keyboard_keydown(SDL_Event& ev){
	const Uint8* keyboard = SDL_GetKeyboardState(NULL);
	int dir = (keyboard[SDL_SCANCODE_LSHIFT]) ? -1 : 1;
	if(keyboard[SDL_SCANCODE_1]){
		coord().w(coord().w() + dir);
	}
	if(keyboard[SDL_SCANCODE_2]){
		coord().h(coord().h() + dir);
	}

	IPane* focused = get_focused_child_pane();
	focused->keyboard_keydown(ev);
	return true;
}
bool Book::keyboard_keyup(SDL_Event& ev){ return false; }

bool Book::mouse_buttondown(SDL_Event& ev, Coords* ref){
	if(has_focus()){
		if(is_selected() == false){
			set_selected(true);
		}				
		// pass the mouse button down to the focused pane.
		IPane* focused = get_focused_child_pane();
		if(ref != nullptr){
			Coords new_reference;
			new_reference.x(ref->x() - focused->coord().x());
			new_reference.y(ref->y() - focused->coord().y());
			focused->mouse_buttondown(ev,&new_reference);
		} else{
			focused->mouse_buttondown(ev);
		}		
		
		// handle our own widgets and stuff...
	}		
	notify_mouse_buttondown(1,this);
	return true;
}
bool Book::mouse_buttonup(SDL_Event& ev, Coords* ref){
	// deselect the book?
	if(is_selected()){
		set_selected(false);
	}
	if(has_focus()){
		get_focused_child_pane()->mouse_buttonup(ev, ref);		

		if(get_focused_child_pane() != selected_shit){
			if(selected_shit != nullptr){
				if(ref != nullptr){
					Coords new_ref;
					new_ref.x(ref->x() - selected_shit->coord().x());
					new_ref.y(ref->y() - selected_shit->coord().y());
					selected_shit->mouse_buttonup(ev,&new_ref);
				} else{
					selected_shit->mouse_buttonup(ev);
				}
				
			}
		}
	}	
	notify_mouse_buttonup(1,this);
	return true;
}

bool Book::mouse_motion(SDL_Event& ev, Coords* reference)
{	
	IMouse& rel_mouse = GMouse::get(&coord());
	rel_mouse.update();
	if(reference != nullptr){
		rel_mouse.x(reference->x());
		rel_mouse.y(reference->y());
	}
	if(has_focus()){
		determine_focused_child_pane(&rel_mouse);
	}

	// should we pass the event to the child, or should I consume it.
	IPane* focused = get_focused_child_pane();
	if(!NULL_Pane::isDummy(focused)){
		if(reference != nullptr){
			Coords next_reference;
			next_reference.x(reference->x() - focused->coord().x());
			next_reference.y(reference->y() - focused->coord().y());
			focused->mouse_motion(ev, &next_reference);
		} else{
			focused->mouse_motion(ev);
		}
		
	} else{
		// do nothing
	}
	return true;	
}
bool Book::mouse_drag(SDL_Event& ev, Coords* reference){
	IMouse& rel_mouse = GMouse::get(&coord());
	rel_mouse.update();
	if(reference != nullptr){
		rel_mouse.x(reference->x());
		rel_mouse.y(reference->y());
	}
	
	if(has_focus()){
		determine_focused_child_pane(&rel_mouse);		
	}

	if(selected_shit != nullptr)
	{	
		assert(selected_shit->is_selected());
		if(reference != nullptr){
			Coords next_reference;
			next_reference.x(reference->x() - selected_shit->coord().x());
			next_reference.y(reference->y() - selected_shit->coord().y());
			selected_shit->mouse_drag(ev, &next_reference);
		} else{
			selected_shit->mouse_drag(ev);
		}
		
		selected_shit->coord().make_within_bounds(&this->coord());
	}else{
		if(is_selected()){
			// handle the event ourselves
			coord().x(coord().x() + ev.motion.xrel);
			coord().y(coord().y() + ev.motion.yrel);			
			coord().make_within_bounds(coord().get_parent());
		}
	}	
	return true;
}

void Book::tick(){}
void Book::update(SDL_Event& ev){}
void Book::render(SDL_Renderer& ren){
	SDL_Rect rect = {
		0,0,
		coord().w(),
		coord().h()
	};
	render(ren, coord().disp_x(),coord().disp_y(), &rect);
}
void Book::render(SDL_Renderer& ren,int x, int y, SDL_Rect* extent){
	if(isvisible() == false){ return; }	
	
	SDL_Rect old_clip;
	SDL_Rect new_clip = { 
		x,y,extent->w,extent->h
	};
	SDL_RenderGetClipRect(&ren, &old_clip);
	SDL_RenderSetClipRect(&ren, &new_clip);	

	// render the rect.
	SDL_Rect rect = *extent;
	rect.x = x;
	rect.y = y;
	if(num_panes != 5){		
		SDL_Color cc = focused_colour;
		cc.r += 40;
		Util::render_fill_rectangle(&ren, &rect, cc);
	} else{
		Util::render_fill_rectangle(&ren, &rect, focused_colour);
	}	

	// render the children...	
	render_children(ren, x, y, extent);	
	
	// render a border around it	
	rect = { 
		x-extent->x,
		y-extent->y,
		coord().w(),
		coord().h()
	};
	Util::render_rectangle(&ren, &rect, selected_colour);
	SDL_RenderSetClipRect(&ren, &old_clip);
}
//IPaBook::ne interface
void Book::on_focus(){	
	focused_colour = { 20, 20, 80, 120 };		
}
void Book::off_focus(){		
	focused_colour = { 20, 20, 20, 120 };	
	Pane::defocus_all_children();
	if(selected_shit != nullptr){
		selected_shit->set_selected(false);
		selected_shit = nullptr;		
	}
//	selectaBall.disown();
}
void Book::on_selected(){
	selected_colour = { 255, 20, 10, 140 };
	// do nothing on selection.
}
void Book::off_selected(){
	selected_colour = {0,0 ,255, 140 };
	// deselect any and all children.	
}

void Book::update_on_mouse_motion(Mouseable* origin,int code, void* data){
	// do nothing	
}
void Book::update_on_mouse_drag(Mouseable* origin,int code, void* data){	
	// do nothing
}
void Book::update_on_mouse_buttondown(Mouseable* origin,int code, void* data){	
	SelectablePane* s = (SelectablePane*)data;
	if(s != selected_shit){
		if(selected_shit != nullptr){
			selected_shit->set_selected(false);
		}
		selected_shit = s;		
		selected_shit->set_selected(true);
	}	
}
void Book::update_on_mouse_buttonup(Mouseable* origin,int code, void* data){
	//SelectablePane* s = (SelectablePane*)data;
	if(selected_shit != nullptr){
		selected_shit->set_selected(false);
	}
	selected_shit = nullptr;				
}



Page::Page()
: SelectablePane(), Mouseable()
{ 
	focused_colour = { 20, 60, 40, 90 };
	selected_colour = { 0, 0, 255, 140 };
}

Page::~Page(){ }

bool Page::keyboard_keydown(SDL_Event& ev){ return false; }
bool Page::keyboard_keyup(SDL_Event& ev){ return false; }

bool Page::mouse_buttondown(SDL_Event& ev, Coords* ref ){
	if(has_focus()){
		set_selected(true);
	}
	notify_mouse_buttondown(0,(void*)this);
	return true;
}
bool Page::mouse_buttonup(SDL_Event& ev, Coords* ref ){	
	set_selected(false);	
	notify_mouse_buttonup(0,(void*)this);
	return true;
}
bool Page::mouse_motion(SDL_Event& ev, Coords* ref){
	return true;
}
bool Page::mouse_drag(SDL_Event& ev, Coords* ref){
	if(is_selected()){
		coord().x(coord().x() + ev.motion.xrel);
		coord().y(coord().y() + ev.motion.yrel);
		coord().make_within_bounds(coord().get_parent());
	}
	return true;
}

void Page::tick(){}
void Page::update(SDL_Event& ev){}
void Page::render(SDL_Renderer& ren){	
	SDL_Rect rect = {
		0, 0,
		coord().w(),
		coord().h()
	};
	render(ren, coord().disp_x(), coord().disp_y(), &rect);
}
void Page::render(SDL_Renderer& ren, int x, int y, SDL_Rect* extent){	
	SDL_Rect old_clip;
	SDL_Rect new_clip = {
		x ,y,extent->w,extent->h
	};
	SDL_RenderGetClipRect(&ren, &old_clip);
	SDL_RenderSetClipRect(&ren, &new_clip);

	if(has_focus()){
		// stuff??
	}
	SDL_Rect rect = *extent;
	rect.x = x;
	rect.y = y;
	Util::render_fill_rectangle(&ren,&rect, focused_colour);

	rect = { 
		x - extent->x,
		y - extent->y,
		coord().w(),
		coord().h()		
	};
	Util::render_rectangle(&ren, &rect, selected_colour);

	SDL_RenderSetClipRect(&ren, &old_clip);
}

void Page::on_focus(){
	focused_colour= { 20, 60, 120, 90 };	
}
void Page::off_focus(){
	focused_colour = { 20, 60, 40, 90 };	
	Pane::defocus_all_children();	
}
void Page::on_selected(){
	selected_colour = { 255, 0, 0, 255 };
	// do nothing ...
}
void Page::off_selected(){
	selected_colour = {0, 0, 255, 255 };
	// do nothing ...
}



View_Play::View_Play(SDL_Window& win, SDL_Renderer& ren)
: IView(win,ren)
{
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
	// Book
	book.coord().init(20, 20, 0, 400, 400);
	book.setvisible(true);

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

		book.keyboard_keydown(ev);
		// MY STUFF END

	} else if(ev.type == SDL_KEYUP){
		book.keyboard_keyup(ev);

	}
}
void View_Play::handle_mouse_events(SDL_Event& ev){
	mouse.update();
	button = SDL_GetMouseState(&mouse_x, &mouse_y);
	if(mouse.hitbox().collides(book.hitbox())){
		book.set_focus(true);		
	} else{
		if(book.has_focus()){
			book.set_focus(false);
		}		
	}

	static int count = 0;
	if(ev.type == SDL_MOUSEBUTTONDOWN){
		drag_flag = true;
		book.mouse_buttondown(ev);
	} else if(ev.type == SDL_MOUSEBUTTONUP){
		drag_flag = false;
		book.mouse_buttonup(ev);
	} else if(ev.type == SDL_MOUSEMOTION){
		++count;
		if(drag_flag){
			book.mouse_drag(ev);
		} else{			
			book.mouse_motion(ev);
		}		
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


void View_Play::render(){		
	SDL_RenderClear(&ren);
	//render_graph();
	book.render(ren);
	
	IMouse& m = GMouse::get(&book.coord());
	Util::render_text(&ren, font_carbon_12, 5, 5, font_carbon_12_colour, "%d, %d", m.x(), m.y());
		
	SDL_RenderPresent(&ren);
}

