#pragma once

#include "IDialog.h"

class View_Game_DevCard_Dialog : public IDialog
{
public:
	View_Game_DevCard_Dialog();
	virtual ~View_Game_DevCard_Dialog();

	void init(View_Game& view,int x, int y, int z, int w, int h);

private:
};

