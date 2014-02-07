#pragma once

#include <string>
#include <SDL.h>
#include <SDL_ttf.h>
#include <vector>
#include "ini_reader.h"

class Util{
public:
	// static methods
	static Uint32 render_count;
	static SDL_Surface* load_surface(const char* file);
	static SDL_Texture* load_texture(const char* file, SDL_Renderer* ren);
	static SDL_Texture* load_texture_bmp(const char* file, SDL_Renderer* ren);
	static SDL_Surface* load_surface(std::string file);
	static SDL_Texture* load_texture(std::string file, SDL_Renderer* ren);
	static SDL_Texture* load_texture_bmp(std::string file, SDL_Renderer* ren);
	static void render_texture(SDL_Renderer* ren, SDL_Texture* tex, int x, int y, int w, int h, SDL_Rect* clip = nullptr);
	static void render_texture(SDL_Renderer* ren, SDL_Texture* tex, int x, int y, SDL_Rect* clip = nullptr);
	static void render_rectangle(SDL_Renderer* rend, SDL_Rect* rect, SDL_Color color);
	static void render_fill_rectangle(SDL_Renderer* rend, SDL_Rect* rect, SDL_Color color);
	static void render_text(SDL_Renderer* ren, TTF_Font* font, int x, int y, SDL_Color color, const char* msg, ...);
	static void render_line(SDL_Renderer* ren, SDL_Color color,int x1, int y1, int x2, int y2);
	static Uint32 get_pixel32(SDL_Surface* surface, int x, int y);
	static void set_pixel32(SDL_Surface* surface, int x, int y,Uint32 value);
	static void shuffle(int* bag, int size);
	static SDL_Color colour_red();
	static SDL_Color colour_green();
	static SDL_Color colour_blue();
	static SDL_Color colour_white();
	static SDL_Color colour_black();
	static SDL_Color colour_orange();
	static std::string data_resource(const char* res);
	static void push_texture_mods(SDL_Texture* texture, int r, int g, int b, int a);
	static void pop_texture_mods(SDL_Texture* texture);
	

	//member methods
	static Util& get();
	Uint32 get_userev(const char* ev);
	void push_userev(Uint32 user_type, Sint32 code, void* data1, void* data2);
private:
	struct userev_and_str{
		std::string name;
		Uint32 ev;
	};
	std::vector<userev_and_str> user_evs;
	
	// static private methods
	static void push_pop_texture_mods(int type, SDL_Texture* texture, int r, int g, int b, int a);

	Util();
	virtual ~Util();
	Util(const Util&);
	void operator= (Util const&);
};

