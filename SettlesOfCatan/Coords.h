#pragma once

class ICoords{
public:
	ICoords(){}
	virtual ~ICoords(){}
	virtual int x()= 0;
	virtual int y()= 0;
	virtual int z()= 0;
	virtual void x(int value)= 0;
	virtual void y(int value)= 0;
	virtual void z(int value)= 0;
	virtual int w()= 0;
	virtual int h()= 0;
	virtual void w(int value)=0;
	virtual void h(int value)=0;
	virtual int disp_x()=0;
	virtual int disp_y()=0;
	virtual int disp_z()=0;
	virtual void make_within_bounds(ICoords* parent)=0;
};

class Coords : public ICoords{
public:
	Coords(ICoords* parent=nullptr);
	virtual ~Coords();
	// ICoords Interface
	int x();
	int y();
	int z();
	int w();
	int h();
	void x(int value);
	void y(int value);
	void z(int value);
	void w(int value);
	void h(int value);
	int disp_x(); // this seems reasonable
	int disp_y();
	int disp_z();
	void make_within_bounds(ICoords* parent);

	// Coords
	void init(int x, int y, int z, int w, int h);
	void set_parent(ICoords* parent);
	ICoords* get_parent();
	void disable_relative_data();
	void set_relative_x(float percent);
	void set_relative_y(float percent);
	void set_relative_w(float percent);
	void set_relative_h(float percent);
	void set_relative_x(int px_offset);
	void set_relative_y(int px_offset);
	void set_relative_w(int px_offset);
	void set_relative_h(int px_offset);
private:	
	class relative_coordinate_data{
	public:
		enum rel_e{
			RELX = 0x01, RELY = 0x02, RELW = 0x04, RELH = 0x08,
			RELX_px = 0x10, RELY_px = 0x20, RELW_px = 0x40, RELH_px = 0x80
		};
		unsigned relative_mask;
		float rel_pos[4]; // x, y, w, h

		void set_mask(unsigned mask){
			relative_mask |= mask;
		}
		void unset_mask(unsigned mask){
			relative_mask = relative_mask &  ~(mask);
		}
	};
	relative_coordinate_data* rel_data;
	void set_relative(int pos, unsigned smask, unsigned umask,float value);

	ICoords* parent;
	int _x, _y, _z, _w, _h;		
};