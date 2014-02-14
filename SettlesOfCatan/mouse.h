#pragma once
#include <SDL.h>
#include "Collision.h"
#include "Coords.h"

//-------------------------------------------------------
// I M O U S E
//-------------------------------------------------------
class IMouse{
public:	
	virtual ~IMouse(){};
	virtual void update()=0;
	virtual bool left() = 0;
	virtual bool right() = 0;
	virtual bool middle() = 0;
	virtual int x() = 0;
	virtual int y() = 0;
	virtual int x(int value) = 0;
	virtual int y(int value) = 0;	
	virtual Collision&  hitbox() = 0;
};

//-------------------------------------------------------
// I W R A P M O U S E
//-------------------------------------------------------
class IWrapMouse : public IMouse{
protected:
	IMouse* wrapee;
public:
	IWrapMouse(IMouse* wrapee);
	virtual ~IWrapMouse(){};

	virtual void update() = 0;
	virtual bool left();
	virtual bool right();
	virtual bool middle();
	virtual int x();
	virtual int y();
	virtual int x(int value);
	virtual int y(int value);
	virtual Collision&  hitbox();
};


//-------------------------------------------------------
// I M O U S E
//-------------------------------------------------------
class Mouse : public IMouse{
public:
	Mouse();	
	virtual ~Mouse();
	void update();
	bool left();
	bool right();
	bool middle();	
	int x();
	int y();		
	int x(int value);
	int y(int value);
	Collision&  hitbox();
protected:	
	int mouse_x, mouse_y;	
	Uint32 buttons;	
	Collision _hitbox;
};


//-------------------------------------------------------
// R E L M O U S E
//-------------------------------------------------------
class relMouse : public IWrapMouse{
public:
	relMouse(IMouse* m, Coords* offset);
	relMouse(IMouse* m, int xoffset, int yoffset);
	virtual ~relMouse();
	void update();
protected:
	enum rel_mouse_e{COORD, X_OFF};
	rel_mouse_e type;
	Coords* coord;
	int xoff;
	int yoff;
};
