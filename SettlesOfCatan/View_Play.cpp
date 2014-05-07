
#include <algorithm>
#include <cassert>

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <string>
#include <list>
#include <iostream>
#include <random>

#include <SDL.h>
#include "Vector3d.h"
#include "View_Play.h"
#include "IDialog.h"
#include "Timer.h"
#include "Util.h"
#include "Configuration.h"
#include "Logger.h"
#include "Collision.h"
#include "TextField.h"
#include "DropDown.h"
#include "Coords.h"
#include "mouse.h"
#include "Pane.h"


#include "ViewPort.h"
#include "Selectable.h"
#include "MouseListener.h"


Vector3d GLOBAL_AXIS( 1.0f, 0.0f);

Polygon::Polygon(float x, float y, float z){
	constructor(x, y, z);
}
Polygon::Polygon(){
	constructor(0, 0, 0);	
}
Polygon::~Polygon(){
	vectors.clear();
	normals.clear();
}
void Polygon::constructor(float x, float y, float z){	
	center.init(x,y,z);
	vectors.clear();
	normals.clear();
	collided_colour = { 40, 40, 40, 90 };
}

int Polygon::num_points(){ return (int) vectors.size(); }
void Polygon::append_point(Vector3d v){
	vectors.push_back(v);
	normals.push_back(v.get_clockwise_2d_normal());
}
void Polygon::add_point(unsigned index, Vector3d v){
	if(index >= vectors.size()){ return; }
	vectors.insert(vectors.begin() + index, v);
	normals.push_back(v.get_clockwise_2d_normal());
}
void Polygon::append_point(float x, float y, float z){
	Vector3d v(x, y, z);
	append_point(v);
}
void Polygon::add_point(unsigned index, float x, float y, float z){
	Vector3d v(x, y, z);
	add_point(index, v);
}
void Polygon::remove_point(unsigned index){
	if(index >= vectors.size()){ return; }
	vectors.erase(vectors.begin() + index);
	normals.erase(normals.begin() + index);
}

int Polygon::min_along_axis(Vector3d& axis, float* rs){
	int num_points = vectors.size();
	if(num_points == 0){ return -1; }
	float target_projection_len = (vectors[0] + center)*axis;
	float projection_len = 0;
	int target_vector = 0;
	
	for(int i = 1; i < num_points; ++i){
		projection_len = (vectors[i] + center).projection(axis);
		if(projection_len < target_projection_len){
			target_projection_len = projection_len;
			target_vector = i;
		}
	}
	if(rs != nullptr){ *rs = target_projection_len; }
	return target_vector;
}
int Polygon::max_along_axis(Vector3d& axis, float* rs){
	int num_points = vectors.size();
	if(num_points == 0){ return -1; }
	float target_projection_len = (vectors[0] + center)*axis;
	float projection_len = 0;
	int target_vector = 0;	

	for(int i = 1; i < num_points; ++i){
		projection_len = (vectors[i] + center).projection(axis);
		if(projection_len > target_projection_len){
			target_projection_len = projection_len;
			target_vector = i;
		}
	}
	if(rs != nullptr){ *rs = target_projection_len; }
	return target_vector;
}
// assumption that the normals for all the vectors have
// already been calculated.
bool Polygon::collides(Polygon& other){	
	// check with the normals from ourselves
	for(unsigned i = 0; i < normals.size(); ++i){
		if(!collides(other, normals[i])){ return false; }
	}
			
	// check with the normals from the other polygon
	for(unsigned i = 0; i < other.normals.size(); ++i){
		if(!collides(other, other.normals[i])){ return false; }
	}
	return true;
}
bool Polygon::collides(Polygon& other, Vector3d& ref_axis){
	Vector3d axis = ref_axis.get_unit_vector();			
	min_max_t A = min_max_along_axis(*this,axis);
	min_max_t B = min_max_along_axis(other,axis);
	if(A.max_len < B.min_len){ return false; }
	if(B.max_len < A.min_len){ return false; }	
	return true;
}
void Polygon::rotate(float angle){ // +ve angle is clockwise
	for(unsigned i = 0; i < vectors.size(); ++i){
		vectors[i].apply_2d_rotation(angle);
	}
}
void Polygon::calculate_normals(){
	for(unsigned i = 0; i < vectors.size(); ++i){
		normals[i] = (vectors[i] - vectors[(i + 1) % vectors.size()]);
		normals[i] = normals[i].get_clockwise_2d_normal();
	}
}
void Polygon::calculate_unit_normals(){
	for(unsigned i = 0; i < vectors.size(); ++i){
		normals[i] = (vectors[i] - vectors[(i + 1) % vectors.size()]);
		normals[i] = normals[i].get_clockwise_2d_normal().get_unit_vector();
	}
}

