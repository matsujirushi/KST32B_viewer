#pragma once

#include <cstdbool>
#include <cstdint>
#include "PlotterInterface.hpp"

class DisplayPlotter : public PlotterInterface
{
private:
    int16_t X_;
    int16_t Y_;
    bool PenDown_;

public:
    DisplayPlotter();
    ~DisplayPlotter();

    void PenDown();
    void PenUp();
    void MovePos(int16_t x, int16_t y);
    void DrawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2);

};
