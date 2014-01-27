#pragma once
#include <string>
#include <list>
#include "Collision.h"


class CheckBox_Group;

// --------------------------------------------------
// C H E C K B O X _ B O X
// --------------------------------------------------
class CheckBox{
	friend class CheckBox_Group;		
public:
	Collision hitbox;
	int x, y, z, w, h;

	CheckBox();
	virtual ~CheckBox();
	
	void init(int x, int y, int z, int w, int h,bool ticked= false);
	void click();
	bool is_ticked();
	void set_label(const char* label);
	std::string get_label();

	virtual void set_data(void* data) = 0;	
protected:
	std::string _label;
	bool _ticked;
private:
	CheckBox_Group* _group;
	virtual void on_tick_action() = 0;
	virtual void off_tick_action() = 0;
};


// --------------------------------------------------
// C H E C K B O X _ G R O U P
// --------------------------------------------------
class CheckBox_Group{
	friend class CheckBox;
	enum group_dir_e { HORIZONTAL, VERTICAL };	
public:	
	Collision hitbox;
	int x, y, z;
	bool can_only_select_one;
	// int w, h;
	
	CheckBox_Group();
	virtual ~CheckBox_Group();
	void init(int x, int y, int z);

	std::list<CheckBox*> get_selected();
	bool add_checkbox(CheckBox* box);
	bool remove_checkbox(CheckBox* box);
	void clear();
	//bool is_horizontal();
	//bool is_vertical();
	//void set_horizontal();
	//void set_vertical();
private:
	//group_dir_e _direction;
	std::vector<CheckBox*> _group;
	void handle_clicked_checkbox(CheckBox* box);
};

