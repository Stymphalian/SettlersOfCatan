#include "model_structs.h"
#include <cstring>
#include <string>
#include "Logger.h"
#include "Util.h"

// static variables
std::string dev_cards_t::default_messages[dev_cards_t::NUM_OF_DEV_CARDS] = {
	"Move the robber. Steal 1 resource card from the owner of an adjacent settlemnt or city.",
	"1 Victory Point.",
	"When you  play this card, announce 1 type of resource. All other players must give you all their reource cards of that type.",
	"Take any 2 resources from the bank. Add them to your hand. They can be 2 of the same resource or 2 different resources.",
	"Place 2 new roads as if you had just built them."
};
std::string dev_cards_t::victory_point_titles[10] = {
	"Chapel", "Market", "Governor's House", "Academy of Catan", "Catan Victory Statue",
	"Chapel", "Market", "Governor's House", "Academy of Catan", "Catan Victory Statue"
};
std::string dev_cards_t::default_titles[dev_cards_t::NUM_OF_DEV_CARDS] = {
	"Soldier", "Victory", "Monopoly", "Year of Plenty", "Road Building"
};

// constructor and destructors
dev_cards_t::dev_cards_t(){
	type = (dev_cards_t::dev_cards_e) 0;
	_title = 0;
	_message = 0;
	visible = false;
}
dev_cards_t::~dev_cards_t(){
	//Logger::getLog("jordan.log").log(Logger::DEBUG, "dev_cards_t destructor");
}

// methods
void dev_cards_t::init(dev_cards_t::dev_cards_e type){
	this->type = type;
	_title = type;
	_message = type;
	this->visible = false;
}

std::string dev_cards_t::title(){
	if(type == dev_cards_t::VICTORY){
		return dev_cards_t::victory_point_titles[_title];
	} else{
		return dev_cards_t::default_titles[_title];
	}
}

std::string dev_cards_t::message(){
	return dev_cards_t::default_messages[_message];
}

void resource_t::zero_out(){
	memset(res, 0, sizeof(int)*NUM_OF_RESOURCES);
}

vertex_face_t::vertex_face_t(){
	type = object_e::NONE;
	player = -1;
	memset(&vert, -1, sizeof(vert));
	memset(&face, -1, sizeof(face));
	is_assigned = false;
}
vertex_face_t::~vertex_face_t(){
	//Logger::getLog("jordan.log").log(Logger::DEBUG, "vertex_face_t destructor");
}


// configuration_t
configuration_t::configuration_t(){
	num_extensions = 0;
}
configuration_t::~configuration_t(){
	Logger::getLog("jordan.log").log(Logger::DEBUG, "configuration_t destructor");
}
void configuration_t::apply_defaults(){
	Logger::getLog("jordan.log").log(Logger::DEBUG, "configuaration_t::apply_defaults() Setting values to default values");
	// set the building costs
	memset(building_costs, 0, sizeof(building_costs));
	building_costs[building_t::buildings::ROAD].res[resource_t::WOOD] = 1;
	building_costs[building_t::buildings::ROAD].res[resource_t::BRICK] = 1;
	building_costs[building_t::buildings::SETTLEMENT].res[resource_t::WOOD] = 1;
	building_costs[building_t::buildings::SETTLEMENT].res[resource_t::SHEEP] = 1;
	building_costs[building_t::buildings::SETTLEMENT].res[resource_t::BRICK] = 1;
	building_costs[building_t::buildings::SETTLEMENT].res[resource_t::WHEAT] = 1;
	building_costs[building_t::buildings::CITY].res[resource_t::WHEAT] = 2;
	building_costs[building_t::buildings::CITY].res[resource_t::ORE] = 3;
	building_costs[building_t::buildings::DEV_CARD].res[resource_t::ORE] = 1;
	building_costs[building_t::buildings::DEV_CARD].res[resource_t::SHEEP] = 1;
	building_costs[building_t::buildings::DEV_CARD].res[resource_t::WHEAT] = 1;

	// set the number of buildings that a player starts with
	memset(buildings_cap, 0, sizeof(buildings_cap));
	buildings_cap[building_t::buildings::ROAD] = 15;
	buildings_cap[building_t::buildings::SETTLEMENT] = 5;
	buildings_cap[building_t::buildings::CITY] = 4;
	buildings_cap[building_t::buildings::DEV_CARD] = 0;

	// setup the maximum number of tiles 
	// default values for the num of tiles
	memset(num_tiles, 0, sizeof(num_tiles));
	num_tiles[Tiles::SHEEP_TILE] = 4;
	num_tiles[Tiles::BRICK_TILE] = 3;
	num_tiles[Tiles::WOOD_TILE] = 4;
	num_tiles[Tiles::WHEAT_TILE] = 4;
	num_tiles[Tiles::ORE_TILE] = 3;
	num_tiles[Tiles::SHEEP_PORT] = 1;
	num_tiles[Tiles::BRICK_PORT] = 1;
	num_tiles[Tiles::WOOD_PORT] = 1;
	num_tiles[Tiles::WHEAT_PORT] = 1;
	num_tiles[Tiles::ORE_PORT] = 1;
	num_tiles[Tiles::TRADE_PORTS] = 4;
	num_tiles[Tiles::DESERT_TILE] = 1;

	// Setup the default amount of bank resources
	for(int i = 0; i < resource_t::NUM_OF_RESOURCES; ++i){
		resource_cards_cap[i] = 20;
	}

	// setup the default number of dev cards
	memset(dev_cards_cap, 0, sizeof(dev_cards_cap));
	dev_cards_cap[dev_cards_t::MONOPOLY] = 2;
	dev_cards_cap[dev_cards_t::ROAD_BUILDING] = 2;
	dev_cards_cap[dev_cards_t::YEAR_PLENTY] = 2;
	dev_cards_cap[dev_cards_t::SOLDIER] = 15;
	dev_cards_cap[dev_cards_t::VICTORY] = 4;
}
void configuration_t::apply_extensions(int num_extensions){
	Logger::getLog("jordan.log").log(Logger::DEBUG, "configuaration_t::apply_extensions(%d)",num_extensions);
	this->num_extensions = num_extensions;

	int amount = 0;
	int extra_resources = 0;
	int spares[5] = {
		Tiles::SHEEP_TILE,
		Tiles::WHEAT_TILE,
		Tiles::ORE_TILE,
		Tiles::BRICK_TILE,
		Tiles::WOOD_TILE
	};

	for(int i = 0; i < num_extensions; ++i){
		// extend the number of tiles
		if(i % 2 == 0){
			amount = (5 + i+1) + (5 + i);
		} else {
			amount = (5 + i+1);
		}
		extra_resources = amount % 5;
				
		Logger::getLog("jordan.log").log(Logger::DEBUG, "configuration_t::apply_extensions() extension %d, amount = %d, remainder = %d", i,amount,extra_resources);
		amount = amount / 5;
		num_tiles[Tiles::SHEEP_TILE] += amount;
		num_tiles[Tiles::WHEAT_TILE] += amount;
		num_tiles[Tiles::WOOD_TILE] += amount;
		num_tiles[Tiles::ORE_TILE] += amount;
		num_tiles[Tiles::BRICK_TILE] += amount;
		if(extra_resources > 0){
			Util::shuffle(spares, 5);
			for(int j = 0; j < extra_resources; ++j){
				num_tiles[spares[j]] += 1;
			}
		}
		
		if(i % 2 == 0){
			num_tiles[Tiles::TRADE_PORTS] += 2;
			num_tiles[Tiles::DESERT_TILE] += 1;
		} else {
			num_tiles[Tiles::TRADE_PORTS] += 1;
		}

		// extend the number of resources held by the bank/default
		for(int i = 0; i < resource_t::NUM_OF_RESOURCES; ++i){
			resource_cards_cap[i] += 5;
		}

		// extend the number of dev cards in the deck
		dev_cards_cap[dev_cards_t::MONOPOLY] += 1;
		dev_cards_cap[dev_cards_t::ROAD_BUILDING] += 1;
		dev_cards_cap[dev_cards_t::YEAR_PLENTY] += 1;
		dev_cards_cap[dev_cards_t::SOLDIER] += 5;
		dev_cards_cap[dev_cards_t::VICTORY] += 1;
	}
}

