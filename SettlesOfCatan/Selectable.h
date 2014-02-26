#pragma once
class ISelectable {
public:
	virtual ~ISelectable(){}
	virtual bool is_selected() = 0;
	virtual void set_selected(bool value) = 0;
protected:
	virtual void on_selected() = 0;
	virtual void off_selected() = 0;
};

class Selectable : public ISelectable{
public:
	Selectable();
	virtual ~Selectable();
	virtual bool is_selected();
	virtual void set_selected(bool value);
protected:
	bool _selected_flag;
	virtual void on_selected();
	virtual void off_selected();
};