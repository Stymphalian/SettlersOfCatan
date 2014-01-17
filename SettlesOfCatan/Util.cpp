#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include "Util.h"
#include "Logger.h"

Util::Util(){}
Util::~Util(){
	Logger::getLog("jordan.log").log(Logger::DEBUG, "Util destructor");
	user_evs.clear();
}
Util& Util::get(){
	static Util instance;
	return instance;
}


SDL_Surface* Util::load_surface(const char* file){
	if(file == nullptr){ return nullptr; }
	SDL_Surface* surface = IMG_Load(file);
	if(surface == nullptr){
		Logger::getLog("jordan.log").IMG_log(Logger::ERROR, "IMG_Load(%s)", file);	
		return nullptr;
	}
	return surface;
}

SDL_Texture* Util::load_texture(const char* file, SDL_Renderer* ren){
	if(file == nullptr || ren == nullptr){ return nullptr; }
	SDL_Texture*  texture = IMG_LoadTexture(ren, file);
	if(texture == nullptr){
		Logger& logger = Logger::getLog("jordan.log");
		logger.IMG_log(logger.ERROR, "IMG_LoadTexture(%s)",file);
		return 0;
	}
	return texture;
}

SDL_Texture* Util::load_texture_bmp(const char* file, SDL_Renderer* ren){
	Logger& logger = Logger::getLog("jordan.log");
	SDL_Surface* bmp = SDL_LoadBMP(file);
	if(bmp == nullptr){
		logger.SDL_log(logger.ERROR, "SDL_LoadBMP(%s)",file);
		return 0;
	}

	SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, bmp);
	SDL_FreeSurface(bmp);
	if(tex == nullptr) {
		logger.SDL_log(logger.ERROR, "SDL_CreateTextureFromSurface");
		return 0;
	}
	return tex;
}


void Util::render_texture(SDL_Renderer* ren, SDL_Texture* tex, int x, int y,
	SDL_Rect* clip){
	// Query the texture to gets its width/height to use
	SDL_Rect dst;
	dst.x = x;
	dst.y = y;
	if(clip != nullptr){
		dst.w = clip->w;
		dst.h = clip->h;
	}
	else{
		Uint32 format = 0;
		int access = 0;
		SDL_QueryTexture(tex, &format, &access, &dst.w, &dst.h);
	}

	// call render texture to do all the rendering
	if(SDL_RenderCopy(ren, tex, clip, &dst) == -1){
		Logger::getLog("jordan.log").SDL_log(Logger::ERROR, "SDL_RenderCopy");
	}
}
void Util::render_texture(SDL_Renderer* ren, SDL_Texture* tex,
	int x, int y, int w, int h, SDL_Rect* clip)
{
	// Query the texture to gets its width/height to use
	SDL_Rect dst;
	dst.x = x;
	dst.y = y;
	dst.w = w;
	dst.h = h;

	// call render texture to do all the rendering
	if(SDL_RenderCopy(ren, tex, clip, &dst) == -1){
		Logger::getLog("jordan.log").SDL_log(Logger::ERROR, "SDL_RenderCopy");
	}
}

void Util::render_rectangle(SDL_Renderer* ren, SDL_Rect* rect, SDL_Color color = { 0, 0, 0, 0 }){
	Uint8 red, green, blue, alpha;
	SDL_GetRenderDrawColor(ren, &red, &green, &blue, &alpha);
	SDL_SetRenderDrawColor(ren, color.r, color.g, color.b,color.a);

	if(SDL_RenderDrawRect(ren, rect) != 0){
		Logger::getLog("jordan.log").log(Logger::ERROR, "SDL_RenderDrawRect()");
	}
	SDL_SetRenderDrawColor(ren, red, green, blue, alpha);
}
void Util::render_fill_rectangle(SDL_Renderer* ren, SDL_Rect* rect, SDL_Color color = { 0, 0, 0, 0 }){
	Uint8 red, green, blue, alpha;
	SDL_GetRenderDrawColor(ren, &red, &green, &blue, &alpha);
	SDL_SetRenderDrawColor(ren, color.r, color.g, color.b, color.a);
	if(SDL_RenderFillRect(ren, rect) != 0){
		Logger::getLog("jordan.log").log(Logger::ERROR, "SDL_RenderFillRect()");
	}
	SDL_SetRenderDrawColor(ren, red, green, blue, alpha);
}