Polygon::min_max_t Polygon::min_max_along_axis(Polygon& p, Vector3d& unit_axis){	
	min_max_t pod;
	float projection_len = 0;		

	pod.good_flag = true;
	if(p.vectors.size() <= 0){ 
		pod.good_flag = false;
		return pod;
	}
	pod.max_len = (p.vectors[0] + p.center).projection(unit_axis);
	pod.min_len = (p.vectors[0] + p.center).projection(unit_axis);
	pod.max = 0;
	pod.min = 0;	
	
	// note we start at index 1
	for(unsigned i = 1; i < p.vectors.size(); ++i){
		projection_len = (p.vectors[i] + p.center).projection(unit_axis);
		if(projection_len > pod.max_len){
			pod.max_len = projection_len;
			pod.max = i;
		}
		if(projection_len < pod.min_len){
			pod.min_len = projection_len;
			pod.min = i;
		}
	}
	return pod;
}


bool Polygon::mouse_drag(SDL_Event& ev, Coords* ref){
	center.x(center.x() + ev.motion.xrel);
	center.y(center.y() + ev.motion.yrel);
	return true;
}

void Polygon::set_collided(bool value){
	if(value == false){
		if(collided_flag == true){ off_collided(); }
	} else {
		if(collided_flag == false){ on_collided(); }
	}
	collided_flag = value;
}
void Polygon::on_collided(){
	collided_colour = { 20, 20, 80, 90 };
}
void Polygon::off_collided(){
	collided_colour = { 40, 40, 40, 90 };
}
void Polygon::render(SDL_Renderer& ren, int x, int y){	
	Vector3d translation(320, 320);

	// draw all the lines
	SDL_Color color = collided_colour;
	Uint8 red, green, blue, alpha;
	SDL_GetRenderDrawColor(&ren, &red, &green, &blue, &alpha);
	SDL_SetRenderDrawColor(&ren, color.r, color.g, color.b, color.a);
	SDL_Point* points = new SDL_Point[vectors.size()+1];
	for(int i = 0; i < (int)vectors.size(); ++i){
		points[i].x = (int) (vectors[i].x() + center.x() + translation.x());
		points[i].y = (int) (vectors[i].y() + center.y() + translation.y());
	}
	points[vectors.size()].x = (int)(vectors[0].x() + center.x() + translation.x());
	points[vectors.size()].y = (int)(vectors[0].y() + center.y() + translation.y());
	SDL_RenderDrawLines(&ren, points, vectors.size()+1);
	delete[] points;
	SDL_SetRenderDrawColor(&ren, red, green, blue, alpha);

	// draw the center dot
	SDL_Rect rect;
	SDL_Color greenish = { 40, 169, 29, 105 };
	rect = {
		(int)(center.x() + translation.x() - 3),
		(int)(center.y() + translation.y() - 3),
		6, 6
	};
	Util::render_fill_rectangle(&ren, &rect, greenish);


	// draw the point dots	
	for(int i = 0; i < (int)vectors.size(); ++i){
		rect = {
			(int) (vectors[i].x() + center.x() + translation.x() - 3),
			(int) (vectors[i].y() + center.y() + translation.y() - 3),
			6, 6
		};
		Util::render_fill_rectangle(&ren, &rect,collided_colour);
	}	

	// draw the min-max dots	
	min_max_t pod = min_max_along_axis(*this, GLOBAL_AXIS.get_unit_vector());
	// min
	SDL_Color blueish = { 40,29, 169, 105 };
	rect = {
		(int)(vectors[pod.min].x() + center.x() + translation.x() - 3),
		(int)(vectors[pod.min].y() + center.y() + translation.y() - 3),
		6, 6
	};
	Util::render_fill_rectangle(&ren, &rect, blueish);

	// max
	SDL_Color redish = { 169,40, 29, 105 };
	rect = {
		(int)(vectors[pod.max].x() + center.x() + translation.x() - 3),
		(int)(vectors[pod.max].y() + center.y() + translation.y() - 3),
		6, 6
	};
	Util::render_fill_rectangle(&ren, &rect, redish);

	// draw the normal vectors
	if(false){
		SDL_Color color = { 110, 110, 110, 90 };		
		for(int i = 0; i < (int)normals.size(); ++i){
			Util::render_line(&ren, color,
				(int) (center.y()+ translation.y()),
				(int) (center.x()+ translation.x()),
				(int) (normals[i].x() + center.x() + translation.x()),
				(int) (normals[i].y() + center.y() + translation.y())
				);
		}		
	}
}



