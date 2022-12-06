#include "Config.h"
#include "hpgl.h"

#include "swSerial.h"
#include "Display.h"

static unsigned char   hpgl_state;
static unsigned char   hpgl_pen;
static signed short    hpgl_x_max, hpgl_x_min;
static signed short    hpgl_y_max, hpgl_y_min;
static unsigned short  hpgl_x,  hpgl_y;
static unsigned short  hpgl_x0, hpgl_y0;

void hpgl_init(void)
{
    hpgl_state = 0;
    hpgl_pen   = 0;
    hpgl_x_max = -1;
    hpgl_y_max = -1;
    hpgl_x_min = 999;
    hpgl_y_min = 999;
}

void hpgl_process(unsigned char c)
{
    unsigned short	x0, y0, x1, y1;
    
    if ( hpgl_state == 0 )	{
        // command first char
        if ( c == 'P' )	{
            hpgl_state = 1;
        }
    } else if ( hpgl_state == 1 )	{
        // command second char
        if ( c == 'U' )	{
            hpgl_pen = 0;
            hpgl_x = 0;
            hpgl_y = 0;
            hpgl_state = 2;
#ifdef PLOTTER_CONTROL
            swSerial.write('U');
            while ( swSerial.read() != 0x55 );
#endif
        } else if ( c == 'D' )	{
            hpgl_pen = 1;
            hpgl_x = 0;
            hpgl_y = 0;
            hpgl_state = 2;
#ifdef PLOTTER_CONTROL
            swSerial.write('D');
            while ( swSerial.read() != 0x55 );
#endif
        } else	{
            hpgl_state = 0;
        }
    } else if ( hpgl_state == 2 )	{
        // number x axis
        if ( c >= '0' && c <= '9' )	{
            hpgl_x = hpgl_x * 10 + c - '0';
        } else if ( c == ',' )	{
            hpgl_state = 3;
#ifdef PLOTTER_CONTROL
            swSerial.write('M');
            swSerial.write(((500-hpgl_x)>>8)&0xFF);
            swSerial.write((500-hpgl_x)&0xFF);
#endif
        } else if ( c != ' ' )	{
            if ( c == 'P' )	{
                hpgl_state = 1;
            } else	{
                hpgl_state = 0;
            }
        }
    } else if ( hpgl_state == 3 )	{
        // number y axis
        if ( c >= '0' && c <= '9' )	{
            hpgl_y = hpgl_y * 10 + c - '0';
        } else if ( c == ',' || c == ';' )	{
            if ( hpgl_pen == 1 )	{
                if ( hpgl_x_max < hpgl_x ) {
                    hpgl_x_max = hpgl_x;
                }
                if ( hpgl_y_max < hpgl_y ) {
                    hpgl_y_max = hpgl_y;
                }
                if ( hpgl_x_min > hpgl_x ) {
                    hpgl_x_min = hpgl_x;
                }
                if ( hpgl_y_min > hpgl_y ) {
                    hpgl_y_min = hpgl_y;
                }
                x0 = (hpgl_x0) * 24 / 50 + (320-240)/2;
                y0 = (500-hpgl_y0) * 24 / 50;
                x1 = (hpgl_x) * 24 / 50 + (320-240)/2;
                y1 = (500-hpgl_y) * 24 / 50;
                tft.drawLine(x0, y0, x1, y1, TFT_WHITE);
            }
#ifdef PLOTTER_CONTROL
            swSerial.write((hpgl_y>>8)&0xFF);
            swSerial.write(hpgl_y&0xFF);
            while ( swSerial.read() != 0x55 );
#endif
            hpgl_x0 = hpgl_x;
            hpgl_y0 = hpgl_y;
            hpgl_x = 0;
            hpgl_y = 0;
            hpgl_state = 2;
        } else if ( c != ' ' )	{
            if ( c == 'P' )	{
                hpgl_state = 1;
            } else	{
                hpgl_state = 0;
            }
        }
    }
}
