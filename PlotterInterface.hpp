#pragma once

#include <cstdint>

class PlotterInterface
{
public:
    virtual ~PlotterInterface(){}
    
    virtual void PenDown() = 0;
    virtual void PenUp() = 0;
    virtual void MovePos(int16_t x, int16_t y) = 0;
    virtual void DrawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2) = 0;
};
