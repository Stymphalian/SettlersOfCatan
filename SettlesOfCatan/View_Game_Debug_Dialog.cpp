#include <SDL.h>
#include "Logger.h"
#include "Util.h"
#include "Collision.h"
#include "View_Game.h"
#include "Button.h"
#include "CheckBox.h"
#include "IDialog.h"
#include "View_Game_Debug_Dialog.h"

// ---------------------------------------------------------------
//  V I E W _ G A M E _ D E B U G _ D I A L O G _ C H E C K B O X
// ---------------------------------------------------------------
View_Game_Debug_Dialog_CheckBox::View_Game_Debug_Dialog_CheckBox()
: CheckBox(), _monitor_value(nullptr)
{
	Logger::getLog().log(Logger::DEBUG, "View_Game_Debug_Dialog_CheckBox() Constructor");
}

void View_Game_Debug_Dialog_CheckBox::set_data(void* data){
	_monitor_value = (bool*)data;
	if(_monitor_value != nullptr){
		_ticked = (*_monitor_value);
	}
}
void View_Game_Debug_Dialog_CheckBox::on_tick_action(){
	if(_monitor_value != nullptr) {
		*_monitor_value = true;
	}
}
void View_Game_Debug_Dialog_CheckBox::off_tick_action(){
	if(_monitor_value != nullptr) {
		*_monitor_value = false;
	}
}

// ------------------------------------------------
//  V I E W _ G A M E _ D E B U G _ D I A L O G 
// ------------------------------------------------

View_Game_Debug_Dialog::View_Game_Debug_Dialog(
	View_Game& view,
	int x, int y, int z, int w, int h)
	: IDialog(view, x, y, z, w, h), view_game(view)
{

	Logger::getLog().log(Logger::DEBUG, "VieW_Game_Debug_Dialog Constructor");
	// mouse handling
	_mouse_x = 0;
	_mouse_y = 0;
	_mouse_buttons = 0;
	mouse_hitbox.hook(&_mouse_x, &_mouse_y);
	mouse_hitbox.add_rect(0, 0, 0, 1, 1);

	// default nullptr data.
	_data = nullptr;

	// add checkboxes to the list.
	_checkboxes.push_back(&_board_tiles);
	_checkboxes.push_back(&_board_vertices);
	_checkboxes.push_back(&_board_faces);

	_checkboxes.push_back(&_selected_tile_vertices);
	_checkboxes.push_back(&_selected_tile_faces);
	_checkboxes.push_back(&_selected_vertex_vertices);

	_checkboxes.push_back(&_selected_vertex_faces);
	_checkboxes.push_back(&_selected_vertex_tiles);
	_checkboxes.push_back(&_selected_face_vertices);

	_checkboxes.push_back(&_selected_face_faces);
	_checkboxes.push_back(&_selected_face_tiles);

	_checkboxes.push_back(&_selected_pane);
	_checkboxes.push_back(&_item_numbering);

	Logger::getLog().log(Logger::DEBUG, "VieW_Game_Debug_Dialog checkboxes size = %d",(int)_checkboxes.size());
	// Intilize all the checkboxes.
	// two columns of checkboxes.
	int offset_x = 5;
	int offset_y = 5;
	int checkbox_w = 24;
	int checkbox_h = 24;
	for(int i = 0; i < (int)_checkboxes.size(); ++i){
		_checkboxes[i]->init(offset_x, offset_y, 0, checkbox_w, checkbox_h);
		offset_y += checkbox_h + 5;
		if(offset_y + checkbox_h >= this->h){
			offset_y = 5;
			offset_x += w / 2 + 5;
		}
	}

	// group all the checkboxes
	_board_group.add_checkbox(&_board_tiles);
	_board_group.add_checkbox(&_board_vertices);
	_board_group.add_checkbox(&_board_faces);
	_board_group.can_only_select_one = false;

	_selected_tile_group.add_checkbox(&_selected_tile_vertices);
	_selected_tile_group.add_checkbox(&_selected_tile_faces);
	_selected_tile_group.can_only_select_one = true;

	_selected_vertex_group.add_checkbox(&_selected_vertex_vertices);
	_selected_vertex_group.add_checkbox(&_selected_vertex_faces);
	_selected_vertex_group.add_checkbox(&_selected_vertex_tiles);
	_selected_vertex_group.can_only_select_one = true;

	_selected_face_group.add_checkbox(&_selected_face_vertices);
	_selected_face_group.add_checkbox(&_selected_face_faces);
	_selected_face_group.add_checkbox(&_selected_face_tiles);
	_selected_face_group.can_only_select_one = true;


	// TODO : Have a resource class for fonts
	font_carbon_12 = TTF_OpenFont(Util::data_resource("carbon.ttf").c_str(), 12);
	font_carbon_12_colour = { 177, 177, 98, 255 };
	if(font_carbon_12 == nullptr){
		Logger::getLog().TTF_log(Logger::ERROR, "View_Game::load() TTF_OpenFont data/carbon.ttf ");
	}
}

View_Game_Debug_Dialog::~View_Game_Debug_Dialog(){
	Logger::getLog().log(Logger::DEBUG, "VieW_Game_Debug_Dialog Destructor");
	TTF_CloseFont(font_carbon_12);
	font_carbon_12 = nullptr;
	_data = nullptr;
	_checkboxes.clear();
	_board_group.clear();
	_selected_tile_group.clear();
	_selected_vertex_group.clear();
	_selected_face_group.clear();
}

