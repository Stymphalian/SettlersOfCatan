#include "Logger.h"

#include "View_Game.h"
#include "View_Game_Model_State.h"
#include "model_structs.h"


// - - - - - - - - - -- - - - - - - - - - - 
// V I E W _ G A M E _ M O D E L _ S T A T E
// - - - - - - - - - -- - - - - - - - - - - 

View_Game_Model_State::View_Game_Model_State(){
	this->context = nullptr;
	this->type = 0;
	this->mouse_button_down_action = nullptr;
	this->mouse_button_up_action = nullptr;
	this->mouse_motion_action = nullptr;
	this->keydown_action = nullptr;
	this->keyup_action = nullptr;	
}
View_Game_Model_State::~View_Game_Model_State(){
	this->context = nullptr;
	this->type =0;
	this->mouse_button_down_action = nullptr;
	this->mouse_button_up_action = nullptr;
	this->mouse_motion_action = nullptr;
	this->keydown_action = nullptr;
	this->keyup_action = nullptr;	
}

void View_Game_Model_State::init(View_Game_Model_State_Context* context, int type){
	this->type = type;
	this->context = context;
}

void View_Game_Model_State::set_mouse_button_down(mouse_action maction){ mouse_button_down_action = maction; }
void View_Game_Model_State::set_mouse_button_up(mouse_action maction){ mouse_button_up_action = maction; }
void View_Game_Model_State::set_mouse_motion(mouse_action maction){ mouse_motion_action = maction; }
void View_Game_Model_State::set_keydown(keyboard_action kaction){ keydown_action = kaction; }
void View_Game_Model_State::set_keyup(keyboard_action kaction){ keyup_action = kaction; }

void View_Game_Model_State::handle_mouse_button_down(SDL_Event& ev){
	if(mouse_button_down_action != nullptr){
		mouse_button_down_action(*this, ev);
	}
}
void View_Game_Model_State::handle_mouse_button_up(SDL_Event& ev){ 
	if(mouse_button_up_action != nullptr){
		mouse_button_up_action(*this, ev);
	}
}
void View_Game_Model_State::handle_mouse_motion(SDL_Event& ev){
	if(mouse_motion_action != nullptr){
		mouse_motion_action(*this, ev);
	}
}
void View_Game_Model_State::handle_keydown(SDL_Event& ev){
	if(keydown_action != nullptr){
		keydown_action(*this, ev);
	}
}
void View_Game_Model_State::handle_keyup(SDL_Event& ev){
	if(keyup_action != nullptr){
		keyup_action(*this, ev);
	}
}




