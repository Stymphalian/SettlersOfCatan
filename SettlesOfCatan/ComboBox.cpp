#include "Logger.h"
#include "ComboBox.h"


ComboBox::ComboBox(){
	Logger::getLog().log(Logger::DEBUG, "ComboBox constructor");
}

ComboBox::~ComboBox(){
	Logger::getLog().log(Logger::DEBUG, "ComboBox destructor");
}
