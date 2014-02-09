#pragma once
#include <SDL.h>
#include <vector>
class View_Game;


// Abstart Base Class for the Model State
class View_Game_Model_State_Context;

// -----------------------------------------------------------------
// V I E W _ G A M E _ M O D E L _ S T A T E 
// -----------------------------------------------------------------
class View_Game_Model_State{
	typedef void(*mouse_action)(View_Game_Model_State& self,SDL_Event& ev);
	typedef void(*keyboard_action)(View_Game_Model_State& self, SDL_Event& ev);
public:
	View_Game_Model_State_Context* context;
	mouse_action mouse_button_down_action;
	mouse_action mouse_button_up_action;
	mouse_action mouse_motion_action;
	keyboard_action keydown_action;
	keyboard_action keyup_action;
	int type;

	View_Game_Model_State();
	virtual ~View_Game_Model_State();
	void init(View_Game_Model_State_Context* context,int type);
	void set_mouse_button_down(mouse_action maction);
	void set_mouse_button_up(mouse_action maction);
	void set_mouse_motion(mouse_action maction);
	void set_keydown(mouse_action maction);
	void set_keyup(mouse_action maction);

	virtual void handle_mouse_button_down(SDL_Event& ev);
	virtual void handle_mouse_button_up(SDL_Event& ev);
	virtual void handle_mouse_motion(SDL_Event& ev);
	virtual void handle_keydown(SDL_Event& ev);
	virtual void handle_keyup(SDL_Event& ev);
};


// -----------------------------------------------------------------
// V I E W _ G A M E _ M O D E L _ S T A T E _ C O N T E X T
// -----------------------------------------------------------------
class View_Game_Model_State_Context{
public:
	enum model_state_e {
		NONE=0,
		START,
		PLACE_SETTLEMENT_1,
		PLACE_ROAD_1,
		PLACE_SETTLEMENT_2,
		PLACE_ROAD_2,
		START_RESOURCES,
		NORMAL,
		TRADING,
		BUILD_SETTLEMENT,
		BUILD_CITY,
		BUILD_ROAD,
		PLACE_THIEF,
		STEAL_RESOURCE,
		PLAY_DEV_CARD,
		NUM_OF_model_state_e
	};

	// variables
	View_Game& view;	
	std::vector<View_Game_Model_State*> state_stack;	
	//methods
	View_Game_Model_State_Context(View_Game& view);
	virtual ~View_Game_Model_State_Context();
	View_Game_Model_State* current_state();
	View_Game_Model_State& obtain_state(model_state_e);
	void push_state(View_Game_Model_State& state);
	void pop_state();
	

private:
	View_Game_Model_State states[model_state_e::NUM_OF_model_state_e];
	static void mouse_buttondown_action_NONE(View_Game_Model_State& self,SDL_Event& ev);
	static void mouse_buttondown_action_START(View_Game_Model_State& self,SDL_Event& ev);
	static void mouse_buttondown_action_PLACE_SETTLEMENT_1(View_Game_Model_State& self,SDL_Event& ev);
	static void mouse_buttondown_action_PLACE_ROAD_1(View_Game_Model_State& self,SDL_Event& ev);
	static void mouse_buttondown_action_PLACE_SETTLEMENT_2(View_Game_Model_State& self,SDL_Event& ev);
	static void mouse_buttondown_action_PLACE_ROAD_2(View_Game_Model_State& self,SDL_Event& ev);
	static void mouse_buttondown_action_START_RESOURCES(View_Game_Model_State& self,SDL_Event& ev);
	static void mouse_buttondown_action_NORMAL(View_Game_Model_State& self,SDL_Event& ev);
	static void mouse_buttondown_action_TRADING(View_Game_Model_State& self,SDL_Event& ev);
	static void mouse_buttondown_action_BUILD_SETTLEMENT(View_Game_Model_State& self,SDL_Event& ev);
	static void mouse_buttondown_action_BUILD_CITY(View_Game_Model_State& self,SDL_Event& ev);
	static void mouse_buttondown_action_BUILD_ROAD(View_Game_Model_State& self,SDL_Event& ev);
	static void mouse_buttondown_action_PLACE_THIEF(View_Game_Model_State& self,SDL_Event& ev);
	static void mouse_buttondown_action_STEAL_RESOURCE(View_Game_Model_State& self,SDL_Event& ev);
	static void mouse_buttondown_action_PLAY_DEV_CARD(View_Game_Model_State& self,SDL_Event& ev);	