// - - - - - - - - - -- - - - - - - - - - - - - - - - - -  - -
// V I E W _ G A M E _ M O D E L _ S T A T E _ C O N T E X T
// - - - - - - - - - -- - - - - - - - - - - - - - - - - -  - -
View_Game_Model_State_Context::View_Game_Model_State_Context(View_Game& view)
:view(view)
{
	state_stack.clear();
	states[model_state_e::NONE].init(this, model_state_e::NONE);
	states[model_state_e::START].init(this, model_state_e::START);
	states[model_state_e::PLACE_SETTLEMENT_1].init(this, model_state_e::PLACE_SETTLEMENT_1);
	states[model_state_e::PLACE_ROAD_1].init(this, model_state_e::PLACE_ROAD_1);
	states[model_state_e::PLACE_SETTLEMENT_2].init(this, model_state_e::PLACE_SETTLEMENT_2);
	states[model_state_e::PLACE_ROAD_2].init(this, model_state_e::PLACE_ROAD_2);
	states[model_state_e::START_RESOURCES].init(this, model_state_e::START_RESOURCES);
	states[model_state_e::NORMAL].init(this, model_state_e::NORMAL);
	states[model_state_e::TRADING].init(this, model_state_e::TRADING);
	states[model_state_e::BUILD_SETTLEMENT].init(this, model_state_e::BUILD_SETTLEMENT);
	states[model_state_e::BUILD_CITY].init(this, model_state_e::BUILD_CITY);
	states[model_state_e::BUILD_ROAD].init(this, model_state_e::BUILD_ROAD);
	states[model_state_e::PLACE_THIEF].init(this, model_state_e::PLACE_THIEF);
	states[model_state_e::STEAL_RESOURCE].init(this, model_state_e::STEAL_RESOURCE);
	states[model_state_e::PLAY_DEV_CARD].init(this, model_state_e::PLAY_DEV_CARD);

	states[model_state_e::NONE].set_mouse_button_down(mouse_buttondown_action_NONE);
	states[model_state_e::START].set_mouse_button_down(mouse_buttondown_action_START);
	states[model_state_e::PLACE_SETTLEMENT_1].set_mouse_button_down(mouse_buttondown_action_PLACE_SETTLEMENT_1);
	states[model_state_e::PLACE_ROAD_1].set_mouse_button_down(mouse_buttondown_action_PLACE_ROAD_1);
	states[model_state_e::PLACE_SETTLEMENT_2].set_mouse_button_down(mouse_buttondown_action_PLACE_SETTLEMENT_2);
	states[model_state_e::PLACE_ROAD_2].set_mouse_button_down(mouse_buttondown_action_PLACE_ROAD_2);
	states[model_state_e::START_RESOURCES].set_mouse_button_down(mouse_buttondown_action_START_RESOURCES);
	states[model_state_e::NORMAL].set_mouse_button_down(mouse_buttondown_action_NORMAL);
	states[model_state_e::TRADING].set_mouse_button_down(mouse_buttondown_action_TRADING);
	states[model_state_e::BUILD_SETTLEMENT].set_mouse_button_down(mouse_buttondown_action_BUILD_SETTLEMENT);
	states[model_state_e::BUILD_CITY].set_mouse_button_down(mouse_buttondown_action_BUILD_CITY);
	states[model_state_e::BUILD_ROAD].set_mouse_button_down(mouse_buttondown_action_BUILD_ROAD);
	states[model_state_e::PLACE_THIEF].set_mouse_button_down(mouse_buttondown_action_PLACE_THIEF);
	states[model_state_e::STEAL_RESOURCE].set_mouse_button_down(mouse_buttondown_action_STEAL_RESOURCE);
	states[model_state_e::PLAY_DEV_CARD].set_mouse_button_down(mouse_buttondown_action_PLAY_DEV_CARD);

	states[model_state_e::BUILD_SETTLEMENT].set_keydown(keydown_action_CANCEL);
	states[model_state_e::BUILD_CITY].set_keydown(keydown_action_CANCEL);
	states[model_state_e::BUILD_ROAD].set_keydown(keydown_action_CANCEL);
}
View_Game_Model_State_Context::~View_Game_Model_State_Context(){
	state_stack.clear();	
}
View_Game_Model_State* View_Game_Model_State_Context::current_state(){
	if(state_stack.empty() || state_stack.back() == nullptr){
		return &obtain_state(model_state_e::NONE);
	} else{
		return state_stack.back();
	}
}

View_Game_Model_State& View_Game_Model_State_Context::obtain_state(model_state_e type){
	return states[type];
}

void View_Game_Model_State_Context::push_state(View_Game_Model_State& state){
	Logger::getLog().log(Logger::DEBUG, "View_Game_Model_State_Context::push_state(%d) %d remaining", state.type,state_stack.size()+1);
	state_stack.push_back(&state);
}
void View_Game_Model_State_Context::pop_state(){
	if(state_stack.empty() == false){
		Logger::getLog().log(Logger::DEBUG, "View_Game_Model_State_Context::pop_state(%d) %d remaining",state_stack.back()->type,state_stack.size() -1);
		state_stack.pop_back();
	}
}

