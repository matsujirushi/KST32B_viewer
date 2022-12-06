#pragma once

#include "Config.h"
#include <stdint.h>
#include "FileSystem.h"

extern char filename[FILE_NUM_MAX][_MAX_LFN+1];

unsigned short sjis2jis(unsigned short c);
unsigned short char2hex4(char *c);
unsigned short listDir(fs::FS& fs, const char* dirname, uint8_t levels);