BB::BB(){
	collided_colour = { 40, 40, 40, 90 };
}

int BB::min_along_axis(Vector3d& axis, float* rs){	
	float target_projection_len = (vectors[0] + center)*axis;
	float projection_len = 0;
	int target_vector = 0;
	
	for(int i = 1; i < num_points; ++i){		
		projection_len = (vectors[i]+ center).projection(axis);
		if(projection_len < target_projection_len){
			target_projection_len = projection_len;
			target_vector = i;
		}
	}
	if(rs != nullptr){ *rs = target_projection_len; }
	return target_vector;
}
int
BB::max_along_axis(Vector3d& axis, float* rs){
	Vector3d temp;	
	temp = vectors[0];
	temp += center;
	float target_projection_len = temp*axis;		
	int target_vector = 0;
	float projection_len = 0;

	for(int i = 1; i < num_points; ++i){
		temp = vectors[i];
		temp += center;
		projection_len = temp.projection(axis);
		if(projection_len > target_projection_len){
			target_projection_len = projection_len;
			target_vector = i;
		}
	}
	if(rs != nullptr){ *rs = target_projection_len; }
	return target_vector;
}

bool BB::collides(BB& other){
	Vector3d axis1 = this->vectors[0] - this->vectors[1];		
	if(!collides(other, axis1)){ return false; }

	Vector3d axis2 = this->vectors[1] - this->vectors[2];
	if(!collides(other, axis2)){ return false; }

	Vector3d axis3 = other.vectors[0] - other.vectors[1];
	if(!collides(other, axis3)){ return false; }

	Vector3d axis4 = other.vectors[1] - other.vectors[2];
	if(!collides(other, axis4)){ return false; }

	return true;
}

bool BB::collides(BB& other,Vector3d& ref_axis){	
	Vector3d axis = ref_axis;
	axis = axis.get_unit_vector();	
	BB* A = this;
	BB* B = &other;	
	
	float max_A;
	float min_B;	
	int min_B_index = B->min_along_axis(axis, &min_B);
	int max_A_index = A->max_along_axis(axis, &max_A);	
	if(max_A < min_B){ return false; }

	float min_A;
	float max_B;
	int min_A_index = A->min_along_axis(axis, &min_A);
	int max_B_index = B->max_along_axis(axis, &max_B);
	if(max_B < min_A){ return false; }
	return true;
}



bool BB::mouse_drag(SDL_Event& ev, Coords* ref){
	center.x(center.x() + ev.motion.xrel);
	center.y(center.y() + ev.motion.yrel);
	return true;
}

