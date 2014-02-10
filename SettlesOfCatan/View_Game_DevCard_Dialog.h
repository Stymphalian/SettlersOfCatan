#pragma once
#include <vector>
#include "IDialog.h"
#include "Collision.h"
#include "Button.h"
#include "CheckBox.h"
class devs_card_t;
class View_Game;
class Player;

// -- -- -- -- -- -- -- -- - -- -- -- -- --- -- -- - -- - -- - - 
// V I E W _ G A M E _ D E V C A R D _ D I A L O G _ B U T T O N
// -- -- -- -- -- -- -- -- - -- -- -- -- --- -- -- - -- - -- - - 
class View_Game_DevCard_Dialog;
class View_Game_DevCard_Dialog_Button : public Button{
	typedef void (*button_action) (View_Game_DevCard_Dialog& dialog, Model& model);
public:
	View_Game_DevCard_Dialog_Button();
	virtual ~View_Game_DevCard_Dialog_Button();

	void set_action(button_action baction);
	void unset_action();
	void action(View_Game_DevCard_Dialog& dialog, Model& model);

	static void play_card_button_action(View_Game_DevCard_Dialog& dialog, Model& model);
	static void play_card_button_action2(View_Game_DevCard_Dialog& dialog, Model& model);
	static void cancel_button_action(View_Game_DevCard_Dialog& dialog, Model& model);
private:
	button_action baction;	
};

// -- -- -- -- -- -- -- -- -- -- -- --  -- -- -- -- - - -- -- --
// V I E W _ G A  M E _ D E V C A R D _ C A R D
// -- -- -- -- -- -- -- -- -- -- -- --  -- -- -- -- - - -- -- --
class View_Game_DevCard_Card : public CheckBox{
public:
	//variables	
	dev_cards_t* card;	
	// constructor and destructor
	View_Game_DevCard_Card();
	virtual ~View_Game_DevCard_Card();
	void render(SDL_Renderer& ren,TTF_Font* font,SDL_Color font_colour,SDL_Rect pane);
	void set_card(const dev_cards_t* card);

	void set_data(void* data);	
private:
	void on_tick_action();
	void off_tick_action();
};

// -- -- -- -- -- -- -- -- -- -- -- --  -- -- -- -- - - -- -- --
// V I E W _ G A  M E _ D E V C A R D _ C A R D _ P R E V I E W
// -- -- -- -- -- -- -- -- -- -- -- --  -- -- -- -- - - -- -- --
class View_Game_DevCard_Card_Preview{
public:
	int x, y, z, w, h;
	//Collision hitbox;
	dev_cards_t* card;

	void set_card(const dev_cards_t* card);
	void init(int x, int y, int z, int w, int h);
	void render(SDL_Renderer& ren, TTF_Font* font, SDL_Color font_colour, SDL_Rect pane);
};


// -- -- -- -- -- -- -- -- - -- -- -- -- --- -- -- - -- - -- - - 
// V I E W _ G A M E _ D E V C A R D _ D I A L O G _ S U B 
// -- -- -- -- -- -- -- -- - -- -- -- -- --- -- -- - -- - -- - - 
class View_Game_DevCard_Dialog_Panel {
public:
	View_Game_DevCard_Dialog& dialog;
	View_Game_DevCard_Dialog_Panel(View_Game_DevCard_Dialog& dialog);

private:

};


// -- -- -- -- -- -- -- -- - -- -- -- -- --- -- -- - -- - -- - - 
// V I E W _ G A M E _ D E V C A R D _ D I A L O G
// -- -- -- -- -- -- -- -- - -- -- -- -- --- -- -- - -- - -- - - 
class View_Game_DevCard_Dialog : public IDialog
{
public:
	//variables
	View_Game& _view_game;
	Model* _model;
	Player* _player;
	//constructor and destructor
	View_Game_DevCard_Dialog(View_Game& view,int x, int y, int z, int w, int h);
	virtual ~View_Game_DevCard_Dialog();

	// implemented for IDialog
	bool open(void* data);
	void* close();	
	void handle_keyboard_events(SDL_Event& ev);
	void handle_mouse_events(SDL_Event& ev);
	void update(SDL_Event& ev);
	void render();
	
	// methods
	dev_cards_t* get_chosen_card();
private:		
	friend class View_Game_DevCard_Dialog_Panel;
	// resources
	TTF_Font* font_carbon_12;
	SDL_Color font_carbon_12_colour;
	// card objects
	View_Game_DevCard_Card* _selected_card;	
	std::vector<View_Game_DevCard_Card*> _visible_cards;
	std::vector<View_Game_DevCard_Card*> _hidden_cards;
	CheckBox_Group _card_groups;
	View_Game_DevCard_Card_Preview _card_preview;
	// buttons	
	View_Game_DevCard_Dialog_Button cancel_button;
	View_Game_DevCard_Dialog_Button play_card_button;
	View_Game_DevCard_Dialog_Button* _selected_button;
	View_Game_DevCard_Dialog_Button* _clicked_button;
	std::vector<View_Game_DevCard_Dialog_Button*> button_list;

	// methods
	View_Game_DevCard_Card* get_selected_card();
	void initialize_cards();	

	//  FLYWEIGHT PATTERN for reusing dev cards objects	
	std::list<View_Game_DevCard_Card*> _cards_available;
	std::list<View_Game_DevCard_Card> _card_store;
	View_Game_DevCard_Card* get_available_card();
	void return_cards_to_available(std::vector<View_Game_DevCard_Card*>& deck);	
};



