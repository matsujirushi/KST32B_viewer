#include "Config.h"
#include "plotter.h"

#include "swSerial.h"

static unsigned char  plotter_pen_pos = 0;
static signed short   plotter_pen_x   = -1;
static signed short   plotter_pen_y   = -1;

void plotter_penDown(void)
{
    swSerial.write('D');
    while ( swSerial.read() != 0x55 );
    plotter_pen_pos = 1;
}

void plotter_penUp(void)
{
    swSerial.write('U');
    while ( swSerial.read() != 0x55 );    
    plotter_pen_pos = 0;
}

void plotter_movePos(signed short x, signed short y)
{
    swSerial.write('M');
    swSerial.write((x>>8)&0xFF);
    swSerial.write(x&0xFF);
    swSerial.write((y>>8)&0xFF);
    swSerial.write(y&0xFF);
    while ( swSerial.read() != 0x55 );
    plotter_pen_x = x;
    plotter_pen_y = y;
}

void plotter_drawLine(signed short x1, signed short y1, signed short x2, signed short y2)
{
#if 0
    if ( plotter_pen_x != x1 || plotter_pen_y != y1 ) {
        if ( plotter_pen_pos == 1 ) {
            plotter_penUp();
        }
        plotter_movePos(x1, y1);
        plotter_penDown();
    }
#else
    if ( plotter_pen_x != x1 || plotter_pen_y != y1 ) {
        plotter_penUp();
        plotter_movePos(x1, y1);
    }
#endif
    plotter_penDown();
    plotter_movePos(x2, y2);
//    plotter_penUp();
}