void BB::set_collided(bool value){
	if(value == false){
		if(collided_flag == true){ off_collided(); }
	} else {
		if(collided_flag == false){ on_collided(); }
	}
	collided_flag = value;
}
void BB::on_collided(){
	collided_colour = { 20, 20, 80, 90 };
}
void BB::off_collided(){
	collided_colour = { 40, 40, 40, 90 };
}
void BB::rotate(float angle){
	for(int i = 0; i < num_points; ++i){
		vectors[i].apply_2d_rotation(angle);
	}
}

//------------------------------------------------------
//------------------------------------------------------

void test_vectors(bool go){
	if(go == false){ return; }
	Vector3d v1;
	Vector3d v2;
	Vector3d v3;
	v1.init(5, 5);
	v2.init(10, 5);
	printf("v1=[%0.2f,%0.2f] v2 =[%0.2f,%0.2f]\n", v1.x(), v1.y(), v2.x(), v2.y());
	printf("v1*v2=%0.2f\n", v1*v2);
	v3 = v1 * 5;
	printf("v1*5= [%0.2f,%0.2f]\n", v3.x(), v3.y());
	v1 *= 5;
	printf("v1*= 5 --> [%0.2f,%0.2f]\n", v1.x(), v1.y());
	v3 = v1 + v2;
	printf("v1+v2= [%0.2f,%0.2f]\n", v3.x(), v3.y());
	v2 += v1;
	printf("v2 += v1 --> [%0.2f,%0.2f]\n", v2.x(), v2.y());
	printf("|v2| = %0.2f\n", v2.get_magnitude());
	v3 = v2.get_unit_vector();
	printf("v2 unit = [%0.2f,%0.2f]\n", v3.x(), v3.y());
}

View_Play::View_Play(SDL_Window& win, SDL_Renderer& ren)
: IView(win,ren)
{
	srand((unsigned)(time(NULL)));
	disp_w = Configuration::DISP_W;
	disp_h = Configuration::DISP_H;
	fps_timer = TimerFactory::get().make(1000 /Configuration::FPS);
	fps_timer->stop();
	button = 0;
	mouse_x = 0;
	mouse_y = 0;

	mouse_intersect.hook(&mouse_x, &mouse_y);
	mouse_intersect.add_rect(0, 0, 0, 1, 1);
	// MY STUFF START
	float b = 25;
	boxes[0].append_point(b,b);
	boxes[0].append_point(b,-b);
	boxes[0].append_point(-b,-b);
	boxes[0].append_point(-b,b);
	boxes[0].center.init(50,50);
	boxes[0].calculate_normals();
	
	b = 75;
	//boxes[1].append_point(b, b+20);
	//boxes[1].append_point(b, -b);
	//boxes[1].append_point(-b, -b);
	//boxes[1].append_point(-b, b);
	//boxes[1].center.init(-50, -50);
	//boxes[1].calculate_normals();

	boxes[1].append_point(0, -50);
	boxes[1].append_point(25,-25);
	boxes[1].append_point(25, 25);
	boxes[1].append_point(0, 50);
	boxes[1].append_point(-25, 25);
	boxes[1].append_point(-25, -25);
	boxes[1].center.init(-50,-50);
	boxes[1].calculate_normals();

	//boxes[0].vectors[0].init(b, b);
	//boxes[0].vectors[1].init(b, -b);
	//boxes[0].vectors[2].init(-b, -b);
	//boxes[0].vectors[3].init(-b, b);
	//boxes[0].center.init(50, 50);	
	//
	//b = 75;
	//boxes[1].vectors[0].init(b, b);
	//boxes[1].vectors[1].init(b, -b);
	//boxes[1].vectors[2].init(-b, -b);
	//boxes[1].vectors[3].init(-b, b);
	//boxes[1].center.init(-50, -50);

	test_vectors(false);
	// MY STUFF END
	font_carbon_12 = TTF_OpenFont(Util::data_resource("carbon.ttf").c_str(), 12);
	font_carbon_12_colour = { 177, 177, 98, 255 };
	if(font_carbon_12 == nullptr){
		Logger::getLog().TTF_log(Logger::ERROR, "View_Game::load() TTF_OpenFont data/carbon.ttf ");
	}
}

