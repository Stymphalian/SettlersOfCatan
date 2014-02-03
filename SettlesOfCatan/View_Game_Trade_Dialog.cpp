#include "IDialog.h"
#include "View_Game.h"
#include "View_Game_Trade_Dialog.h"



View_Game_Trade_Dialog::View_Game_Trade_Dialog(View_Game& view, int x, int y, int z, int w, int h)
: IDialog(view, x, y, z, w, h), view_game(view)
{

}

View_Game_Trade_Dialog::~View_Game_Trade_Dialog()
{

}