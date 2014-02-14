#include <SDL.h>
#include "Logger.h"
#include "Util.h"
#include "IView.h"
#include "IDialog.h"
#include "Player.h"
#include "View_Game.h"
#include "View_Game_Model_State.h"
#include "View_Game_DevCard_Dialog.h"


// -- -- -- -- -- -- -- -- - -- -- -- -- --- -- -- - -- - -- - - 
// V I E W _ G A M E _ D E V C A R D _ D I A L O G _ B U T T O N
// -- -- -- -- -- -- -- -- - -- -- -- -- --- -- -- - -- - -- - -
View_Game_DevCard_Dialog_Button::View_Game_DevCard_Dialog_Button()
:Button()
{
	Logger::getLog().log(Logger::DEBUG, "View_Game_DevCard_Dialog_Button constructor");
	baction = nullptr;
}

View_Game_DevCard_Dialog_Button::~View_Game_DevCard_Dialog_Button(){
	Logger::getLog().log(Logger::DEBUG, "View_Game_DevCard_Dialog_Button destructor");
	baction = nullptr;
}

void View_Game_DevCard_Dialog_Button::set_action(button_action baction){
	this->baction = baction;
}
void View_Game_DevCard_Dialog_Button::unset_action(){
	this->baction = nullptr;
}
void View_Game_DevCard_Dialog_Button::action(View_Game_DevCard_Dialog& dialog, Model& model){
	if(this->baction != nullptr){
		this->baction(dialog, model);
	}
}
void View_Game_DevCard_Dialog_Button::play_card_button_action(View_Game_DevCard_Dialog& dialog, Model& model){
	Logger::getLog().log(Logger::DEBUG, "View_Game_DevCard_Dialog::play_card_button_action dialog=%x, model=%x", dialog, model);
	// play the dev card that was called to be played.

	// set the card as visible
	dev_cards_t* card = dialog.get_chosen_card();
	if(card == nullptr){ return; }
	if(card->has_been_played == true){ return; }
	Logger::getLog().log(Logger::DEBUG, "View_Game_DevCard_Dialog::play_card_button_action Playing card %x", card);
	card->visible = true;
	card->has_been_played = true;		

	if(card->type == dev_cards_t::SOLDIER){
		View_Game_Model_State_Context& context = dialog._view_game.model_state_context;
		context.push_state(context.obtain_state(View_Game_Model_State_Context::PLACE_THIEF));
		dialog._player->num_soldiers++;
	} else if(card->type == dev_cards_t::VICTORY){
		// don't really need to do anything.		
	} else if(card->type == dev_cards_t::MONOPOLY){
		// transition... to other dialog?
	} else if(card->type == dev_cards_t::YEAR_PLENTY){
		// 
	} else if(card->type == dev_cards_t::ROAD_BUILDING){
		View_Game_Model_State_Context& context = dialog._view_game.model_state_context;
		context.push_state(context.obtain_state(View_Game_Model_State_Context::BUILD_ROAD));
		context.push_state(context.obtain_state(View_Game_Model_State_Context::BUILD_ROAD));	
	}
	
	// send the close event.
	Util::get().push_userev(
		Util::get().get_userev("close_dialog_event"),
		0, &dialog, 0);
}
void View_Game_DevCard_Dialog_Button::cancel_button_action(View_Game_DevCard_Dialog& dialog, Model& model){
	Logger::getLog().log(Logger::DEBUG, "View_Game_DevCard_Dialog::cancel_button_action dialog=%x, model=%x", dialog, model);
	// send a close dialog event.
	Util::get().push_userev(
		Util::get().get_userev("close_dialog_event"),
		0, &dialog, 0);
}

// -- -- -- -- -- -- -- -- -- -- -- --  -- -- -- -- - - -- -- --
// V I E W _ G A  M E _ D E V C A R D _ C A R D
// -- -- -- -- -- -- -- -- -- -- -- --  -- -- -- -- - - -- -- --
View_Game_DevCard_Card::View_Game_DevCard_Card(){
	Logger::getLog().log(Logger::DEBUG, "View_Game_DevCard_Card constructor");
	x = 0;y = 0;z = 0;
	w = 0; h = 0;
	card = nullptr;
}
View_Game_DevCard_Card::~View_Game_DevCard_Card(){
	Logger::getLog().log(Logger::DEBUG, "View_Game_DevCard_Card destructor");
	card = nullptr;
}
void View_Game_DevCard_Card::render(SDL_Renderer& ren, TTF_Font* font, SDL_Color font_colour, SDL_Rect pane)
{
	if(card == nullptr){ return; }

	SDL_Color dark_gray = { 60, 60, 60, 65 };
	SDL_Color gray = { 110, 110, 110, 80 };
	SDL_Color blue = { 20, 110, 30, 80 };
	SDL_Color* chosen_colour = (_ticked) ? &blue : 
								(card->visible) ? &gray : &dark_gray ;
	SDL_Rect rect = {
		this->x + pane.x,
		this->y + pane.y,
		this->w,this->h
	};
	
	Util::render_fill_rectangle(&ren, &rect,*chosen_colour);
	int x_text = rect.x + 10; 
	int y_text = rect.y + this->h / 2 - 10; // -10 to handle the text height
//	Util::render_text(&ren, font, x_text, y_text, font_colour, "title = %s", card->title().c_str());
}
void View_Game_DevCard_Card::set_card(const dev_cards_t* card){
	this->card = (dev_cards_t*)card;
}