View_Play::~View_Play(){
	Logger::getLog().log(Logger::DEBUG, "View_Play destructor()");
	delete fps_timer;
	SDL_FreeSurface(surface);
	SDL_DestroyTexture(texture);
	TTF_CloseFont(font_carbon_12);
}

void View_Play::on_start(SDL_Event& ev){
	fps_timer->start();	
}

void View_Play::on_switch(SDL_Event& ev){
	fps_timer->stop();
}

void View_Play::on_close(SDL_Event& ev){
	fps_timer->stop();
}

void View_Play::handle_keyboard_events(SDL_Event& ev){	
	const Uint8 *keyboard = SDL_GetKeyboardState(NULL);
	if(ev.type == SDL_KEYDOWN){
		if(keyboard[SDL_SCANCODE_ESCAPE]){
			SDL_Event ev;
			SDL_zero(ev);
			ev.type = SDL_QUIT;
			ev.quit.type = SDL_QUIT;
			ev.quit.timestamp = SDL_GetTicks();
			SDL_PushEvent(&ev);
			return;
		}

		// MY STUFF END		
		int dir = keyboard[SDL_SCANCODE_LSHIFT] ? -1 : 1;
		if(keyboard[SDL_SCANCODE_1]){			
			selected_box_index += dir;
			if(selected_box_index < 0){
				selected_box_index = num_boxes - 1;
			} else if(selected_box_index >= num_boxes){
				selected_box_index = 0;				
			}
			selected_box = &boxes[selected_box_index];
		}
		if(keyboard[SDL_SCANCODE_2]){			
			selected_box = nullptr;
		}
		if(keyboard[SDL_SCANCODE_3]){
			if(selected_box != nullptr){
				selected_box->rotate(dir*0.05f);
				selected_box->calculate_normals();
			}
		}		
		if(keyboard[SDL_SCANCODE_4]){
			GLOBAL_AXIS.apply_2d_rotation(dir*0.05f);			
		}		
	} else if(ev.type == SDL_KEYUP){	
	}
}
void View_Play::handle_mouse_events(SDL_Event& ev){
	mouse.update();
	button = SDL_GetMouseState(&mouse_x, &mouse_y);
	
	static int count = 0;
	if(ev.type == SDL_MOUSEBUTTONDOWN){
		drag_flag = true;
		
	} else if(ev.type == SDL_MOUSEBUTTONUP){
		drag_flag = false;
	
	} else if(ev.type == SDL_MOUSEMOTION){
		++count;
		if(drag_flag){
			if(selected_box != nullptr){
				selected_box->mouse_drag(ev);
				selected_box->calculate_normals();
			}	
		} else{			
	
		}		
	}
}

void View_Play::handle_user_events(SDL_Event& ev){
	if(ev.type >= SDL_USEREVENT){
		// handle a frame per 1/30 the of a second
		if(ev.user.type == TimerFactory::get().event_type()){
			draw_flag = true;
		} else if(ev.user.type == Util::get().get_userev("dialog_close_event")){			
		} else{
		}
	}
}

void View_Play::update(SDL_Event& ev){
	// do something for every user event	
}