void View_Game_Model_State_Context::mouse_buttondown_action_NONE(View_Game_Model_State& self,SDL_Event& ev){
	// do nothing	
}
void View_Game_Model_State_Context::mouse_buttondown_action_START(View_Game_Model_State& self,SDL_Event& ev){
	Logger::getLog().log(Logger::DEBUG, "View_Game_Model_State_Context::mouse_buttondown_action_START()");
	View_Game_Model_State_Context* context = self.context; 
	Model& model = self.context->view.model;
	context->pop_state(); // pop off the current state.
	
	// kick off the starting placement of settlements and roads.
	context->push_state(context->obtain_state(model_state_e::NORMAL));
	context->push_state(context->obtain_state(model_state_e::START_RESOURCES));	

	for(int i = 0; i < model.get_num_players(); ++i){
		context->push_state(context->obtain_state(model_state_e::PLACE_ROAD_2));
		context->push_state(context->obtain_state(model_state_e::PLACE_SETTLEMENT_2));
	}
	for(int i = 0; i < model.get_num_players(); ++i){
		context->push_state(context->obtain_state(model_state_e::PLACE_ROAD_1));
		context->push_state(context->obtain_state(model_state_e::PLACE_SETTLEMENT_1));		
	}	
	
}
void View_Game_Model_State_Context::mouse_buttondown_action_PLACE_SETTLEMENT_1(View_Game_Model_State& self,SDL_Event& ev){
	vertex_face_t_intersect* selected_vertex = self.context->view.selected_vertex;
	Model& model = self.context->view.model;

	if(selected_vertex != nullptr){
		//Logger::getLog().log(Logger::DEBUG, "View_Game_Model_State_Context::mouse_buttondown_action_PLACE_SETTLEMENT");
		bool rs = model.build_building(
			building_t::SETTLEMENT,
			selected_vertex->num,
			model.get_current_player());
		if(rs){
			self.context->pop_state();			
		}
	}

}
void View_Game_Model_State_Context::mouse_buttondown_action_PLACE_ROAD_1(View_Game_Model_State& self,SDL_Event& ev){
	vertex_face_t_intersect* selected_face = self.context->view.selected_face;
	Model& model = self.context->view.model;
	View_Game_Model_State_Context* context = self.context;

	//Logger::getLog().log(Logger::DEBUG, "View_Game_Model_State_Context:mouse_buttondown_action_PLACE_ROAD_1 Place Roads 1 for player %d", model.get_current_player());	
	if(selected_face != nullptr){
		bool rs = model.build_building(
			building_t::ROAD,
			selected_face->num,
			model.get_current_player());
		if(rs){
			context->pop_state();
			if(model.get_current_player() != model.get_num_players() - 1){
				if(model.get_num_players() != 0){
					model.set_current_player((model.get_current_player() + 1) % model.get_num_players());
				}
			}			
		} // end if(rs)				
	}
}
void View_Game_Model_State_Context::mouse_buttondown_action_PLACE_SETTLEMENT_2(View_Game_Model_State& self,SDL_Event& ev){
	vertex_face_t_intersect* selected_vertex = self.context->view.selected_vertex;
	Model& model = self.context->view.model;
	View_Game_Model_State_Context* context = self.context;

	//Logger::getLog().log(Logger::DEBUG, "View_Game_Model_State_Context::mouse_buttondown_action_PLACE_SETTLEMENT_2 for player %d", model.get_current_player());
	if(selected_vertex != nullptr){
		bool rs = model.build_building(
			building_t::SETTLEMENT,
			selected_vertex->num,
			model.get_current_player());
		if(rs) { 
			context->pop_state();			
		}
	}

}
void View_Game_Model_State_Context::mouse_buttondown_action_PLACE_ROAD_2(View_Game_Model_State& self,SDL_Event& ev){
	vertex_face_t_intersect* selected_face = self.context->view.selected_face;
	Model& model = self.context->view.model;
	View_Game_Model_State_Context* context = self.context;

	//Logger::getLog().log(Logger::DEBUG, "View_Game_Model_State_Contxt::mouse_buttondown_action_PLACE_ROAD_2 for player %d", model.get_current_player());
	static int counter = 0;
	if(selected_face != nullptr){
		bool rs = model.build_building(
			building_t::ROAD,
			selected_face->num,
			model.get_current_player());
		if(rs) {
			context->pop_state();
			model.set_current_player(model.get_current_player() - 1);
			if(model.get_current_player() < 0){
				model.set_current_player(0);
			}			
		}// end if rs()

	}

}
void View_Game_Model_State_Context::mouse_buttondown_action_START_RESOURCES(View_Game_Model_State& self,SDL_Event& ev){
	Model& model = self.context->view.model;
	View_Game_Model_State_Context* context = self.context;

//	Logger::getLog().log(Logger::DEBUG, "View_Game_Model_State_Context::mouse_buttondown_action_START_RESOURCES() Start Resources and Placing the Thief");
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
				placed_thief = true;
				break;
			}
		}
		if(placed_thief == true){ break; }
	}
	context->pop_state();	
}
void View_Game_Model_State_Context::mouse_buttondown_action_NORMAL(View_Game_Model_State& self,SDL_Event& ev){
	// do nothing.
}
void View_Game_Model_State_Context::mouse_buttondown_action_TRADING(View_Game_Model_State& self,SDL_Event& ev){	
	View_Game_Model_State_Context* context = self.context;
	//Logger::getLog().log(Logger::DEBUG, "View_Game_Model_State_Context::mouse_buttondown_action_TRADING");
	context->pop_state();
}
void View_Game_Model_State_Context::mouse_buttondown_action_BUILD_SETTLEMENT(View_Game_Model_State& self,SDL_Event& ev){
	vertex_face_t_intersect* selected_vertex = self.context->view.selected_vertex;
	Model& model = self.context->view.model;
	View_Game_Model_State_Context* context = self.context;

	//Logger::getLog().log(Logger::DEBUG, "View_Game_Model_State_Context::mouse_buttondown_action_BUILD_SETTLEMENT()");
	if(selected_vertex != nullptr){
		bool rs = model.build_building(
			building_t::SETTLEMENT,
			selected_vertex->num,
			model.get_current_player());
		if(rs) {
			context->pop_state();
		}
	}

}
void View_Game_Model_State_Context::mouse_buttondown_action_BUILD_CITY(View_Game_Model_State& self,SDL_Event& ev){
	vertex_face_t_intersect* selected_vertex = self.context->view.selected_vertex;
	Model& model = self.context->view.model;
	View_Game_Model_State_Context* context = self.context;

	//Logger::getLog().log(Logger::DEBUG, "View_Game_Model_State_Context::mouse_buttondown_action_BUILD_CITY()");
	if(selected_vertex != nullptr){
		bool rs = model.build_building(
			building_t::CITY,
			selected_vertex->num,
			model.get_current_player());
		if(rs) {
			context->pop_state();
		}
	}

}

