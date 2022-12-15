// KST32B Viewer for Wio Terminal
// 2020.05.23 Kyosuke Ishikawa

// Libraries:
//   Seeed Arduino FS 2.0.3 - https://github.com/Seeed-Studio/Seeed_Arduino_FS

////////////////////////////////////////////////////////////////////////////////
// Includes

#include "Config.h"

#include "FileSystem.h"
#include "Misc.h"
#include "plotter.h"
#include "swSerial.h"
#include "KST32B_draw.h"
#include "KST32B_plot.h"
#include "hpgl.h"
#include "Display.h"

////////////////////////////////////////////////////////////////////////////////
// Variables

static unsigned short  file_select;
static unsigned short  file_disp_start;

////////////////////////////////////////////////////////////////////////////////
// setup and loop

void setup()
{
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

void loop()
{
    unsigned char  c, flg;
    unsigned short i, n;
    unsigned char  ss[5];
    
    tft.fillScreen(TFT_BLACK);
    KST32B_drawString((unsigned char *)" Wio Terminal Plotter", 0, 0, 0.8);
    KST32B_drawString((unsigned char *)" with KST32B", 0, 28, 0.8);
    while (digitalRead(WIO_5S_PRESS) != 0);
    delay(100);
    while (digitalRead(WIO_5S_PRESS) == 0);
    
    plotter_penUp();
    plotter_movePos(0, 0);
//    KST32B_plotString((unsigned char *)"Wio Terminal Polotter", 0, 0, 1.5);
    KST32B_plotString((unsigned char *)"香桂銀金王金銀桂香", 0,   0, 1.5);
    KST32B_plotString((unsigned char *)"  飛          角  ", 0,  60, 1.5);
    KST32B_plotString((unsigned char *)"歩歩歩歩歩歩歩歩歩", 0, 120, 1.5);
    KST32B_plotString((unsigned char *)"歩歩歩歩歩歩歩歩歩", 0, 330, 1.5);
    KST32B_plotString((unsigned char *)"  角          飛  ", 0, 390, 1.5);
    KST32B_plotString((unsigned char *)"香桂銀金玉金銀桂香", 0, 450, 1.5);
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

////////////////////////////////////////////////////////////////////////////////