void View_Play::render(){		
	SDL_RenderClear(&ren);

	boxes[0].set_collided(false);
	boxes[1].set_collided(false); 
	if(boxes[0].collides(boxes[1])){
		boxes[0].set_collided(true);
		boxes[1].set_collided(true);
	}

	//render the axis line
	SDL_Color axis_colour = { 10, 170, 30, 145 };
	Vector3d axis = GLOBAL_AXIS;
	axis *= (int)(disp_w*0.4);	
	int start_x = (int) -axis.x() + 320;
	int start_y = (int) -axis.y() + 320;
	int end_x	= (int)  axis.x() + 320;
	int end_y	= (int)  axis.y() + 320;
	Util::render_line(
		&ren, axis_colour,
		start_x, start_y,
		end_x, end_y
		);

	// render all the boxes
	for(int i = 0; i < num_boxes; ++i){
		boxes[i].render(ren, 0, 0);
	}
	// render origin;
	SDL_Rect rect = {
		0 + 320-3,
		0 + 320-3,
		6,6
	};
	SDL_Color redish = { 180, 10, 10, 110 };
	Util::render_fill_rectangle(&ren, &rect, redish);

	// render some text
	Util::render_text(&ren, font_carbon_12, 5, 5, font_carbon_12_colour, "selected = %d, %x", selected_box_index,selected_box);		
	SDL_RenderPresent(&ren);
}


void BB::render(SDL_Renderer& ren, int x, int y){
	SDL_Color focused_colour = { 20, 90, 20, 160 };
	Vector3d translation(320,320);

	for(int i = 0; i < num_points; ++i){
		// render faces
		Util::render_line(
			&ren, collided_colour,
			(int)(vectors[i].x() + center.x() + x + translation.x()),
			(int)(vectors[i].y() + center.y() + y + translation.y()),
			(int)(vectors[(i + 1) % num_points].x() + center.x() + x + translation.x()),
			(int)(vectors[(i + 1) % num_points].y() + center.y() + y + translation.y())
			);

		// render corner dots
		SDL_Rect center_rect = {
			(int)(vectors[i].x() - 3 + center.x() + x + translation.x()),
			(int)(vectors[i].y() - 3 + center.y() + y + translation.y()),
			6, 6
		};
		Util::render_fill_rectangle(&ren, &center_rect, collided_colour);
	}

	// render center dot
	SDL_Rect center_rect = {
		(int)(center.x() + x - 3+ translation.x()),
		(int)(center.y() + y - 3+ translation.y()),
		6, 6
	};
	Util::render_fill_rectangle(&ren, &center_rect, focused_colour);

	// render min and max dots
	Vector3d axis = GLOBAL_AXIS;
	int min = min_along_axis(axis);
	int max = max_along_axis(axis);

	// minx dot
	SDL_Color min_colour = { 180, 80, 20, 140 };
	center_rect = {
		(int)(vectors[min].x() + x + center.x() - 3  + translation.x()),
		(int)(vectors[min].y() + y + center.y() - 3  + translation.y()),
		6, 6
	};
	Util::render_fill_rectangle(&ren, &center_rect, min_colour);

	// max dot
	SDL_Color max_colour = { 20, 80, 180, 140 };
	center_rect = {
		(int)(vectors[max].x() + x + center.x() - 3 + translation.x()),
		(int)(vectors[max].y() + y + center.y() - 3 + translation.y()),
		6, 6
	};
	Util::render_fill_rectangle(&ren, &center_rect, max_colour);


	SDL_Color axis_colour	= { 255, 0, 0, 255 };
	Vector3d start_vector	= (vectors[min] + center).projection_vector(GLOBAL_AXIS);
	Vector3d end_vector		= (vectors[max] + center).projection_vector(GLOBAL_AXIS);	
	start_vector += translation;
	end_vector += translation;

	/*
	printf("[%0.2f,%0.2f]  [%0.2f,%0.2f]     \r",
		start_vector.x(),
		start_vector.y(),
		end_vector.x(),
		end_vector.y()
		);
	*/

	int center_x = 640 / 2;
	int center_y = 640 / 2;
	int start_x = (int) start_vector.x();
	int start_y = (int) start_vector.y();
	int end_x	= (int) end_vector.x();
	int end_y	= (int) end_vector.y();
	Util::render_line(
		&ren, axis_colour,
		start_x, start_y,
		end_x, end_y
		);
}