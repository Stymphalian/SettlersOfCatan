#include "Logger.h"

#include "IDialog.h"
#include "View_Game_DevCard_Dialog.h"


View_Game_DevCard_Dialog::View_Game_DevCard_Dialog()
:IDialog(0,0,0,0,0)
{
	Logger::getLog().log(Logger::DEBUG, "View_Game_DevCard_Dialog constructor");
}

View_Game_DevCard_Dialog::~View_Game_DevCard_Dialog(){
	Logger::getLog().log(Logger::DEBUG, "View_Game_DevCard_Dialog destructor");
}

View_Game_DecCard_Dialog::init()
