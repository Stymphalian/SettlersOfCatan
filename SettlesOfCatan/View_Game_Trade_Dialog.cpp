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

// -----------------------------------------------------------------
// - V I E W _ G A M E _ T R A D E _ D I A L O G _ D R O P D O W N
// -----------------------------------------------------------------
View_Game_Trade_Dialog_DropDown::View_Game_Trade_Dialog_DropDown()
: DropDown()
{

}

View_Game_Trade_Dialog_DropDown::~View_Game_Trade_Dialog_DropDown()
{

}


// -----------------------------------------------------------------
// - V I E W _ G A M E _ T R A D E _ D I A L O G _ C O M B O B O X 
// -----------------------------------------------------------------
View_Game_Trade_Dialog_ComboBox::View_Game_Trade_Dialog_ComboBox()
: ComboBox()
{

}
View_Game_Trade_Dialog_ComboBox::~View_Game_Trade_Dialog_ComboBox()
{

}


// -----------------------------------------------------------------
// - V I E W _ G A M E _ T R A D E _ D I A L O G _ T E X T F I E L D
// -----------------------------------------------------------------

View_Game_Trade_Dialog_TextField::View_Game_Trade_Dialog_TextField()
: TextField()
{

}
View_Game_Trade_Dialog_TextField::~View_Game_Trade_Dialog_TextField()
{

}

// -----------------------------------------------------------------
// -- --  V I E W _ G A M E _ T R A D E _ D I A L O G _ B U T T O N
// -----------------------------------------------------------------
View_Game_Trade_Dialog_Button::~View_Game_Trade_Dialog_Button()
{
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
void View_Game_Trade_Dialog_Button::submit_button(View_Game_Trade_Dialog* dialog, Model* model){
	return;
}
void View_Game_Trade_Dialog_Button::cancel_button(View_Game_Trade_Dialog* dialog, Model* model){
	return;
}

// -----------------------------------------------------------------
// -- -- -- -- V I E W _ G A M E _ T R A D E _ D I A L O G -- -- --
// -----------------------------------------------------------------
View_Game_Trade_Dialog::View_Game_Trade_Dialog(View_Game& view, int x, int y, int z, int w, int h)
: IDialog(view, x, y, z, w, h), view_game(view)
{
	_mouse_x = 0;
	_mouse_y = 0;
	_mouse_buttons = 0;
	mouse_hitbox.hook(&_mouse_x, &_mouse_y);
	mouse_hitbox.add_rect(0, 0, 0, 1, 1);


	// TODO: Have a resource class for fonts
	font_carbon_12_colour = { 177, 177, 98, 255 };
	font_carbon_12 = TTF_OpenFont(Util::data_resource("carbon.ttf").c_str(),12);
	if(font_carbon_12 == nullptr){
		Logger::getLog().TTF_log(Logger::ERROR, "View_Game_Trade_Dialog() TTF_Openfont");
	}
}

View_Game_Trade_Dialog::~View_Game_Trade_Dialog()
{
	TTF_CloseFont(font_carbon_12);
}

bool View_Game_Trade_Dialog::open(void* data){ 
	IDialog::open(data);
	_model = (Model*)data;
	// init buttons
	// init left text fields
	// init right text fields
	return true; 
}
void* View_Game_Trade_Dialog::close(){ 
	IDialog::close();
	return nullptr;
}
void View_Game_Trade_Dialog::handle_keyboard_events(SDL_Event& ev){
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
}
void View_Game_Trade_Dialog::update(SDL_Event& ev){
}
void View_Game_Trade_Dialog::render(){
}