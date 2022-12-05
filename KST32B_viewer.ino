// KST32B Viewer for Wio Terminal
// 2020.05.23 Kyosuke Ishikawa

#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();
#define TFT_WIDTH   320
#define TFT_HEIGHT  240

#define PLOTTER_WIDTH   500

#include <Seeed_FS.h>
#include "SD/Seeed_SD.h"

#include <SoftwareSerial.h>
SoftwareSerial swSerial(D0, D1);

//#define PLOTTER_CONTROL

#define KST32B_WIDTH_D  32
#define KST32B_WIDTH_S  16
#define KST32B_HEIGHT   32

#define FILE_NUM_MAX  32
#define FILE_DISP_NUM_MAX  14
char filename[FILE_NUM_MAX][_MAX_LFN+1];
unsigned short  file_select;
unsigned short  file_disp_start;

unsigned char   hpgl_state;
unsigned short  hpgl_x,  hpgl_y;
unsigned short  hpgl_x0, hpgl_y0;
unsigned short  hpgl_x1, hpgl_y1;
unsigned char   hpgl_pen;
unsigned char   hpgl_plot;
signed short    hpgl_x_max, hpgl_x_min;
signed short    hpgl_y_max, hpgl_y_min;


unsigned short sjis2jis(unsigned short c)
{
    unsigned char  c1, c2;
    
    c1 = (c & 0xFF00)>>8;
    c2 = c & 0x00FF;
    c1 = c1 << 1;
    if( c2 < 0x9F ) {
        if (c1 < 0x3F) {
            c1 = c1 + 0x1F;
        } else {
            c1 = c1 - 0x61;
        }
        if (c2 > 0x7E) {
            c2 = c2 - 0x20;
        } else {
            c2 = c2 - 0x1F;
        }
    } else {
        if (c1 < 0x3F) {
            c1 = c1 + 0x20;
        } else {
            c1 = c1 - 0x60;
        }
        c2 = c2 - 0x7E;
    }
    return (unsigned short)(c1*0x100+c2);
}

unsigned short char2hex4(char *c) {
    unsigned char  i;
    unsigned short v;
    
    v = 0;
    for ( i=0 ; i<4 ; i++ ) {
        v = v * 0x10;
        if ( *c >= 'A' && *c <= 'F' ) {
            v = v + (*c - 'A' + 10);
        } else if ( *c >= '0' && *c <= '9' ) {
            v = v + (*c - '0');
        }
        c++;
    }
    return v;
}

void KST32B_drawChar(unsigned short code, signed short x, signed short y, double p) {
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
                        tft.drawLine((int32_t)(x+x1*p), (int32_t)(y+(KST32B_HEIGHT-y1)*p), (int32_t)(x+x2*p), (int32_t)(y+(KST32B_HEIGHT-y2)*p), TFT_WHITE);
                        x1 = x2;
                    } else if ( c >= 0x5E && c <= 0x5F ) {
                        x2 = c - 0x5E + 28;
                        tft.drawLine((int32_t)(x+x1*p), (int32_t)(y+(KST32B_HEIGHT-y1)*p), (int32_t)(x+x2*p), (int32_t)(y+(KST32B_HEIGHT-y2)*p), TFT_WHITE);
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
                        tft.drawLine((int32_t)(x+x1*p), (int32_t)(y+(KST32B_HEIGHT-y1)*p), (int32_t)(x+x2*p), (int32_t)(y+(KST32B_HEIGHT-y2)*p), TFT_WHITE);
                        x1 = x2;
                        y1 = y2;
                    }
                }
            }
        }
    }
}

void KST32B_drawString(unsigned char *s, signed short x, signed short y, double p) {
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
#if 0
        if ( (signed short)(x+w*p) > TFT_WIDTH ) {
            x = 0;
            y = y + (signed short)(KST32B_HEIGHT*p);
        }
#endif
        if ( code != 0x20 ) {
            KST32B_drawChar(code, x, y, p);
        }
        x = x + (signed short)(w*p);
    }
}

unsigned char  plotter_pen_pos = 0;
signed short   plotter_pen_x   = -1;
signed short   plotter_pen_y   = -1;

void plotter_penDown(void) {
    swSerial.write('D');
    while ( swSerial.read() != 0x55 );
    plotter_pen_pos = 1;
}

void plotter_penUp(void) {
    swSerial.write('U');
    while ( swSerial.read() != 0x55 );    
    plotter_pen_pos = 0;
}

void plotter_movePos(signed short x, signed short y) {
    swSerial.write('M');
    swSerial.write((x>>8)&0xFF);
    swSerial.write(x&0xFF);
    swSerial.write((y>>8)&0xFF);
    swSerial.write(y&0xFF);
    while ( swSerial.read() != 0x55 );        
    plotter_pen_x = x;
    plotter_pen_y = y;
}

