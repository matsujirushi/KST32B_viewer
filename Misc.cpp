#include "Config.h"
#include "Misc.h"

#include <Arduino.h>

char filename[FILE_NUM_MAX][_MAX_LFN+1];

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

unsigned short char2hex4(char *c)
{
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

unsigned short listDir(fs::FS& fs, const char* dirname, uint8_t levels)
{
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
