#pragma once
#include <vector>


class Observable;

class IObserver{
public:	
	virtual void update(Observable* origin, void* data) = 0;
private:
};

class Observable{
public:
	Observable();
	virtual ~Observable();

	bool subscribe(IObserver* observer);
	bool unsubscribe(IObserver* observer);
	bool clear_subscribers();
	unsigned num_subscribers();
	bool has_changed();
	void notify(void* data);	
protected:	
	void mark_changed();
	void unmark_changed();	
private:	
	bool _changed;	
	std::vector<IObserver*> observers;
};

