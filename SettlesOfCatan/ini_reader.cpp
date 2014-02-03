#include "ini_reader.h"
#include <cstdio>
#include <string>
#include <list>
#include <fstream>
#include <iostream>

namespace CONFIG_INI {
// - - - - I N I _ R E A D E R - - - - - 
ini_reader::ini_reader(const char* filename){
	// initial variables set.
	this->_good_flag = false;
	this->_root = NULL;
	this->_current_section = NULL;
	this->_line = 0;
	if(filename == NULL){return;}
	this->_filename = filename;

	// check that the file extension is "ini"
	std::string filename_str = filename;
	if(get_file_extension(&filename_str).compare("ini") != 0){return;}

	// open the file
	std::ifstream input_stream(filename, std::ifstream::in);
	if(input_stream.fail()){ return; }
		
	// create the dom		
	this->_root = create_dom(input_stream);
	input_stream.close();
	_good_flag = (this->_root != NULL);
}

ini_reader::~ini_reader(){
	cut_tree(_root);
	_root = NULL;
}

ini_property ini_reader::get_property(const char* tag, const char* section){
	ini_property prop;
	if(tag == NULL){ return prop; }
	ini_node* section_node = (section == NULL) ? find_section_node() : find_section_node(section);
	if(section_node == NULL){ return prop; }

	ini_node* property_node = find_tag_from_section(tag, section_node);
	if(property_node == NULL){ return prop; }

	// return the property object
	prop.section = section_node->get_section();
	prop.tag = property_node->get_tagname();
	prop.value = property_node->get_value();
	return prop;
}
std::list<std::string> ini_reader::get_all_tags(const char* section){
	std::list<std::string> stuff;	
	ini_node* section_node = (section == NULL) ? find_section_node() : find_section_node(section);
	if(section_node == NULL){ return stuff; }

	std::list<ini_node*>::iterator it;
	for(it = section_node->children.begin(); it != section_node->children.end(); ++it){
		if((*it)->type != ini_node::ELEMENT){continue; }
		stuff.push_front((*it)->get_tagname());
	}	
	return stuff;
}
std::list<std::string> ini_reader::get_all_sections(){
	std::list<std::string> stuff;
	std::list<ini_node*>::iterator it;
	for(it = _root->children.begin(); it != _root->children.end(); ++it){
		if((*it)->type != ini_node::GLOBAL_SECTION && (*it)->type != ini_node::SECTION){
			continue;
		}
		stuff.push_front((*it)->get_section());
	}
	return stuff;
}
int ini_reader::get_num_sections(){
	return _root->children.size();
}
int ini_reader::get_num_tags(const char* section){
	ini_node* section_node = (section == NULL) ? find_section_node() : find_section_node(section);
	if(section_node == NULL){ return -1; }

	std::list<ini_node*>::iterator it;
	int count = 0;
	for(it = section_node->children.begin(); it != section_node->children.end(); ++it){
		if((*it)->type == ini_node::ELEMENT){ ++count; }
	}
	return count;
}

bool ini_reader::add_item(const char* tag, const char* value, const char* section){
	if(tag == NULL || value == NULL){ return false; }
	ini_node* section_node = (section == NULL) ? find_section_node() : find_section_node(section);
	if(section_node == NULL){ return false; }

	ini_node* newnode = new ini_node(ini_node::ELEMENT, section_node);
	newnode->str1 = tag;
	newnode->str2 = value;
	section_node->add_child(newnode);
	return true;
}

bool ini_reader::add_comment(const char* comment, const char* section){
	if(comment == NULL){ return false; }
	ini_node* section_node = (section == NULL) ? find_section_node() : find_section_node(section);
	if(section_node == NULL){ return false; }

	ini_node* newnode = new ini_node(ini_node::COMMENT, section_node);
	newnode->str1 = comment;
	section_node->add_child(newnode);
	return true;
}

bool ini_reader::add_section(const char* section){
	if(section == NULL){ return false; }
	// check for duplicate sections.
	std::list<ini_node*>::iterator it = _root->children.begin();
	while(it != _root->children.end()){
		if((*it)->get_section().compare(section) == 0){
			return false;
		}
		it++;
	}

	// no duplicate section, so add away.
	ini_node* section_node = new ini_node(ini_node::SECTION, _root);
	section_node->str1 = section;
	_root->add_child(section_node);	
	return true;
}
bool ini_reader::remove_section(const char* section){
	std::list<ini_node*>::iterator it;
	std::string section_name = (section == NULL) ? "" : section;
	ini_node* section_node = NULL;
	for(it = _root->children.begin(); it != _root->children.end(); ++it){
		if((*it)->type != ini_node::GLOBAL_SECTION && (*it)->type != ini_node::SECTION){
			continue;
		}
		if((*it)->get_section().compare(section_name) == 0){
			section_node = *it;
			_root->children.erase(it);
			break;
		}
	}
	if(section_node == NULL){ return false; }
	cut_tree(section_node);
	return true;
}
bool ini_reader::remove_tag(const char* tag, const char* section ){
	if(tag == NULL){ return false; }
	ini_node* section_node = (section == NULL) ? find_section_node() : find_section_node(section);
	if(section_node == NULL){ return false; }

	ini_node* tag_node = NULL;
	std::list<ini_node*>::iterator it;
	for(it = section_node->children.begin(); it != section_node->children.end(); ++it){
		if((*it)->type != ini_node::ELEMENT){ continue; }
		if((*it)->get_tagname().compare(tag) == 0){
			tag_node = *it;
			section_node->children.erase(it);
			break;
		}
	}
	if(tag_node == NULL){ return false; }
	cut_tree(tag_node);
	return false;
}
bool ini_reader::rename_tag(const char* tag, const char* new_name, const char* section){
	if(tag == NULL || new_name == NULL){ return false; }
	ini_node* section_node = (section == NULL) ? find_section_node() : find_section_node(section);
	if(section_node == NULL){ return false; }

	ini_node* property_node = find_tag_from_section(tag, section_node);
	if(property_node == NULL){ return false; }

	property_node->str1 = std::string(new_name);
	return true;
}
bool ini_reader::modify_value(const char* tag, const char* newvalue, const char* section ){
	if(tag == NULL || newvalue == NULL){ return false; }
	ini_node* section_node = (section == NULL) ? find_section_node() : find_section_node(section);
	if(section_node == NULL){ return false; }

	ini_node* property_node = find_tag_from_section(tag, section_node);
	if(property_node == NULL){ return false; }

	property_node->str2 = std::string(newvalue);
	return true;
}
bool ini_reader::write_to_file(const char* output_filename){
	std::ofstream output(output_filename,std::ofstream::out);
	if(output.fail()){ return false; }
	bool rs = serialize(output);
	output.close();
	return rs;
}
void ini_reader::print(){
	serialize((std::ofstream&) std::cout);
}

bool ini_reader::serialize(std::ofstream& output){
	std::list<ini_node*> stack;
	ini_node* current = NULL;
	stack.push_front(_root);

	while(!stack.empty())
	{
		current = stack.front();
		stack.erase(stack.begin());
		if(current == NULL){ continue; }

		if(current->type == ini_node::GLOBAL_SECTION){
			// don't print anything
		} else if(current->type == ini_node::SECTION){
			output << "[" << current->get_section() << "]" << std::endl;
		} else if(current->type == ini_node::ELEMENT){
			output << current->get_tagname() << "=" << current->get_value() << std::endl;
		} else if(current->type == ini_node::COMMENT){
			output << ";" << current->get_comment() << std::endl;
		} else{
			// debugging statements?
		}
		if(!output.good()){ return false; }

		// add all the children
		std::list<ini_node*>::iterator it;
		for(it = current->children.begin(); it != current->children.end(); ++it){
			stack.push_front(*it);			
		}
	}
	return true;
}


void ini_reader::cut_tree(ini_node* node){
	//std::cout << "cut tree " << node << std::endl;
	std::list<ini_node*> stack;
	stack.push_front(node);
	ini_node* current = NULL;

	int count = 0;
	while(!stack.empty()){
		// pop off the front
		current = stack.front();
		stack.erase(stack.begin());
		if(current == NULL){ continue; }
		//std::cout << "[" << count++ << "] " << current->str1 << std::endl;

		// add all the children of current to the stack for processing.
		std::list<ini_node*>::iterator it = current->children.begin();
		while(it != current->children.end()){
			stack.push_front((*it));
			it++;
		}

		// delete the current node.
		delete current;
	}
}


void ini_reader::trim_whitespace_right(std::string* str){
	size_t p = str->find_last_not_of(" \t");
	if(std::string::npos != p){
		str->erase(p + 1);
	}
}
void ini_reader::trim_whitespace_left(std::string* str){
	size_t p = str->find_first_not_of(" \t");
	str->erase(0, p);
}

std::string ini_reader::get_file_extension(std::string* str){
	size_t p = str->find_last_of(".");
	if(p == std::string::npos){
		return "";
	}
	std::string ext = str->substr(p+1, std::string::npos);
	return ext;
}

/*
Creates a DOM tree from the provided input stream.
It will only accept a valid *.ini file.
@Parameter std::ifstream& input_stream - the stream from which we read in the ini file.
@Return: the root ini_node* pointer on success, NULL otherwise.
*/
ini_reader::ini_node* ini_reader::create_dom(std::ifstream& input_stream){
	// initialize a root node to begin with
	_root = new ini_node();
	_root->init(ini_node::NONE, NULL);

	// add a global section node underneath root.
	_current_section = new ini_node();
	_current_section->init(ini_node::GLOBAL_SECTION,_root);
	_root->add_child(_current_section);

	// Parse the input stream
	dom_states_e final_states[4] = { DOM_START, DOM_CLOSE_SECTION, DOM_COMMENT, DOM_VALUE };
	dom_states_e state = DOM_START;
	std::string temp_str;
	char c;

	while(input_stream.good()){
		input_stream.get(c);
		if(input_stream.eof()){ break; }
		if(input_stream.fail()){ state = DOM_DEAD; break; }		
				
		// handle the transition functions
		switch(state){
		case(DOM_START) : state = handle_dom_start(&temp_str, c); break;
		case(DOM_OPEN_SECTION) : state = handle_dom_open_section(&temp_str, c); break;
		case(DOM_SECTION) : state = handle_dom_section(&temp_str, c); break;
		case(DOM_SECTION_END) : state = handle_dom_section_end(&temp_str, c); break;
		case(DOM_CLOSE_SECTION) : state = handle_dom_close_section(&temp_str, c); break;
		case(DOM_COMMENT) : state = handle_dom_comment(&temp_str, c); break;
		case(DOM_TAGNAME) : state = handle_dom_tagname(&temp_str, c); break;
		case(DOM_TAGNAME_END) : state = handle_dom_tagname_end(&temp_str, c); break;
		case(DOM_VALUE) : state = handle_dom_value(&temp_str, c); break;		
		default:break;
		}

		//std::cout << prev_state << " " << c << " --> " << state << std::endl;		
		if(state == DOM_DEAD){ break; }
	}

	// make sure we are in an accepted state state.
	// and do any actions necessary to finish off any last charcters that are read.	
	if(state == DOM_START){
		// do nothing
	} else if(state == DOM_CLOSE_SECTION){
		// do nothing
	} else if(state == DOM_COMMENT){
		// append the last comment
		ini_node* new_node = new ini_node(ini_node::COMMENT, _current_section);
		new_node->str1 = temp_str.c_str();
		int before = _current_section->children.size();
		_current_section->add_child(new_node);
		temp_str.clear();
	} else if(state == DOM_VALUE){
		// append the last value node to the latest tag node.
		ini_node* property = _current_section->children.front();
		property->str2 = temp_str.c_str();
		temp_str.clear();
	} else{
		// we are not in a good valid state. So destory the tree and 
		// return NULL to signal the failure.
		/*
		char c;
		printf("failure on line %d", _line);
		printf("Press Enter to quit ...");
		int rs = scanf("%c",&c);
		*/
		cut_tree(_root);
		_root = NULL;
		_current_section = NULL;
		return NULL;
	}
	return _root;
}
ini_reader::dom_states_e ini_reader::handle_dom_start(std::string* temp_str, char c){
	if(c == ';'){ return ini_reader::DOM_COMMENT; }
	else if(c == ' ' || c == '\t'){ return ini_reader::DOM_START; }
	else if(c == '\n'){ 
		_line++;
		return ini_reader::DOM_START;
	}
	else if(c == '['){ return ini_reader::DOM_OPEN_SECTION; }
	else { 
		temp_str->clear();
		temp_str->append(1, c);
		return ini_reader::DOM_TAGNAME;
	}
}
ini_reader::dom_states_e ini_reader::handle_dom_open_section(std::string* temp_str, char c){
	if(c == ' ' || c == '\t'){
		return ini_reader::DOM_OPEN_SECTION;
	}else if(c == '\n'){ 
		_line++;
		return ini_reader::DOM_DEAD; 
	}else if(c == ']'){ return ini_reader::DOM_DEAD; }
	else { 
		temp_str->append(1,c);
		return ini_reader::DOM_SECTION;
	}
}

ini_reader::dom_states_e ini_reader::handle_dom_section(std::string* temp_str, char c){
	if(c == ' ' || c == '\t'){ return ini_reader::DOM_SECTION_END; }
	else if(c == '\n'){
		_line++;
		return  ini_reader::DOM_DEAD;
	}else if(c == ']'){
		trim_whitespace_left(temp_str);
		trim_whitespace_right(temp_str);
		if(temp_str->compare("") == 0){ return ini_reader::DOM_DEAD; }

		// check for duplicate sections.
		std::list<ini_node*>::iterator it = _root->children.begin();
		while(it != _root->children.end()){
			if(temp_str->compare((*it)->get_section()) == 0){
				temp_str->clear();
				_current_section = *it;				
				return ini_reader::DOM_CLOSE_SECTION;
			}
			it++;
		}

		// no duplicate section, so add away.
		ini_node* section = new ini_node(ini_node::SECTION,_root);
		section->str1 = temp_str->c_str();
		_root->add_child(section);

		temp_str->clear();
		_current_section = section;
		return ini_reader::DOM_CLOSE_SECTION;
	} else {
		temp_str->append(1, c);
		return ini_reader::DOM_SECTION;
	}

}

ini_reader::dom_states_e ini_reader::handle_dom_section_end(std::string* temp_str, char c){
	if(c == ' ' || c == '\t'){ return ini_reader::DOM_SECTION_END;}
	else if(c == '\n'){ 
		_line++;
		return ini_reader::DOM_DEAD; 
	}else if ( c == ']'){
		trim_whitespace_left(temp_str);
		trim_whitespace_right(temp_str);
		if(temp_str->compare("") == 0){ return ini_reader::DOM_DEAD; }

		// check for duplicate sections.
		std::list<ini_node*>::iterator it = _root->children.begin();
		while(it != _root->children.end()){
			if(temp_str->compare((*it)->get_section()) == 0){
				temp_str->clear();
				_current_section = *it;
				return ini_reader::DOM_CLOSE_SECTION;
			}
			it++;
		}

		// no duplicate section, so add away.
		ini_node* section = new ini_node(ini_node::SECTION, _root);
		section->str1 = temp_str->c_str();
		_root->add_child(section);

		temp_str->clear();
		_current_section = section;
		return ini_reader::DOM_CLOSE_SECTION;
	} else{
		return ini_reader::DOM_DEAD;
	}

}
ini_reader::dom_states_e ini_reader::handle_dom_close_section(std::string* temp_str, char c){
	if(c == ' ' || c == '\t'){return ini_reader::DOM_CLOSE_SECTION;}
	else if(c == '\n'){
		_line++;
		temp_str->clear();
		return ini_reader::DOM_START;
	}
	else {return ini_reader::DOM_DEAD;}
}
ini_reader::dom_states_e ini_reader::handle_dom_comment(std::string* temp_str, char c){
	if(c == '\n'){
		_line++;
		// append the new node.
		ini_node* comment = new ini_node(ini_node::COMMENT,_current_section);
		comment->str1 = temp_str->c_str();
		_current_section->add_child(comment);
		temp_str->clear();
		return ini_reader::DOM_START;
	} else{
		temp_str->append(1, c);
		return ini_reader::DOM_COMMENT;
	}
}
ini_reader::dom_states_e ini_reader::handle_dom_tagname(std::string* temp_str, char c){
	if(c == ' ' || c == '\t'){ return ini_reader::DOM_TAGNAME_END; }
	if(c == '\n'){
		_line++;
		return ini_reader::DOM_DEAD; 
	}else if(c == '='){
		trim_whitespace_left(temp_str);
		trim_whitespace_right(temp_str);
		ini_node* property = new ini_node(ini_node::ELEMENT, _current_section);
		property->str1 = temp_str->c_str();
		_current_section->add_child(property);

		temp_str->clear();
		return ini_reader::DOM_VALUE;		
	} else {
		temp_str->append(1, c);
		return ini_reader::DOM_TAGNAME;
	}
}

ini_reader::dom_states_e ini_reader::handle_dom_tagname_end(std::string* temp_str, char c){
	if(c == ' ' || c == '\t'){ return ini_reader::DOM_TAGNAME_END; }
	else if(c == '\n') { 
		_line++;
		return ini_reader::DOM_DEAD; 
	}else if(c == '='){
		trim_whitespace_left(temp_str);
		trim_whitespace_right(temp_str);
		ini_node* property = new ini_node(ini_node::ELEMENT, _current_section);
		property->str1 = temp_str->c_str();
		_current_section->add_child(property);

		temp_str->clear();
		return ini_reader::DOM_VALUE;
	} else{return ini_reader::DOM_DEAD;}
}

ini_reader::dom_states_e ini_reader::handle_dom_value(std::string* temp_str, char c){
	if(c == '\n'){
		_line++;
		// add the value to the required node.
		ini_node* property = NULL;
		property = _current_section->children.front();
		property->str2 = temp_str->c_str();
		temp_str->clear();
		return ini_reader::DOM_START;
	} else {
		temp_str->append(1, c);
		return ini_reader::DOM_VALUE;
	}
}


ini_reader::ini_node* ini_reader::find_section_node(std::string section){
	if(section.compare("") == 0){
		if(_root->children.back()->type == ini_node::GLOBAL_SECTION){
			return _root->children.back();
		}
	}
	std::list<ini_node*>::iterator it = _root->children.begin();
	while(it != _root->children.end()){
		if((*it)->type == ini_node::SECTION &&
			(*it)->get_section().compare(section) == 0)
		{
			return *it;
		}
		it++;
	}
	return NULL;
}
ini_reader::ini_node* ini_reader::find_tag_from_section(std::string tagname, ini_node* section){
	if(section == NULL){ return NULL; }
	if(section->type != ini_node::SECTION && section->type != ini_node::GLOBAL_SECTION){ return NULL; }
	std::list<ini_node*>::iterator it = section->children.begin();
	for(it = it; it != section->children.end(); ++it){
		if((*it)->type != ini_node::ELEMENT){ continue; }
		if((*it)->get_tagname().compare(tagname) == 0){
			return *it;
		}
	}	
	return NULL;
}

// I N N E R   C L A S S E S 
// - - - -  I N I _ N O D E  - - - - - 
ini_reader::ini_node::ini_node(){
	init(ini_reader::ini_node::NONE, NULL);
}
ini_reader::ini_node::ini_node(int type,ini_reader::ini_node* parent){
	init(type, parent);
}

// NOTE, this will node call 'delete' on the children of the node.
// It is up to you to free any memory. associatead with the nodes.
ini_reader::ini_node::~ini_node(){
//	printf("ini_node destructor %x", this);
	parent = NULL;
	type = 0;
	children.clear();
}
void ini_reader::ini_node::init(int type, ini_node* parent){
	this->parent = parent;
	this->type = type;
}
bool ini_reader::ini_node::add_child(ini_node* child){
	if(child == NULL){return false;}
	// NOTE, we push to the front so that when we iterate we get the correct order.
	children.push_front(child);
	return true;
}
bool ini_reader::ini_node::remove_child(ini_node* child){
	if(child == NULL){ return false; }
	std::list<ini_node*>::iterator it= children.begin();
	while(it != children.end()){
		if(*it == child){
			children.erase(it++);
		} else{
			it++;
		}
	}
	return true;
}

std::string ini_reader::ini_node::get_section(){
	if(type == ini_reader::ini_node::SECTION || type == ini_reader::ini_node::GLOBAL_SECTION){
		return str1;
	} else {
		return "";
	}
}
std::string ini_reader::ini_node::get_tagname(){
	return (type != ini_reader::ini_node::ELEMENT) ? "" : str1;
}
std::string ini_reader::ini_node::get_value(){
	return (type != ini_reader::ini_node::ELEMENT) ? "" : str2;
}
std::string ini_reader::ini_node::get_comment(){
	return (type != ini_reader::ini_node::COMMENT) ? "" : str1;
}


// ----- I N I _ F A C T O R Y ------ 
ini_reader* ini_factory::open(const char* file){
	if(file == NULL){ return NULL; }
	ini_reader* reader = new ini_reader(file);
	if(reader->_good_flag == false){
		delete reader;
		reader = NULL;
	}
	return reader;
}
}
	