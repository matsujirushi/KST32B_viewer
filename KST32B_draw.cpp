#include "Config.h"
#include "KST32B_draw.h"

#include "FileSystem.h"
#include "Misc.h"
#include "Display.h"

static void KST32B_drawChar(unsigned short code, signed short x, signed short y, double p)
{
    char           strLine[256];
    unsigned char  c, ptr;
    unsigned char  x1, y1, x2, y2;
    int            d;
    
    File fp = SD.open("/kst32b.txt");
    while(d = fp.read()) {
        ptr = 0;
        strLine[ptr++] = (unsigned char)d;
        do {
            d = fp.read();
            strLine[ptr++] = (unsigned char)d;
        } while ( d != 0x0A );
        if ( strLine[0] >= '0' && strLine[0] <= '9' ) {
            if ( char2hex4((char *)strLine) == code ) {
                ptr = 5;
                x1 = 0;
                y1 = 0;
                x2 = 0;
                x2 = 0;
                for ( ptr = 5 ; ptr < 155+5 ; ptr++ ) {
                    c = strLine[ptr];
                    if ( c == 0x20 ) {
                        return;
                    } else if ( c == 0x27 || c == 0x5C || c == 0x5D ) {
                    } else if ( c >= 0x21 && c <= 0x26 ) {
                        x1 = c - 0x21 + 0;
                        x2 = x1;
                    } else if ( c >= 0x28 && c <= 0x3F ) {
                        x1 = c - 0x28 + 6;
                        x2 = x1;
                    } else if ( c >= 0x40 && c <= 0x5B ) {
                        x2 = c - 0x40 + 0;
// Here! ---
                        tft.drawLine((int32_t)(x+x1*p), (int32_t)(y+(KST32B_HEIGHT-y1)*p), (int32_t)(x+x2*p), (int32_t)(y+(KST32B_HEIGHT-y2)*p), TFT_WHITE);
// ---------
                        x1 = x2;
                    } else if ( c >= 0x5E && c <= 0x5F ) {
                        x2 = c - 0x5E + 28;
// Here! ---
                        tft.drawLine((int32_t)(x+x1*p), (int32_t)(y+(KST32B_HEIGHT-y1)*p), (int32_t)(x+x2*p), (int32_t)(y+(KST32B_HEIGHT-y2)*p), TFT_WHITE);
// ---------
                        x1 = x2;
                    } else if ( c >= 0x60 && c <= 0x7D ) {
                        x2 = c - 0x60 + 0;
                    } else if ( c == 0x7E ) {
                        y1 = c - 0x7E + 0;
                        y2 = y1;
                    } else if ( c >= 0xA1 && c <= 0xBF ) {
                        y1 = c - 0xA1 + 1;
                        y2 = y1;
                    } else if ( c >= 0xC0 && c <= 0xDF ) {
                        y2 = c - 0xC0 + 0;
// Here! ---
                        tft.drawLine((int32_t)(x+x1*p), (int32_t)(y+(KST32B_HEIGHT-y1)*p), (int32_t)(x+x2*p), (int32_t)(y+(KST32B_HEIGHT-y2)*p), TFT_WHITE);
// ---------
                        x1 = x2;
                        y1 = y2;
                    }
                }
            }
        }
    }
}

void KST32B_drawString(unsigned char *s, signed short x, signed short y, double p)
{
    unsigned short  code;
    unsigned char   w;

    while ( *s != 0x00 ) {
        if ((*s >= 0x81 && *s < 0x9F) || (*s >= 0xE0 && *s < 0xFC)) {
            // 2byte code
            code = *s;
            s++;
            code = code * 0x100 + *s;
            s++;
            code = sjis2jis(code);
            w = KST32B_WIDTH_D;
        } else {
            // 1byte code
            code = *s;
            s++;
            w = KST32B_WIDTH_S;
        }
// Here! ---
#if 0
        if ( (signed short)(x+w*p) > TFT_WIDTH ) {
            x = 0;
            y = y + (signed short)(KST32B_HEIGHT*p);
        }
#endif
// ---------
        if ( code != 0x20 ) {
// Here! ---
            KST32B_drawChar(code, x, y, p);
// ---------
        }
        x = x + (signed short)(w*p);
    }
}