void View_Game_Model_State_Context::mouse_buttondown_action_BUILD_ROAD(View_Game_Model_State& self,SDL_Event& ev){
	vertex_face_t_intersect* selected_face = self.context->view.selected_face;
	Model& model = self.context->view.model;
	View_Game_Model_State_Context* context = self.context;

	//Logger::getLog().log(Logger::DEBUG, "View_Game_Model_State_Context::mouse_buttondown_action_BUILD_ROAD()");
	if(selected_face != nullptr){
		bool rs = model.build_building(
			building_t::ROAD,
			selected_face->num,
			model.get_current_player());
		if(rs) {
			context->pop_state();
		}
	}

}
void View_Game_Model_State_Context::mouse_buttondown_action_PLACE_THIEF(View_Game_Model_State& self,SDL_Event& ev){
	Tile_intersect* selected_tile = self.context->view.selected_tile;
	vertex_face_t_intersect* selected_vertex = self.context->view.selected_vertex;
	vertex_face_t_intersect* selected_face = self.context->view.selected_vertex;
	Model& model = self.context->view.model;
	View_Game_Model_State_Context* context = self.context;

	//Logger::getLog().log(Logger::DEBUG, "View_Game_Model_State_Context::mouse_buttodown_action_PLACE_THIEF()");
	if(selected_tile != nullptr  &&
		selected_vertex == nullptr  && 
		selected_face == nullptr)
	{
		// condition, that we can't place the thief on the same spot
		if(selected_tile->col == model.get_thief_x() && selected_tile->row == model.get_thief_y()){
			return;
		}

		bool rs = model.place_thief(selected_tile->col, selected_tile->row);
		if(rs){
			context->pop_state();			

			// check for the case in which there are no valid choices for stealing/
			// if none exists then we don't want to transition 
			// into the stealing resources state
			bool exists_valid_choices = false;
			Tiles* thief_tile = model.get_tile(model.get_thief_x(), model.get_thief_y());
			for(int i = 0; i < 6; ++i){
				vertex_face_t* model_vertex = model.get_vertex(thief_tile->vertices[i]);
				if(model_vertex == nullptr){ continue; }
				if(model_vertex->type != vertex_face_t::NONE &&
					model_vertex->player != -1 &&
					model_vertex->player != model.get_current_player())
				{
					exists_valid_choices = true;
					break;
				}
			}
			if(exists_valid_choices == false){
				Logger::getLog().log(Logger::DEBUG, "View_Game_Model_State_Context::mouse_buttondown_action_PLACE_THIEF There does not exist a valid stealing choice. This is acceptable");				
			} else{
				context->push_state(context->obtain_state(model_state_e::STEAL_RESOURCE));
			}
		}
	}
}
void View_Game_Model_State_Context::mouse_buttondown_action_STEAL_RESOURCE(View_Game_Model_State& self,SDL_Event& ev){
	vertex_face_t_intersect* selected_vertex = self.context->view.selected_vertex;
	Model& model = self.context->view.model;
	View_Game_Model_State_Context* context = self.context;
	//Logger::getLog().log(Logger::DEBUG, "View_Game_Model_State_Context::mouse_buttondown_action_STEAL_RESOURCE()");
		
	if(selected_vertex != nullptr){
		vertex_face_t* model_vertex = model.get_vertex(selected_vertex->num);
		if(model_vertex == nullptr){ return; }


		// make sure that the selected vertex is one of the
		// vertices surrounding the theif tile.
		Tiles* thief_tile = model.get_tile(model.get_thief_x(), model.get_thief_y());
		if(thief_tile == nullptr){ return; }
		bool good = false;		
		for(int i = 0; i < 6; ++i){
			if(thief_tile->vertices[i] == selected_vertex->num){
				good = true;
				break;
			}
		}
		if(good == false){ return; }
		Logger::getLog().log(Logger::DEBUG, "View_Game_Model_State_Context tile col=%d,row=%d with vertex %d",
			model.get_thief_x(), model.get_thief_y(), selected_vertex->num);					
		

		// get the target and receiving players.
		int target_player = model.get_current_player();
		int receiving_player = model.get_current_player();
		if(model_vertex->type != vertex_face_t::NONE &&
			model_vertex->player != -1 &&
			model_vertex->player != receiving_player)
		{
			target_player = model_vertex->player;
		} else{
			// get out because it is an invalid vertex choice.
			return;
		}
		Logger::getLog().log(Logger::DEBUG, "View_Game_Model_State_Context target player=%d receiving player=%d",
			target_player, receiving_player);
	
		// steal the resources from the target player and give it to the current player		
		bool rs = model.steal_random_resource_from_player(target_player, receiving_player);
		if(rs == false){
			Logger::getLog().log(Logger::DEBUG, "View_Game_Modle_State_Context::mouse_butttondown_action_STEAL_RESOURCE failed to teal resource");
		}
		context->pop_state();
	}

}

void View_Game_Model_State_Context::mouse_buttondown_action_PLAY_DEV_CARD(View_Game_Model_State& self,SDL_Event& ev){
	vertex_face_t_intersect* selected_face = self.context->view.selected_face;
	Model& model = self.context->view.model;
	View_Game_Model_State_Context* context = self.context;

	//Logger::getLog().log(Logger::DEBUG, "View_Game_Model_State_Context::mouse_buttondown_action_PLAY_DEV_CARD");
	context->pop_state();
}


void View_Game_Model_State_Context::keydown_action_CANCEL(View_Game_Model_State& self, SDL_Event& ev){
	View_Game_Model_State_Context* context = self.context;
	//Logger::getLog().log(Logger::DEBUG, "View_Game_Model_State_Context::keydown_action_CANCEL");
	if(ev.key.keysym.scancode == SDL_SCANCODE_Q){
		context->pop_state();
	}	
}
void View_Game_Model_State_Context::mouse_action_empty_action(View_Game_Model_State& self,SDL_Event& ev){}
void View_Game_Model_State_Context::keyboard_action_empty_action(View_Game_Model_State& self,SDL_Event& ev){}