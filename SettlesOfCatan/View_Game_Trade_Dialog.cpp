#include <SDL.h>
#include "Logger.h"
#include "Util.h"
#include "Configuration.h"

#include "IDialog.h"
#include "View_Game.h"
#include "View_Game_Trade_Dialog.h"
#include "Button.h"
#include "TextField.h"
#include "ComboBox.h"
#include "DropDown.h"
#include "Player.h"

// -----------------------------------------------------------------
// - V I E W _ G A M E _ T R A D E _ D I A L O G _ D R O P D O W N
// -----------------------------------------------------------------
View_Game_Trade_Dialog_DropDown::View_Game_Trade_Dialog_DropDown()
: DropDown()
{
	Logger::getLog().log(Logger::DEBUG, "View_Game_Trade_Dialog_DropDown() constructor");
}

View_Game_Trade_Dialog_DropDown::~View_Game_Trade_Dialog_DropDown()
{
	Logger::getLog().log(Logger::DEBUG, "View_Game_Trade_Dialog_DropDown() destructor");
}


// -----------------------------------------------------------------
// - V I E W _ G A M E _ T R A D E _ D I A L O G _ C O M B O B O X 
// -----------------------------------------------------------------
View_Game_Trade_Dialog_ComboBox::View_Game_Trade_Dialog_ComboBox()
: ComboBox()
{
	Logger::getLog().log(Logger::DEBUG, "View_Game_Trade_Dialog_ComboBox constructor");
}
View_Game_Trade_Dialog_ComboBox::~View_Game_Trade_Dialog_ComboBox()
{
	Logger::getLog().log(Logger::DEBUG, "View_Game_Trade_Dialog_ComboBox destructor");
}


// -----------------------------------------------------------------
// - V I E W _ G A M E _ T R A D E _ D I A L O G _ T E X T F I E L D
// -----------------------------------------------------------------

View_Game_Trade_Dialog_TextField::View_Game_Trade_Dialog_TextField()
: TextField()
{
	Logger::getLog().log(Logger::DEBUG, "View_Game_Trade_Dialog_TextField constructor");
}
View_Game_Trade_Dialog_TextField::~View_Game_Trade_Dialog_TextField()
{
	Logger::getLog().log(Logger::DEBUG, "View_Game_Trade_Dialog_TextField destructor");
}

// -----------------------------------------------------------------
// -- --  V I E W _ G A M E _ T R A D E _ D I A L O G _ B U T T O N
// -----------------------------------------------------------------
View_Game_Trade_Dialog_Button::View_Game_Trade_Dialog_Button()
: Button()
{
	Logger::getLog().log(Logger::DEBUG, "View_Game_Trade_Dialog_Button constructor");
	_baction = nullptr;
}
View_Game_Trade_Dialog_Button::~View_Game_Trade_Dialog_Button()
{
	Logger::getLog().log(Logger::DEBUG, "View_Game_Trade_Dialog_Button destructor");
	_baction = nullptr;
}

void View_Game_Trade_Dialog_Button::action(View_Game_Trade_Dialog* dialog, Model* model){
	if(_baction != nullptr){
		_baction(dialog, model);
	}
	return;
}
void View_Game_Trade_Dialog_Button::set_action(button_action baction){
	_baction = baction;
}
void View_Game_Trade_Dialog_Button::unset_action(){
	_baction = nullptr;
}
void View_Game_Trade_Dialog_Button::submit_button_action(View_Game_Trade_Dialog* dialog, Model* model){
	Logger::getLog().log(Logger::DEBUG, "View_Game_Trade_Dialog_Button::submit_button_action(%x,%x)", dialog, model);
	// one of two thigns can be done here
	// 1) The dialog directly manipulates the models and performs the trade
	// transaction
	// 2) We pass back an envelope of the changes that we want done
	// to the model.
	Util::get().push_userev(
		Util::get().get_userev("close_dialog_event"),
		0, dialog, 0);
	return;
}
void View_Game_Trade_Dialog_Button::cancel_button_action(View_Game_Trade_Dialog* dialog, Model* model){
	Logger::getLog().log(Logger::DEBUG, "View_Game_Trade_Dialog_Button::cancel_button_action(%x,%x)",dialog,model);
	// Just close the dialog. Don't need to do anything.
	Util::get().push_userev(
		Util::get().get_userev("close_dialog_event"),
		0, dialog, 0);
}

