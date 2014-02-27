//-------------------------------------------------
// I F O C U S A B L E
//-------------------------------------------------
class IFocusable{
public:
	virtual ~IFocusable(){}
	virtual bool has_focus() = 0;
	virtual void set_focus(bool value) = 0;
	virtual void on_focus() = 0;
	virtual void off_focus() = 0;
};

//-------------------------------------------------
// F O C U S A B L E
//-------------------------------------------------
class Focusable : public IFocusable{
public:
	Focusable();
	virtual ~Focusable();		
	virtual bool has_focus();
	virtual void set_focus(bool value);
	virtual void on_focus() = 0;
	virtual void off_focus() = 0;
protected:
	bool _focus_flag;	
};

//-------------------------------------------------
// N U L L _ F O C U S A B L E
//-------------------------------------------------
class NULL_Focusable : public IFocusable{
public:	
	static NULL_Focusable& get(){
		static NULL_Focusable instance;
		return instance;
	}
	static bool isDummy(void* object){
		return (&NULL_Focusable::get() == object);
	}
	virtual bool has_focus(){ return false; }
	virtual void set_focus(bool value){}
	virtual void on_focus(){}
	virtual void off_focus(){};	
private:
	NULL_Focusable(){}
	virtual ~NULL_Focusable(){}
	NULL_Focusable(const NULL_Focusable&);
	void operator= (NULL_Focusable const&);
};