void Util::render_text(SDL_Renderer* ren, TTF_Font* font, int x, int y, SDL_Color color, const char* format, ...){
	Logger& logger = Logger::getLog("jordan.log");

	// Create a string with the message
	va_list args;
	va_start(args, format);
	char msg[2056];
	int size = vsprintf(msg, format, args);
	if(size == -1){
		logger.log(logger.ERROR, "vsprintf");
		return;
	}
	va_end(args);

	// Create the surface, which we will load onto the texture.
	SDL_Surface* surface = TTF_RenderText_Blended(font, msg, color);
	if(surface == nullptr){
		logger.TTF_log(logger.ERROR, "TTF_RenderText_Blended");
		return;
	}

	// Create the texture from the text-font surface
	SDL_Texture* texture = SDL_CreateTextureFromSurface(ren, surface);
	SDL_FreeSurface(surface);
	if(texture == nullptr){
		logger.SDL_log(logger.ERROR, "SDL_CreateTextureFromSurface");
		return;
	}

	// Call render_texture on the texture
	render_texture(ren, texture, x, y);
	SDL_DestroyTexture(texture);
}

Uint32 Util::get_pixel32(SDL_Surface* surface, int x, int y){	
	Uint32* pixels = (Uint32*) surface->pixels;
	return pixels[x + y*surface->w];
	
}
void Util::set_pixel32(SDL_Surface* surface, int x, int y,Uint32 value){
	Uint32* pixels = (Uint32*)surface->pixels;
	pixels[x + y*surface->w] = value;
}

void Util::shuffle(int* arr, int size){
	if(!arr){ return; }
	Logger::getLog("jordan.log").log(Logger::DEBUG, "Util::shuffle(array,size=%d)", size);
	// array to keep track of what values have been already used.
	// temproray array to copy elements into
	char* taken = new char[size];
	int* copy = new int[size];

	memset(taken, 0, size);
	memset(copy, 0, sizeof(int)*size);
	int copy_size = 0;

	// fill the 'copy' array until every element is placed.
	int item;
	unsigned count = 0;
	while(copy_size != size){
		++count;
		item = rand() % size;
		if(taken[item] == 1){ continue; }

		taken[item] = 1;
		copy[copy_size++] = arr[item];
	}

	// copy 'copy' back into the array
	memcpy(arr, copy, sizeof(int)*size);
	// clean up
	delete[] copy;
	delete[] taken;
	Logger::getLog("jordan.log").log(Logger::DEBUG, "Util::shuffle rolled %u times for %d items", count, size);
}


SDL_Color Util::colour_red(){
	static SDL_Color instance = { 255, 0, 0, 255 };
	return instance;
}
SDL_Color Util::colour_green(){
	static SDL_Color instance = { 0, 255, 0, 255 };
	return instance;
}
SDL_Color Util::colour_blue(){
	static SDL_Color instance = { 0, 0, 255, 255 };
	return instance;
}
SDL_Color Util::colour_white(){
	static SDL_Color instance = { 255, 255, 255, 255 };
	return instance;
}
SDL_Color Util::colour_black(){
	static SDL_Color instance = { 0, 0, 0, 255 };
	return instance;
}

Uint32 Util::get_userev(const char* ev){
	// special case in which there are no user-event types registered
	if(user_evs.size() == 0){
		userev_and_str type;
		type.ev = SDL_RegisterEvents(1);
		type.name = ev;
		if(type.ev == ((Uint32)-1)){
			Logger::getLog("jordan.log").SDL_log(Logger::levels::ERROR, "SDL_RegisterEvents");
			return ((Uint32)-1);
		} else{
			user_evs.push_back(type);
		}
	}
	
	// get the user event, OR add a new one if it isn't registered
	std::vector<userev_and_str>::iterator it;
	for(it = user_evs.begin(); it != user_evs.end(); ++it){
		if(it->name.compare(ev) == 0){
			return it->ev;
		} else{
			userev_and_str type;
			type.ev = SDL_RegisterEvents(1);
			if(type.ev == ((Uint32)-1)){
				Logger::getLog("jordan.log").SDL_log(Logger::levels::ERROR, "SDL_RegisterEvents");
				break;
			}
			type.name = ev;
			user_evs.push_back(type);
			return type.ev;
		}
	}
	return ((Uint32) - 1);
}

void Util::push_userev(Uint32 user_type, Sint32 code, void* data1, void* data2){
	SDL_Event ev;
	SDL_zero(ev);
	ev.type = user_type;
	ev.user.type = user_type;
	ev.user.code = code;
	ev.user.data1 = data1;
	ev.user.data2 = data2;
	//Logger::getLog("jordan.log").log(Logger::DEBUG, "Util::push_userev %u %u, %d, %x, %x",SDL_USEREVENT,user_type, code, data1,data2);
	if(SDL_PushEvent(&ev) != 1){
		Logger::getLog("jordan.log").SDL_log(Logger::DEBUG, "SDL_PushEvent");
	}
}