// -----------------------------------------------------------------
// -- -- -- -- V I E W _ G A M E _ T R A D E _ D I A L O G -- -- --
// -----------------------------------------------------------------
View_Game_Trade_Dialog::View_Game_Trade_Dialog(View_Game& view, int x, int y, int z, int w, int h)
: IDialog(view, x, y, z, w, h), view_game(view)
{
	Logger::getLog().log(Logger::DEBUG, "View_Game_Trade_Dialog(view=%x,x=%d,y=%d,z=%d,w=%d,h=%d) constructor",&view, x, y, z, w, h);
	_mouse_x = 0;
	_mouse_y = 0;
	_mouse_buttons = 0;
	mouse_hitbox.hook(&_mouse_x, &_mouse_y);
	mouse_hitbox.add_rect(0, 0, 0, 1, 1);

	// initialize the buttons
	selected_button = nullptr;
	submit_button.set_action(View_Game_Trade_Dialog_Button::submit_button_action);
	cancel_button.set_action(View_Game_Trade_Dialog_Button::cancel_button_action);	
	submit_button.init("Submit", this->w - 130, this->h - 30, 0, 0, 30);
	cancel_button.init("Cancel", submit_button.x + submit_button.w + 5, this->h - 30, 0, 0, 30);	
	button_list.push_back(&submit_button);
	button_list.push_back(&cancel_button);

	// initialize the combos boxes.


	// initialize the textfields
	selected_textfield = nullptr;	
	left_textfields.push_back(&left_brick_textfield);
	left_textfields.push_back(&left_ore_textfield);
	left_textfields.push_back(&left_sheep_textfield);
	left_textfields.push_back(&left_wheat_textfield);
	left_textfields.push_back(&left_wood_textfield);
	right_textfields.push_back(&right_brick_textfield);
	right_textfields.push_back(&right_ore_textfield);
	right_textfields.push_back(&right_sheep_textfield);
	right_textfields.push_back(&right_wheat_textfield);
	right_textfields.push_back(&right_wood_textfield);

	// place all the textfields on the screen.
	int x_offset = this->w/4;
	int y_offset = this->h/8;
	int padding = 5;
	int num_cols = 3;
	int num_rows = 2;
	int char_w = 9;
	int char_h = 10;
	// init the left side
	for(int i = 0; i < (int)left_textfields.size(); ++i){
		left_textfields[i]->init(x_offset, y_offset, 0, num_cols, num_rows, char_w,char_h);
		left_textfields[i]->set_max_num_chars(3);
		Logger::getLog().log(Logger::DEBUG, "View_Game_Trade_Dialog() textfield  left[%d] = %d,%d", i, left_textfields[i]->x, left_textfields[i]->y);

		x_offset += 0;
		y_offset += num_rows*char_h + padding;
	}

	// init the right side
	x_offset = this->w / 4 + this->w / 2;
	y_offset = this->h/8;
	for(int i = 0; i < (int)right_textfields.size(); ++i){
		right_textfields[i]->init(x_offset, y_offset, 0, num_cols, num_rows, char_w, char_h);
		right_textfields[i]->set_max_num_chars(3);
		Logger::getLog().log(Logger::DEBUG, "View_Game_Trade_Dialog() textfield right[%d] = %d,%d", i,right_textfields[i]->x, right_textfields[i]->y );

		x_offset += 0;
		y_offset += num_rows*char_h + padding;
	}

	// TODO: Have a resource class for fonts
	font_carbon_12_colour = { 177, 177, 98, 255 };
	font_carbon_12 = TTF_OpenFont(Util::data_resource("carbon.ttf").c_str(),12);
	if(font_carbon_12 == nullptr){
		Logger::getLog().TTF_log(Logger::ERROR, "View_Game_Trade_Dialog() TTF_Openfont");
	}
}

View_Game_Trade_Dialog::~View_Game_Trade_Dialog()
{
	Logger::getLog().log(Logger::DEBUG, "View_Game_Trade_Dialog destructor");
	TTF_CloseFont(font_carbon_12);
}