void plotter_drawLine(signed short x1, signed short y1, signed short x2, signed short y2) {
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

void KST32B_plotChar(unsigned short code, signed short x, signed short y, double p) {
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
                        plotter_penUp();
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
                        swSerial.write('D');
                        while ( swSerial.read() != 0x55 );
                        plotter_drawLine((int32_t)(x+x1*p), (int32_t)(y+(KST32B_HEIGHT-y1)*p), (int32_t)(x+x2*p), (int32_t)(y+(KST32B_HEIGHT-y2)*p));
//                        tft.drawLine((int32_t)(x+x1*p), (int32_t)(y+(KST32B_HEIGHT-y1)*p), (int32_t)(x+x2*p), (int32_t)(y+(KST32B_HEIGHT-y2)*p), TFT_WHITE);
                        x1 = x2;
                    } else if ( c >= 0x5E && c <= 0x5F ) {
                        x2 = c - 0x5E + 28;
                        plotter_drawLine((int32_t)(x+x1*p), (int32_t)(y+(KST32B_HEIGHT-y1)*p), (int32_t)(x+x2*p), (int32_t)(y+(KST32B_HEIGHT-y2)*p));
//                        tft.drawLine((int32_t)(x+x1*p), (int32_t)(y+(KST32B_HEIGHT-y1)*p), (int32_t)(x+x2*p), (int32_t)(y+(KST32B_HEIGHT-y2)*p), TFT_WHITE);
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
                        plotter_drawLine((int32_t)(x+x1*p), (int32_t)(y+(KST32B_HEIGHT-y1)*p), (int32_t)(x+x2*p), (int32_t)(y+(KST32B_HEIGHT-y2)*p));
//                        tft.drawLine((int32_t)(x+x1*p), (int32_t)(y+(KST32B_HEIGHT-y1)*p), (int32_t)(x+x2*p), (int32_t)(y+(KST32B_HEIGHT-y2)*p), TFT_WHITE);
                        x1 = x2;
                        y1 = y2;
                    }
                }
            }
        }
    }
    plotter_penUp();
}

void KST32B_plotString(unsigned char *s, signed short x, signed short y, double p) {
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
        if ( (signed short)(x+w*p) > PLOTTER_WIDTH ) {
            x = 0;
            y = y + (signed short)((KST32B_HEIGHT+1)*p);
        }
        if ( code != 0x20 ) {
            KST32B_plotChar(code, x, y, p);
        }
        x = x + (signed short)(w*p);
    }
}

void hpgl_init(void)
{
    hpgl_plot  = 0;
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

unsigned short listDir(fs::FS& fs, const char* dirname, uint8_t levels) {
    unsigned short num = 0;
    
    Serial.print("Listing directory: ");
    Serial.println(dirname);

    File root = fs.open(dirname);
    if (!root) {
        Serial.println("Failed to open directory");
        return 0;
    }
    if (!root.isDirectory()) {
        Serial.println("Not a directory");
        return 0;
    }

    File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            Serial.print("  DIR : ");
            Serial.println(file.name());
//            if (levels) {
//                listDir(fs, file.name(), levels - 1);
//            }
//            strcpy((char *)filename[num], (char *)(file.name()+1));
//            num++;
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
//            strcpy((char *)filename[num], (char *)(file.name()+1));
            if ( num < FILE_NUM_MAX ) {
                strcpy(filename[num], (char *)(file.name()+1));
                num++;
            }
        }
        file = root.openNextFile();
    }
    return num;
}

void setup() {
	
    pinMode(WIO_5S_UP, INPUT_PULLUP);
    pinMode(WIO_5S_DOWN, INPUT_PULLUP);
    pinMode(WIO_5S_LEFT, INPUT_PULLUP);
    pinMode(WIO_5S_RIGHT, INPUT_PULLUP);
    pinMode(WIO_5S_PRESS, INPUT_PULLUP);
    
    tft.init();
    tft.setRotation(3);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    
    Serial.begin(115200);
    swSerial.begin(9600);
    while (!SD.begin(SDCARD_SS_PIN, SDCARD_SPI)) {
        Serial.println("Card Mount Failed");
        return;
    }
    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
        Serial.println("No SD card attached");
        return;
    }
    uint64_t cardSize = SD.cardSize() / (1024 * 1024);

    Serial.print("SD Card Size: ");
    Serial.print((uint32_t)cardSize);
    Serial.println("MB");

    file_select     = 0;
    file_disp_start = 0;
}

