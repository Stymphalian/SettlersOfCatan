#pragma once
#include <string>
#include <list>
#include <fstream>

namespace CONFIG_INI{
	// - - - I N I _ I T E M
	class ini_property{
	public:
		std::string section;
		std::string tag;
		std::string value;
	};

	// - - - - I N I _ R E A D E R - - - - - 
	class ini_reader{
	public:
		// destructor
		virtual ~ini_reader();

		// methods
		ini_property get_property(const char* tag, const char* section = NULL);
		std::list<std::string> get_all_tags(const char* section=NULL);
		std::list<std::string> get_all_sections();
		int get_num_sections();
		int get_num_tags(const char* section=NULL);
		bool add_item(const char* tag, const char* value, const char* section = NULL);
		bool add_comment(const char* comment, const char* section = NULL);
		bool add_section(const char* section);
		bool remove_section(const char* section = NULL);
		bool remove_tag(const char* tag, const char* section = NULL);
		bool rename_tag(const char* tag, const char* new_name, const char* section=NULL);
		bool modify_value(const char* tag, const char* newvalue, const char* section = NULL);
		bool write_to_file(const char* output_filename);
		void print();
	private:
		// private constructor. You should only be able to create 
		// an ini_reader throught the factory.
		friend class ini_factory;
		bool _good_flag;
		int _line;
		ini_reader(){}
		ini_reader(const char* filename);
		

		// I N N E R   C L A S S E S 
		// - - - -  I N I _ N O D E  - - - - -
		class ini_node{
		public:
			enum {NONE,SECTION,GLOBAL_SECTION,ELEMENT, COMMENT};
			ini_node();
			ini_node(int type, ini_node* parent);
			virtual ~ini_node();
			//variables
			int type;
			ini_node* parent;
			std::list<ini_node*> children;
			std::string str1;
			std::string str2; // used for	value 
			//methods
			void init(int type, ini_node* parent);
			bool add_child(ini_node* child);
			bool remove_child(ini_node* child);
			std::string get_section();
			std::string get_tagname();
			std::string get_value();
			std::string get_comment();			
		};
		

		// variables		
		ini_node* _root;
		ini_node* _current_section;
		std::string _filename;		
		
		//methods
		enum dom_states_e {
			DOM_NONE, DOM_START, DOM_OPEN_SECTION, DOM_SECTION,DOM_SECTION_END,
			DOM_CLOSE_SECTION, DOM_COMMENT, DOM_TAGNAME, DOM_TAGNAME_END, DOM_VALUE,DOM_DEAD
		};
		bool serialize(std::ofstream& output);
		void trim_whitespace_right(std::string* str);
		void trim_whitespace_left(std::string* str);
		std::string get_file_extension(std::string* str);
		void cut_tree(ini_node* node);
		ini_node* create_dom(std::ifstream& input_stream);
		dom_states_e handle_dom_start(std::string* temp_str,char c);
		dom_states_e handle_dom_open_section(std::string* temp_str, char c);
		dom_states_e handle_dom_section(std::string* temp_str, char c);
		dom_states_e handle_dom_section_end(std::string* temp_str, char c);
		dom_states_e handle_dom_close_section(std::string* temp_str, char c);
		dom_states_e handle_dom_comment(std::string* temp_str, char c);
		dom_states_e handle_dom_tagname(std::string* temp_str, char c);
		dom_states_e handle_dom_tagname_end(std::string* temp_str, char c);
		dom_states_e handle_dom_value(std::string* temp_str, char c);
		ini_node* find_section_node(std::string section="");
		ini_node* find_tag_from_section(std::string tagname, ini_node* section);
	};


	// ----- I N I _ F A C T O R Y ------ 
	class ini_factory{
	public:
		static ini_reader* open(const char* file);
	private:
		ini_factory(){}
		ini_factory(ini_factory const&){};
		virtual ~ini_factory(){};
		void operator= (const ini_factory& orig);
	};

}

