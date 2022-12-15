#include "DisplayPlotter.hpp"
#include "Display.h"

DisplayPlotter::DisplayPlotter() :
    X_{ 0 },
    Y_{ 0 },
    PenDown_{ false }
{
}

DisplayPlotter::~DisplayPlotter()
{
}

void DisplayPlotter::PenDown()
{
    PenDown_ = true;
}

void DisplayPlotter::PenUp()
{
    PenDown_ = false;
}

void DisplayPlotter::MovePos(int16_t x, int16_t y)
{
    if (PenDown_)
    {
        tft.drawLine(X_, Y_, x, y, TFT_WHITE);
    }

    X_ = x;
    Y_ = y;
}

void DisplayPlotter::DrawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2)
{
    PenUp();
    MovePos(x1, y1);
    PenDown();
    MovePos(x2, y2);
}