	static void keydown_action_CANCEL(View_Game_Model_State& self, SDL_Event& ev);

	static void mouse_action_empty_action(View_Game_Model_State& self, SDL_Event& ev);
	static void keyboard_action_empty_action(View_Game_Model_State& self, SDL_Event& ev);
};
/*
Logger& logger = Logger::getLog();

if(ev.type == SDL_MOUSEBUTTONDOWN){
	if(s == View_Game::NONE)
	{
		//do nothing
		//logger.log(Logger::DEBUG, "View_Game::handle_mouse_given_state(%d) None State", (int)s);
	} else if(s == View_Game::START)
	{
		static bool once = false;
		if(once == false){
			//once = true;
			once = false;
			logger.log(Logger::DEBUG, "View_Game::handle_mouse_given_state(%d) Start", (int)s);
			// do roll assignment. for now we just assume we start
			// from player one and then keep going.
			set_state(View_Game::PLACE_SETTLEMENT_1);
		} else{
			set_state(View_Game::NORMAL);
		}
	} else if(s == View_Game::PLACE_SETTLEMENT_1)
	{
		logger.log(Logger::DEBUG, "View_Game::handle_mouse_given_state(%d) Place Settlement 1 for player %d", (int)s, model.get_current_player());
		if(selected_vertex != nullptr){
			bool rs = model.build_building(
				building_t::SETTLEMENT,
				selected_vertex->num,
				model.get_current_player());
			if(rs){
				set_state(View_Game::PLACE_ROAD_1);
			}
		}
	} else if(s == View_Game::PLACE_ROAD_1)
	{
		logger.log(Logger::DEBUG, "View_Game::handle_mouse_given_state(%d) Place Roads 1 for player %d", (int)s, model.get_current_player());
		static int counter = 0;
		if(selected_face != nullptr){
			bool rs = model.build_building(
				building_t::ROAD,
				selected_face->num,
				model.get_current_player());
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
					if(model.get_num_players() != 0){
						model.set_current_player((model.get_current_player() + 1) % model.get_num_players());
					}
					//model.m_current_player = (model.m_current_player + 1) % model.m_num_players;
				}
			} // end if(rs)				
		}
	} else if(s == View_Game::PLACE_SETTLEMENT_2)
	{
		logger.log(Logger::DEBUG, "View_Game::handle_mouse_given_state(%d) Place Settlement 2 for player %d", (int)s, model.get_current_player());
		if(selected_vertex != nullptr){
			bool rs = model.build_building(
				building_t::SETTLEMENT,
				selected_vertex->num,
				model.get_current_player());
			if(rs) { set_state(View_Game::PLACE_ROAD_2); }
		}
	} else if(s == View_Game::PLACE_ROAD_2)
	{
		logger.log(Logger::DEBUG, "View_Game::handle_mouse_given_state(%d) Place Road 2 for player %d", (int)s, model.get_current_player());
		static int counter = 0;
		if(selected_face != nullptr){
			bool rs = model.build_building(
				building_t::ROAD,
				selected_face->num,
				model.get_current_player());
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
	} else if(s == View_Game::START_RESOURCES)
	{
		logger.log(Logger::DEBUG, "View_Game::handle_mouse_given_state(%d) Start Resources and Placing the Thief", (int)s);
		// give resources.
		for(int i = model.get_num_dice(); i < model.get_num_dice()*model.get_num_dice_sides(); ++i){
			model.give_resources_from_roll(i);
		}

		// place the thief on a desert tile.
		bool placed_thief = false;
		for(int row = 0; row < model.get_board_width(); ++row){
			for(int col = 0; col < model.get_board_height(); ++col){
				if(model.get_tile(col, row)->active == 0){ continue; }
				if(model.get_tile(col, row)->type == Tiles::DESERT_TILE){
					model.place_thief(col, row);
					set_state(View_Game::NORMAL);
					placed_thief = true;
					break;
				}
			}
			if(placed_thief == true){ break; }
		}

	} else if(s == View_Game::NORMAL)
	{
		// do nothing.
	} else if(s == View_Game::TRADING)
	{
		logger.log(Logger::DEBUG, "View_Game::handle_mouse_given_state(%d) Trading", (int)s);
		set_state(View_Game::NORMAL);
	} else if(s == View_Game::BUILD_SETTLEMENT)
	{
		logger.log(Logger::DEBUG, "View_Game::handle_mouse_given_state(%d) Building settlement", (int)s);
		if(selected_vertex != nullptr){
			bool rs = model.build_building(
				building_t::SETTLEMENT,
				selected_vertex->num,
				model.get_current_player());
			if(rs) {
				set_state(View_Game::NORMAL);
			}
		}
	} else if(s == View_Game::BUILD_CITY)
	{
		logger.log(Logger::DEBUG, "View_Game::handle_mouse_given_state(%d) Building City", (int)s);

		if(selected_vertex != nullptr){
			bool rs = model.build_building(
				building_t::CITY,
				selected_vertex->num,
				model.get_current_player());
			if(rs) {
				set_state(View_Game::NORMAL);
			}
		}
	} else if(s == View_Game::BUILD_ROAD)
	{
		logger.log(Logger::DEBUG, "View_Game::handle_mouse_given_state(%d) Building Road", (int)s);
		if(selected_face != nullptr){
			bool rs = model.build_building(
				building_t::ROAD,
				selected_face->num,
				model.get_current_player());
			if(rs) {
				set_state(View_Game::NORMAL);
			}
		}

	} else if(s == View_Game::PLAY_DEV_CARD)
	{
		logger.log(Logger::DEBUG, "View_Game::handle_mouse_given_state(%d) Playing Dev Card", (int)s);
		set_state(View_Game::NORMAL);
	}
} else  if(ev.type == SDL_MOUSEBUTTONUP){
	if(s == View_Game::NONE){} else if(s == View_Game::START){} else if(s == View_Game::PLACE_SETTLEMENT_1){} else if(s == View_Game::PLACE_ROAD_1){} else if(s == View_Game::PLACE_SETTLEMENT_2){} else if(s == View_Game::PLACE_ROAD_2){} else if(s == View_Game::START_RESOURCES){} else if(s == View_Game::NORMAL){} else if(s == View_Game::TRADING){} else if(s == View_Game::BUILD_SETTLEMENT){} else if(s == View_Game::BUILD_CITY){} else if(s == View_Game::BUILD_ROAD){} else if(s == View_Game::PLAY_DEV_CARD){}
} else if(ev.type == SDL_MOUSEMOTION){
	if(s == View_Game::NONE){} else if(s == View_Game::START){} else if(s == View_Game::PLACE_SETTLEMENT_1){} else if(s == View_Game::PLACE_ROAD_1){} else if(s == View_Game::PLACE_SETTLEMENT_2){} else if(s == View_Game::PLACE_ROAD_2){} else if(s == View_Game::START_RESOURCES){} else if(s == View_Game::NORMAL){} else if(s == View_Game::TRADING){} else if(s == View_Game::BUILD_SETTLEMENT){} else if(s == View_Game::BUILD_CITY){} else if(s == View_Game::BUILD_ROAD){} else if(s == View_Game::PLAY_DEV_CARD){}
}

if(model.get_error() != Model::MODEL_ERROR_NONE){
	message_pane.setMessage(View_Game::view_game_model_error_strings[model.get_error()].c_str());
	message_pane.reset();
}

*/