bool View_Game_Trade_Dialog::open(void* data){ 
	Logger::getLog().log(Logger::DEBUG, "View_Game_Trade_Dialog::open(data=%x)", data);
	IDialog::open(data);
	_model = (Model*)data;

	selected_button = nullptr;
	selected_textfield = nullptr;

	// not much that we have to do here.
	return true; 
}
void* View_Game_Trade_Dialog::close(){ 
	IDialog::close();
	Logger::getLog().log(Logger::DEBUG, "View_Game_Trade_Dialog::close() Closing the dialog. Passing back data %x",_model);
	return nullptr;
}
void View_Game_Trade_Dialog::handle_keyboard_events(SDL_Event& ev){
	// handle the text fields
	if(selected_textfield != nullptr && selected_textfield->has_focus){
		selected_textfield->handle_keyboard_events(ev);
		return;
	}
	// handle the buttons
		// do nothing

	// handle the combo box

	// handle the dialog
	const Uint8* keyboard = SDL_GetKeyboardState(NULL);
	if(ev.type == SDL_KEYDOWN){
		if(keyboard[SDL_SCANCODE_ESCAPE]){
			//close the dialog
			Util::get().push_userev(
				Util::get().get_userev("close_dialog_event"),
				0,this, 0
			);
		}
	}
}
void View_Game_Trade_Dialog::handle_mouse_events(SDL_Event& ev){
	_mouse_buttons = SDL_GetMouseState(&_mouse_x, &_mouse_y);
	_mouse_x -= this->x;
	_mouse_y -= this->y;

	// handle any intersections with the mouse.
	selected_textfield = nullptr;
	for(int i = 0; i < (int)left_textfields.size(); ++i){
		left_textfields[i]->handle_mouse_events(ev, mouse_hitbox);
		if(left_textfields[i]->has_focus){
			selected_textfield = left_textfields[i];
		}
	}
	for(int i = 0; i < (int)right_textfields.size(); ++i){
		right_textfields[i]->handle_mouse_events(ev, mouse_hitbox);
		if(right_textfields[i]->has_focus){
			selected_textfield = right_textfields[i];
		}
	}


	selected_button = nullptr;
	if(ev.type == SDL_MOUSEBUTTONDOWN){
		for(int i = 0; i < (int)button_list.size(); ++i){
			if(mouse_hitbox.collides(button_list[i]->hitbox)){
				button_list[i]->hit_flag = true;
				selected_button = button_list[i];
			} else{
				button_list[i]->hit_flag = false;
			}
		}
	}else if(ev.type == SDL_MOUSEBUTTONUP){
		for(int i = 0; i < (int)button_list.size(); ++i){
			if(mouse_hitbox.collides(button_list[i]->hitbox) &&
				button_list[i]->hit_flag == true)
			{
				button_list[i]->action(this, _model);
			}
			button_list[i]->hit_flag = false;
		}		
	}
	
}
void View_Game_Trade_Dialog::update(SDL_Event& ev){
	if(selected_textfield != nullptr){
		selected_textfield->tick();
	}
}
void View_Game_Trade_Dialog::render(){
	// TODO: this is repeated code.
	int x_offset = 0;
	int y_offset = 0;
	SDL_Rect rect = { 0, 0, 0, 0 };
	SDL_Color colour = { 180, 90, 0, 255 };
	std::string resource_strings[5] = {"Bricks:", "Ore:", "Sheep:", "Wheat:", "Wood:"};

	// TODO: This might be a little ineffienct, clearing and drawing
	// the area again, but we will see.
	//SDL_RenderSetClipRect(&view.ren, &new_clip);
	SDL_RenderClear(&view.ren);
	//SDL_RenderSetClipRect(&view.ren, &old_clip);


	// -- -- -- -- --  L E F T  -- -- -- -- -- -- --
	// render the player's name
	int player_num = _model->get_current_player();
	Player* p = _model->get_player(player_num);
	Util::render_text(&view.ren, font_carbon_12, 5 +this->x, 5 + this->y, font_carbon_12_colour,
		"%d : %s", player_num, p->name.c_str());


	// render the text fields and labels
	rect = { this->x+1 , this->y+1, this->w / 2-1, this->h -1};
	Util::render_rectangle(&view.ren, &rect, Util::colour_green());
	x_offset = 0;
	y_offset = 0;
	for(int i = 0; i < (int)left_textfields.size(); ++i){
		x_offset = this->x;
		y_offset = this->y  + left_textfields[i]->y;
		Util::render_text(&view.ren, font_carbon_12,x_offset, y_offset, font_carbon_12_colour,
			"%s", resource_strings[i].c_str());

		SDL_Rect pane_rect = { this->x, this->y, this->w, this->h };
		left_textfields[i]->render(view.ren, *font_carbon_12, font_carbon_12_colour, pane_rect);
	}



	// -- - - - -- - R I G H T -- - - - -- - -
	// render the combo-box
	// render the text fields and labels
	rect = { this->x + this->w / 2 + 1, this->y+1, this->w / 2-1, this->h -1};
	Util::render_rectangle(&view.ren, &rect, Util::colour_blue());
	x_offset = 0;
	y_offset = 0;
	for(int i = 0; i < (int)right_textfields.size(); ++i){
		x_offset = this->x + this->w/2;
		y_offset = this->y + right_textfields[i]->y;
		Util::render_text(&view.ren, font_carbon_12, x_offset, y_offset, font_carbon_12_colour,
			"%s", resource_strings[i].c_str());

		SDL_Rect pane_rect = { this->x, this->y, this->w, this->h };
		right_textfields[i]->render(view.ren, *font_carbon_12, font_carbon_12_colour, pane_rect);
	}


	// render the buttons
	std::vector<View_Game_Trade_Dialog_Button*>::iterator it;
	for(it = button_list.begin(); it != button_list.end(); ++it){
		rect = { 
			(*it)->x + this->x,
			(*it)->y + this->y,
			(*it)->w, (*it)->h };

		if((*it)->hit_flag){
			Util::render_rectangle(&view.ren, &rect, Util::colour_blue());
		} else{
			Util::render_rectangle(&view.ren, &rect, Util::colour_green());
		}
		Util::render_text(&view.ren, font_carbon_12, rect.x, rect.y, font_carbon_12_colour, "%s", (*it)->text.c_str());
	}


	// draw a bounding box around the dialog.
	rect = { this->x, this->y, this->w, this->h };
	Util::render_rectangle(&view.ren, &rect, colour);

	SDL_RenderSetClipRect(&view.ren, &old_clip);
}