// TODO: RUCK This siht1
bool View_Game_Debug_Dialog::open(void* data) {
	IDialog::open(data);
	Logger::getLog().log(Logger::DEBUG, "View_Game_Debug_Dialog::open(data=%x) Opening the dialog", data);
	_data = (view_debug_t*)data;

	// set the label for the checkboxes
	_board_tiles.set_label("board tiles");
	_board_vertices.set_label("board vertices");
	_board_faces.set_label("board faces");
	_selected_tile_vertices.set_label("selected tile vertices");
	_selected_tile_faces.set_label("selected tile faces");
	_selected_vertex_vertices.set_label("selected vertex vertices");
	_selected_vertex_faces.set_label("selected vertex faces");
	_selected_vertex_tiles.set_label("selected vertex tiles");
	_selected_face_vertices.set_label("selected face vertices");
	_selected_face_faces.set_label("selected face faces");
	_selected_face_tiles.set_label("selected faces tiles");
	_selected_pane.set_label("selected_pane");
	_item_numbering.set_label("item numbering");

	// set the data value that the checkboxes will manipulate
	_board_tiles.set_data((void*)&_data->board_tiles);
	_board_vertices.set_data((void*)&_data->board_vertices);
	_board_faces.set_data((void*)&_data->board_faces);;
	_selected_pane.set_data((void*)&_data->selected_pane);
	_item_numbering.set_data((void*)&_data->item_numbering);
	_selected_tile_vertices.set_data((void*)&_data->selected_tile_vertices);
	_selected_tile_faces.set_data((void*)&_data->selected_tile_faces);
	_selected_vertex_vertices.set_data((void*)&_data->selected_vertex_vertices);
	_selected_vertex_faces.set_data((void*)&_data->selected_vertex_faces);
	_selected_vertex_tiles.set_data((void*)&_data->selected_vertex_tiles);
	_selected_face_vertices.set_data((void*)&_data->selected_face_vertices);
	_selected_face_faces.set_data((void*)&_data->selected_face_faces);
	_selected_face_tiles.set_data((void*)&_data->selected_face_tiles);
	return true;
}

void* View_Game_Debug_Dialog::close() {
	IDialog::close();
	Logger::getLog().log(Logger::DEBUG, "View_Game_Debug_Dialog::close() Closing the dialog. Passing back %x", _data);
	return _data;
}

void View_Game_Debug_Dialog::handle_keyboard_events(SDL_Event& ev) {
	const Uint8* keyboard = SDL_GetKeyboardState(NULL);
	if(ev.type == SDL_KEYDOWN){
		if(keyboard[SDL_SCANCODE_ESCAPE]){
			Util::get().push_userev(
				Util::get().get_userev("close_dialog_event"),
				0, this, 0
				);
		}

		if(keyboard[SDL_SCANCODE_1]){
			modal = (modal) ? false : true;
		}
	}
}

void View_Game_Debug_Dialog::handle_mouse_events(SDL_Event& ev) {
	_mouse_buttons = SDL_GetMouseState(&_mouse_x, &_mouse_y);
	// make mouse position relative to the dialog box
	_mouse_x -= this->x;
	_mouse_y -= this->y;

	if(ev.type == SDL_MOUSEBUTTONDOWN){
		if(_selected_checkbox != nullptr){
			_selected_checkbox->click();
		}
	}
}

void View_Game_Debug_Dialog::update(SDL_Event& ev) {
	// check for collisions with the mouse.
	_selected_checkbox = nullptr;
	for(int i = 0; i < (int)_checkboxes.size(); ++i){
		if(_checkboxes[i]->hitbox.collides(mouse_hitbox)){
			_selected_checkbox = _checkboxes[i];
			break;
		}
	}
}

void View_Game_Debug_Dialog::render(){
	SDL_Rect rect = { 0, 0, 0, 0 };
	SDL_Color colour = { 180, 90, 0, 255 };
	SDL_Rect new_clip = { this->x, this->y, this->w, this->h };
	SDL_Rect old_clip;

	// TODO: This might be a little ineffienct, clearing and drawing
	// the area again, but we will see.
	SDL_RenderSetClipRect(&view.ren, &new_clip);
	SDL_RenderClear(&view.ren);

	for(int i = 0; i < (int)_checkboxes.size(); ++i){
		rect = {
			_checkboxes[i]->x + this->x,
			_checkboxes[i]->y + this->y,
			_checkboxes[i]->w,
			_checkboxes[i]->h
		};

		if(_checkboxes[i]->is_ticked()){
			Util::render_fill_rectangle(&view.ren, &rect, colour);
		} else{
			Util::render_rectangle(&view.ren, &rect, colour);
		}

		Util::render_text(&view.ren, font_carbon_12, rect.x + rect.w + 5, rect.y, font_carbon_12_colour,
			"%s", _checkboxes[i]->get_label().c_str());
	}

	rect = { this->x, this->y, this->w, this->h };
	Util::render_rectangle(&view.ren, &rect, colour);

	// TODO: I think we might not need this.
	// We should only do a render present at the end of ALL the drawing.
	// So therefore only in IView classes.
	//SDL_RenderPresent(&view.ren);	
	SDL_RenderSetClipRect(&view.ren, &old_clip);
}