void loop() {
    unsigned char  c, flg;
    unsigned short i, n;
    unsigned char  ss[5];
    
    tft.fillScreen(TFT_BLACK);
    KST32B_drawString((unsigned char *)" Wio Terminal Plotter", 0, 0, 0.8);
    KST32B_drawString((unsigned char *)" $B$3$NG=NO$OI=<($G$-$k$+$J(B", 0, 28, 0.8);    while (digitalRead(WIO_5S_PRESS) != 0);
    delay(100);
    while (digitalRead(WIO_5S_PRESS) == 0);
    
    plotter_penUp();
    plotter_movePos(0, 0);
//    KST32B_plotString((unsigned char *)"Wio Terminal Polotter", 0, 0, 1.5);
    KST32B_plotString((unsigned char *)"$B~A(I=$B~A(I=$B~A(I=(Bj$B~A(I=$B~A(I=$B~A(I=$B~A(I=$B~A(I=$B~A(I=$B~A(I=$B~A(I=$B~A(I=(Bj$B~A(I=$B~A(I=(B", 0,   0, 1.5);
    KST32B_plotString((unsigned char *)"  $B~A(I=$B~A(I=(B          $B~A(I=(Bp  ", 0,  60, 1.5);
    KST32B_plotString((unsigned char *)"$B~A(I=$B~A(I=$B~A(I=$B~A(I=$B~A(I=$B~A(I=$B~A(I=$B~A(I=$B~A(I=$B~A(I=$B~A(I=$B~A(I=$B~A(I=$B~A(I=$B~A(I=$B~A(I=$B~A(I=$B~A(I=(B", 0, 120, 1.5);
    KST32B_plotString((unsigned char *)"$B~A(I=$B~A(I=$B~A(I=$B~A(I=$B~A(I=$B~A(I=$B~A(I=$B~A(I=$B~A(I=$B~A(I=$B~A(I=$B~A(I=$B~A(I=$B~A(I=$B~A(I=$B~A(I=$B~A(I=$B~A(I=(B", 0, 330, 1.5);
    KST32B_plotString((unsigned char *)"  $B~A(I=(Bp          $B~A(I=$B~A(I=(B  ", 0, 390, 1.5);
    KST32B_plotString((unsigned char *)"$B~A(I=$B~A(I=$B~A(I=(Bj$B~A(I=$B~A(I=$B~A(I=$B~A(I=J$B6q(I?=$B~A(I=$B~A(I=(Bj$B~A(I=$B~A(I=(B", 0, 450, 1.5);
    while (digitalRead(WIO_5S_PRESS) != 0);
    delay(100);
    while (digitalRead(WIO_5S_PRESS) == 0);
    
    tft.fillScreen(TFT_BLACK);
    tft.setFreeFont(&FreeSans9pt7b);
    n = listDir(SD, "/", 0);
    if ( n == 0 ) {
        return;
    }
    flg = 0;
    while ( flg == 0 ) {
        tft.fillScreen(TFT_BLACK);
        for ( i=file_disp_start ; i<n ; i++ ) {
            if ( i == file_select ) {
                tft.setTextColor(TFT_BLACK, TFT_WHITE);
            } else {
                tft.setTextColor(TFT_WHITE, TFT_BLACK);
            }
            if ( i < file_disp_start + FILE_DISP_NUM_MAX ) {
                tft.drawString((char *)filename[i], 0, 16*(i-file_disp_start));
            }
        }
        i = 0;
        while ( i == 0 ) {
            if ( digitalRead(WIO_5S_PRESS) == 0 ) {
                flg = 1;
                i = 1;
                while ( digitalRead(WIO_5S_PRESS) == 0 );
            } else if ( digitalRead(WIO_5S_UP) == 0 && file_select > 0 ) {
                if ( file_select > file_disp_start ) {
                    tft.setTextColor(TFT_WHITE, TFT_BLACK);
                    tft.drawString((char *)filename[file_select], 0, 16*(file_select-file_disp_start));
                    file_select--;
                    tft.setTextColor(TFT_BLACK, TFT_WHITE);
                    tft.drawString(filename[file_select], 0, 16*(file_select-file_disp_start));
                    while ( digitalRead(WIO_5S_UP) == 0 );
                    i = 0;
                } else {
                    file_select--;
                    file_disp_start--;
                    i = 1;
                }
            } else if ( digitalRead(WIO_5S_DOWN) == 0 && file_select < n-1 ) {
                if ( file_select < file_disp_start + FILE_DISP_NUM_MAX - 1 ) {
                    tft.setTextColor(TFT_WHITE, TFT_BLACK);
                    tft.drawString((char *)filename[file_select], 0, 16*(file_select-file_disp_start));
                    file_select++;
                    tft.setTextColor(TFT_BLACK, TFT_WHITE);
                    tft.drawString((char *)filename[file_select], 0, 16*(file_select-file_disp_start));
                    while ( digitalRead(WIO_5S_DOWN) == 0 );
                    i = 0;
                } else {
                    file_select++;
                    file_disp_start++;
                    i = 1;
                }
            }
        }
        
    }
    
    tft.fillScreen(TFT_BLACK);
    
    File file = SD.open((const char *)filename[file_select]);
    if (!file) {
        Serial.println("Failed to open file for reading");
    } else {
        hpgl_init();
        while (file.available()) {
            c = file.read();
            hpgl_process(c);
        }
        file.close();
//        swSerial.print("E");
//        while ( swSerial.read() != 0x55 );
    }
    while (digitalRead(WIO_5S_PRESS) != 0);
    delay(100);
    while (digitalRead(WIO_5S_PRESS) == 0);
}
