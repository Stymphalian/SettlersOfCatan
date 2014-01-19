#include "Player.h"
#include "Logger.h"

Player::Player()
{
	this->name = "Player";
	this->color = { 255, 0, 0, 255 };
	this->hand_size = 0;	
	this->resources.zero_out();
	dev_cards.clear();
	buildings.clear();
	roads.clear();
	this->num_soldiers = 0;
	for(int i = 0; i < building_t::NUM_OF_BUILDINGS; ++i){
		this->building_cap[i] = 0;
	}
	
}

Player::~Player(){
	Logger::getLog("jordan.log").log(Logger::DEBUG, "Player destructor");
	dev_cards.clear();
	buildings.clear();
	roads.clear();
}

void Player::init(std::string name,
						SDL_Color color,
						int hand_size,
						resource_t start_resources,
						int num_soldiers,
						int start_building_cap[building_t::NUM_OF_BUILDINGS]
						)
{
	this->name = name;
	this->color = color;
	this->hand_size = hand_size;
	for(int i = 0; i < resource_t::NUM_OF_RESOURCES; ++i){
		this->resources.res[i] = start_resources.res[i];
	}
	this->num_soldiers = num_soldiers;
	for(int i = 0; i < building_t::NUM_OF_BUILDINGS; ++i){
		this->building_cap[i] = start_building_cap[i];
	}	
}

int Player::get_hand_size(){
	int size = 0;
	for(int i = 0; i < resource_t::NUM_OF_RESOURCES; ++i){
		size += resources.res[i];
	}
	hand_size = size;
	return size;
}

bool Player::add_resource(int type,int amount){
	if(resources.res[type] + amount < 0){ return false; }
	resources.res[type] += amount;
	return true;
}