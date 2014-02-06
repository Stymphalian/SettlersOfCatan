#include "Logger.h"
#include "DropDown.h"


DropDown::DropDown(){
	Logger::getLog().log(Logger::DEBUG, "DropDown constructor");
}


DropDown::~DropDown()
{
	Logger::getLog().log(Logger::DEBUG, "DropDown destructor");
}