// FUCK THIS SHIT!
int configuration_t::num_of_extensions(int num_players){
	if(num_players <= 4){ return 0; }

	//	the ternary operation at the end is to handle the even/odd case for number of players
	//return ((num_players - 4) / 2) + ((num_players % 2 == 1) ? 1 : 0);
	int n = num_players - 4;
	int num_exts = 1;
	int state = 0;
	int count = 0;
	bool flag = false;
	for(int i = 1; i <= n; ++i){
		count++;
		if(flag){
			num_exts++;
			flag = false;
		}
		if(state == 0){
			if(count == 2){
				// wait count = 2
				state = 1;
				count = 0;
				flag = true;
			}			
		} else if( state == 1){
			if(count == 1){
				// wait count = 1
				state = 0;
				count = 0;
				flag = true;
			}			
		}
	}
	return num_exts;
}

// FUCK THIS SHIT!
int configuration_t::num_of_players_from_extension(int exts){
	if(exts <= 0){ return 4; }
	int n = 1;
	int num_exts = 0;
	int state = 0;
	int count = 0;
	bool flag = false;
	for(n = 0; n < 100 ; ++n){
		count++;
		if(flag){
			num_exts++;
			if(num_exts == exts){ break; }
			flag = false;
		}
		if(state == 0){
			if(count == 2){
				// wait count = 2
				state = 1;
				count = 0;
				flag = true;
			}
		} else if(state == 1){
			if(count == 1){
				// wait count = 1
				state = 0;
				count = 0;
				flag = true;
			}
		}
	}
	return n + 4;
}

void configuration_t::print_configuration_to_log(){
	Logger& logger = Logger::getLog("jordan.log");
	logger.log(Logger::DEBUG, "configuration_t %d", num_extensions);
	for(int i = 0; i < Tiles::NUM_OF_TILES; ++i){
		logger.log(Logger::DEBUG, "configuration_t num_tiles[%d]=%d",i,num_tiles[i]);
	}
	for(int i = 0; i< building_t::NUM_OF_BUILDINGS; ++i){
		logger.log(Logger::DEBUG, "configuration_t buildings_cap[%d]=%d", i, buildings_cap[i]);
	}
	for(int i = 0; i < dev_cards_t::NUM_OF_DEV_CARDS; ++i){
		logger.log(Logger::DEBUG, "configuration_t dev_cards_cap[%d]=%d",i, dev_cards_cap[i]);
	}
	for(int i = 0; i < resource_t::NUM_OF_RESOURCES; ++i){
		logger.log(Logger::DEBUG, "resource_cards_cap[%d]=%d", i, resource_cards_cap[i]);
	}
	for(int i = 0; i < building_t::NUM_OF_BUILDINGS; ++i){
		for(int j = 0; j < resource_t::NUM_OF_RESOURCES; ++j){
			logger.log(Logger::DEBUG, "building_costs[%d]=%d", i, building_costs[i].res[j]);
		}		
	}
}