void View_Game_DevCard_Card::set_data(void* data){}
void View_Game_DevCard_Card::on_tick_action(){}
void View_Game_DevCard_Card::off_tick_action(){}

// -- -- -- -- -- -- -- -- -- -- -- --  -- -- -- -- - - -- -- --
// V I E W _ G A  M E _ D E V C A R D _ C A R D _ P R E V I E W
// -- -- -- -- -- -- -- -- -- -- -- --  -- -- -- -- - - -- -- --
void View_Game_DevCard_Card_Preview::init(int x, int y, int z, int w, int h){
	this->x = x; 
	this->y = y;
	this->z = z;
	this->w = w;
	this->h = h;
	card = nullptr;
}
void View_Game_DevCard_Card_Preview::set_card(const dev_cards_t* card){
	this->card = (dev_cards_t*) card;
}
void View_Game_DevCard_Card_Preview::render(
	SDL_Renderer& ren,TTF_Font* font, SDL_Color font_colour, SDL_Rect pane)
{
	
	SDL_Color background_colour = { 80, 20, 80, 70 };
	SDL_Color foreground_colour = { 180, 180, 180, 70 };
	SDL_Rect rect = {
		this->x + pane.x,
		this->y + pane.y,
		this->w, this->h
	};
	Util::render_fill_rectangle(&ren, &rect, background_colour);

	if(card == nullptr){ return; }
	// title
	int x_offset = rect.x + 5;
	int y_offset = rect.y + 5;
	Util::render_text(&ren, font, x_offset,y_offset, font_colour, "%s", card->title().c_str());

	// line spacer
	x_offset += 0;
	y_offset += 15;
	Util::render_line(&ren, foreground_colour, x_offset, y_offset, x_offset + this->w - 2*5, y_offset);

	x_offset = rect.x + 5;
	y_offset += 15;
	// message
	Util::render_text(&ren, font, x_offset, y_offset, font_colour, "%s", card->message().c_str());		

	// render a border
	Util::render_rectangle(&ren, &rect, foreground_colour);
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -
// I P A N E
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -
// V I E W _ G A M E _ D E V C A R D _ D I A L O G
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -
View_Game_DevCard_Dialog::View_Game_DevCard_Dialog(
	View_Game& view, int x, int y, int z, int w, int h)
	:IDialog((IView&) view, x, y, z, w, h), _view_game(view)
{
	Logger::getLog().log(Logger::DEBUG, "View_Game_DevCard_Dialog constructor");
	_model = nullptr;
	_player = nullptr;

	// create the font that we will be using for stuff...
	font_carbon_12_colour = { 177, 177, 98, 255 };
	font_carbon_12 = TTF_OpenFont(Util::data_resource("carbon.ttf").c_str(), 12);
	if(font_carbon_12 == nullptr){
		Logger::getLog().TTF_log(Logger::ERROR, "View_Game_Trade_Dialog() TTF_Openfont");
	}
		
	// initialize the card objects
	_selected_card = nullptr;	
	_visible_cards.clear();
	_hidden_cards.clear();
	_cards_available.clear();
	_card_store.clear();
	_card_groups.clear();
	_card_groups.can_only_select_one = true;
	_card_preview.init(5,20,0,this->w - 2*5,this->h/2 -2*20 -5);

	// init the buttons
	int button_w = 0;
	int button_h = 30;
	_selected_button = nullptr;	
	_clicked_button = nullptr;
	cancel_button.set_action(View_Game_DevCard_Dialog_Button::cancel_button_action);
	play_card_button.set_action(View_Game_DevCard_Dialog_Button::play_card_button_action);
	play_card_button.init("Play Card", this->w - 160, this->h - button_h -5, 0, button_w, button_h );
	cancel_button.init("Cancel", play_card_button.x + play_card_button.w + 5, this->h - button_h -5,0,button_w,button_h);
	button_list.clear();
	button_list.push_back(&play_card_button);
	button_list.push_back(&cancel_button);
}

View_Game_DevCard_Dialog::~View_Game_DevCard_Dialog(){
	Logger::getLog().log(Logger::DEBUG, "View_Game_DevCard_Dialog destructor");
	TTF_CloseFont(font_carbon_12);

	_model = nullptr;
	_player = nullptr;
	_selected_card = nullptr;
	_visible_cards.clear();
	_hidden_cards.clear();
	_selected_button = nullptr;
	_clicked_button = nullptr;
	button_list.clear();
	_cards_available.clear();
	_card_store.clear();	
}

bool View_Game_DevCard_Dialog::open(void* data){ 
	IDialog::open(data);
	_model = (Model*)data;
	_player = _model->get_player(_model->get_current_player());
	if(_model == nullptr){ return false; }
	if(_player == nullptr){ return false; }

	_selected_button = nullptr;
	_clicked_button = nullptr;

	initialize_cards();
	
	return true; //??
}
void* View_Game_DevCard_Dialog::close(){
	IDialog::close();	
	_model = nullptr;
	_player = nullptr;
	_selected_card = nullptr;
	_selected_button = nullptr;
	_clicked_button = nullptr;
	return_cards_to_available(_visible_cards);
	return_cards_to_available(_hidden_cards);	
	_card_groups.clear();
	_card_preview.set_card(nullptr);
	return nullptr;
}

void View_Game_DevCard_Dialog::handle_keyboard_events(SDL_Event& ev){
	const Uint8* keyboard = SDL_GetKeyboardState(NULL);
	if(ev.type == SDL_KEYDOWN){
		if(keyboard[SDL_SCANCODE_ESCAPE]){
			// close the dialog.
			Util::get().push_userev(
				Util::get().get_userev("close_dialog_event"),
				0, this, 0);				
		}
	} else if(ev.type == SDL_KEYUP){

	}
}
void View_Game_DevCard_Dialog::handle_mouse_events(SDL_Event& ev){	
	_mouse_buttons = SDL_GetMouseState(&_mouse_x, &_mouse_y);
	_mouse_x -= this->x;
	_mouse_y -= this->y;	
	
	if(ev.type == SDL_MOUSEBUTTONDOWN){
		if(_selected_button != nullptr){
			_clicked_button = _selected_button;
		}

		if(_selected_card != nullptr){
			_selected_card->click();
		}
				
	} else if(ev.type == SDL_MOUSEBUTTONUP){
		if(_selected_button != _clicked_button){_clicked_button = nullptr;}				
		if(_clicked_button != nullptr){
			_clicked_button->action(*this,*_model);
			_clicked_button = nullptr;
		}

	} else if(ev.type == SDL_MOUSEMOTION){

	}	
}

void View_Game_DevCard_Dialog::update(SDL_Event& ev){
	// animations?

	// check for button intersections	
	_selected_button = nullptr;
	for(int i = 0; i < (int)button_list.size(); ++i){
		if(button_list[i]->hitbox.collides(_mouse_hitbox)){
			button_list[i]->hit_flag = true;
			_selected_button = button_list[i];
		} else{
			button_list[i]->hit_flag = false;
		}
	}

	// check for card intersections
	do{
		_selected_card = nullptr;		
		for(int i = 0; i < (int)_visible_cards.size(); ++i){
			if(_visible_cards[i]->hitbox.collides(_mouse_hitbox)){
				_selected_card = _visible_cards[i];
				break;
			}
		}
		if(_selected_card != nullptr){ break; }

		for(int i = 0; i < (int)_hidden_cards.size(); ++i){
			if(_hidden_cards[i]->hitbox.collides(_mouse_hitbox)){
				_selected_card = _hidden_cards[i];
				break;
			}
		}
	} while(false);
}

void View_Game_DevCard_Dialog::render(){
	SDL_Rect rect = { 0, 0, 0, 0 };
	SDL_Color c1 = { 110, 110, 110, 80 };

	SDL_RenderSetClipRect(&view.ren, &new_clip);
	SDL_RenderClear(&view.ren);

	// draw a box around the dialog box
	rect = { this->x, this->y, this->w, this->h };
	Util::render_rectangle(&view.ren, &rect, Util::colour_orange());

	// draw a line in the middle of the screen to divide visible and hidden cards
	Util::render_line(&view.ren, Util::colour_orange(), 
		0 + this->x, 
		this->y + this->h / 2,
		this->x + this->w, 
		this->y + this->h / 2);

	// draw the current player label
	Util::render_text(&view.ren, font_carbon_12, this->x + 5,this->y + 5, font_carbon_12_colour, "%d %s", _model->get_current_player(), _player->name.c_str());
		
	
	// render the buttons
	std::vector<View_Game_DevCard_Dialog_Button*>::iterator it;
	for(it = button_list.begin(); it != button_list.end(); ++it){
		rect = {
			(*it)->x + this->x,
			(*it)->y + this->y ,
			(*it)->w, (*it)->h };

		if((*it)->hit_flag){
			Util::render_rectangle(&view.ren, &rect, Util::colour_blue());
		} else{
			Util::render_rectangle(&view.ren, &rect, Util::colour_green());
		}
		Util::render_text(&view.ren, font_carbon_12, rect.x, rect.y, font_carbon_12_colour, "%s", (*it)->text.c_str());
	}

	// render the card objects
	SDL_Rect pane = { this->x, this->y, this->w, this->h };
	for(int i = 0; i < (int)_visible_cards.size(); ++i){
		_visible_cards[i]->render(view.ren, font_carbon_12, font_carbon_12_colour, pane);
	}
	for(int i = 0; i < (int)_hidden_cards.size(); ++i){
		_hidden_cards[i]->render(view.ren, font_carbon_12, font_carbon_12_colour, pane);		
	}

	// specially render the selected card.	
	if(_selected_card != nullptr){
		_card_preview.set_card(_selected_card->card);
		_card_preview.render(view.ren, font_carbon_12, font_carbon_12_colour, pane);

		rect = {
			_selected_card->x + 5 + pane.x,
			_selected_card->y + 5 + pane.y,
			_selected_card->w - 10,
			_selected_card->h - 10,
		};
		SDL_Color pale_green = { 120, 60, 120, 120 };
		Util::render_fill_rectangle(&view.ren, &rect, pale_green);
	}

	SDL_RenderSetClipRect(&view.ren, &old_clip);
}

dev_cards_t* View_Game_DevCard_Dialog::get_chosen_card(){	
	View_Game_DevCard_Card* clicked_card = get_selected_card();
	if(clicked_card == nullptr){ return nullptr; }
	return clicked_card->card;	
}

View_Game_DevCard_Card* View_Game_DevCard_Dialog::get_selected_card(){
	std::list<CheckBox*> selected = _card_groups.get_selected();
	if(selected.empty()) { return nullptr; }
	return (View_Game_DevCard_Card*)selected.front();
}


void View_Game_DevCard_Dialog::initialize_cards(){
	// add all the cards to their appropriate arrays.	
	_selected_card = nullptr;
	const  dev_cards_t* card;
	View_Game_DevCard_Card* card_sprite;	

	for(int i = 0; i < (int)_player->dev_cards.size(); ++i){
		card = _player->dev_cards[i];
		card_sprite = get_available_card();
		card_sprite->set_card(card);

		if(card->visible){
			_visible_cards.push_back(card_sprite);
		} else{ _hidden_cards.push_back(card_sprite); }
	}

	// NOTE!, the call to init() removes the checkbox from its group.
	// position the visible cards.
	int card_w = 30;
	int card_h = 50;
	int padding = 5;
	int x_offset = padding;
	int y_offset = this->h/2 + padding;
	for(int i = 0; i < (int)_visible_cards.size(); ++i){
		_visible_cards[i]->init(x_offset, y_offset, 0, card_w, card_h);
		x_offset += card_w + padding;
		y_offset += 0;
		if(x_offset + card_w >= this->w){
			if(y_offset + card_h < this->h / 2){
				x_offset = 5;
				y_offset += card_h + padding;
			}
		}

		// add them to the checkbox group
		_card_groups.add_checkbox(_visible_cards[i]);
	}

	// position the hidden cards
	//x_offset = 5;
	//y_offset = this->h / 2 + padding;
	x_offset += 0; // continue after.
	y_offset += 0;
	for(int i = 0; i < (int)_hidden_cards.size(); ++i){
		_hidden_cards[i]->init(x_offset, y_offset, 0, card_w, card_h);
		x_offset += card_w + padding;
		y_offset += 0;
		if(x_offset + card_w >= this->w){
			x_offset = 5;
			y_offset += card_h + padding;
		}

		// add them to the checkbox group
		_card_groups.add_checkbox(_hidden_cards[i]);
	}
}


// --------------------------------------------------------------
// F L Y W E I G H T     P A T T E R N
// --------------------------------------------------------------
View_Game_DevCard_Card* View_Game_DevCard_Dialog::get_available_card(){
	if(_cards_available.empty()){
		View_Game_DevCard_Card newcard;
		_card_store.push_back(newcard);
		_cards_available.push_back(&_card_store.back());
	}

	View_Game_DevCard_Card* card = _cards_available.front();
	_cards_available.erase(_cards_available.begin());
	return card;
}

void View_Game_DevCard_Dialog::return_cards_to_available(std::vector<View_Game_DevCard_Card*>& deck){
	// return all the used cards back to the available cards store.
	for(int i = 0; i < (int)deck.size(); ++i){
		deck[i]->card = nullptr;
		_cards_available.push_back(deck[i]);
	}	
	deck.clear();
}