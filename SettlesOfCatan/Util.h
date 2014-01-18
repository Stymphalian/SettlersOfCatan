#pragma once

#include <SDL.h>
#include <SDL_ttf.h>
#include <vector>

#define GAME_SMALL
#ifdef GAME_SMALL
	#define UTIL_DISP_W 640
	#define UTIL_DISP_H 640
	#define UTIL_VERTEX_HITBOX_W 12
	#define UTIL_VERTEX_HITBOX_H 12
	#define UTIL_HEXTILE_W 40
	#define UTIL_HEXTILE_H 40
	#define UTIL_SPRITE_SMALL_W 20
	#define UTIL_SPRITE_SMALL_H 20
	#define UTIL_SPRITE_MEDIUM_W 40
	#define UTIL_SPRITE_MEDIUM_H 40
	#define UTIL_SPRITE_LARGE_W 64
	#define UTIL_SPRITE_LARGE_H 64
	#define UTIL_SPRITE_FACE_W 20
	#define UTIL_SPRITE_FACE_H 10
	#define UTIL_FPS 30
#else 
	#define UTIL_DISP_W 1200
	#define UTIL_DISP_H 640
	#define UTIL_VERTEX_HITBOX_W 24
	#define UTIL_VERTEX_HITBOX_H 24
	#define UTIL_HEXTILE_W 80
	#define UTIL_HEXTILE_H 80
	#define UTIL_SPRITE_SMALL_W 40
	#define UTIL_SPRITE_SMALL_H 40
	#define UTIL_SPRITE_MEDIUM_W 80
	#define UTIL_SPRITE_MEDIUM_H 80
	#define UTIL_SPRITE_LARGE_W 128
	#define UTIL_SPRITE_LARGE_H 128
	#define UTIL_SPRITE_FACE_W 40
	#define UTIL_SPRITE_FACE_H 20
	#define UTIL_FPS 30
#endif

class Util{
public:
	// static methods
	static SDL_Surface* load_surface(const char* file);
	static SDL_Texture* load_texture(const char* file, SDL_Renderer* ren);
	static SDL_Texture* load_texture_bmp(const char* file, SDL_Renderer* ren);
	static void render_texture(SDL_Renderer* ren, SDL_Texture* tex, int x, int y, int w, int h, SDL_Rect* clip = nullptr);
	static void render_texture(SDL_Renderer* ren, SDL_Texture* tex, int x, int y, SDL_Rect* clip = nullptr);
	static void render_rectangle(SDL_Renderer* rend, SDL_Rect* rect, SDL_Color color);
	static void render_fill_rectangle(SDL_Renderer* rend, SDL_Rect* rect, SDL_Color color);
	static void render_text(SDL_Renderer* ren, TTF_Font* font, int x, int y, SDL_Color color, const char* msg, ...);
	static Uint32 get_pixel32(SDL_Surface* surface, int x, int y);
	static void set_pixel32(SDL_Surface* surface, int x, int y,Uint32 value);
	static void shuffle(int* bag, int size);
	static SDL_Color colour_red();
	static SDL_Color colour_green();
	static SDL_Color colour_blue();
	static SDL_Color colour_white();
	static SDL_Color colour_black();

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

	Util();
	virtual ~Util();
	Util(const Util&);
	void operator= (Util const&);
};

