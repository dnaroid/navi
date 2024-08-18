#include <cstdint>
#include <FS.h>
#include "globals.h"
#include <PNGdec.h>
#include <SD.h>

File file;
PNG png;
int xpos = 0;
int ypos = 0;
uint16_t lineBuffer[TILE_SIZE];

void* pngOpen(const char* filename, int32_t* size) {
    file = SD.open(filename);
    if (!file) { print("! Failed to open file: ", filename); }
    *size = file.size();
    return &file;
}

void pngClose(void* handle) {
    if (file) file.close();
}

int32_t pngRead(PNGFILE* handle, uint8_t* buffer, int32_t length) {
    if (!file) return 0;
    return file.read(buffer, length);
}

int32_t pngSeek(PNGFILE* handle, int32_t position) {
    if (!file) return 0;
    return file.seek(position);
}

#ifdef DISABLE_TILE_CACHE

void pngDraw(PNGDRAW* pDraw) {
    png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
    if (pDraw->y + ypos >= SCREEN_HEIGHT) return;
    if (xpos >= SCREEN_WIDTH) return;
    int drawWidth = pDraw->iWidth;
    if (xpos + drawWidth > SCREEN_WIDTH) { drawWidth = SCREEN_WIDTH - xpos; }
    mapSprite.pushImage(xpos, ypos + pDraw->y, drawWidth, 1, lineBuffer);
}

void drawPngTile(const char* filename, int x, int y) {
    xpos = x;
    ypos = y;
    int16_t rc = png.open(filename, pngOpen, pngClose, pngRead, pngSeek, pngDraw);
    if (rc == PNG_SUCCESS) {
        rc = png.decode(NULL, 0);
        png.close();
    } else {
        LOG("! Failed to decode PNG: ", rc);
    }
}
#else

int cacheIndex = 0;
int cacheFreeSlot = 0;

struct TileCacheEntry {
    char filename[MAX_FILENAME_LENGTH];
    uint16_t* data;
};

TileCacheEntry* tileCache = nullptr;

TileCacheEntry* findInCache(const char* filename) {
    for (int i = 0; i < MAX_CACHE_SIZE; i++) {
        if (strcmp(tileCache[i].filename, filename) == 0) {
            return &tileCache[i];
        }
    }
    return nullptr;
}

void initializeCache() {
    tileCache = new TileCacheEntry[MAX_CACHE_SIZE];
    for (int i = 0; i < MAX_CACHE_SIZE; i++) {
        tileCache[i].data = new uint16_t[TILE_SIZE * TILE_SIZE];
        tileCache[i].filename[0] = '\0';
    }
    cacheFreeSlot = 0;
}

void freeCache() {
    if (tileCache) {
        for (int i = 0; i < MAX_CACHE_SIZE; i++) {
            delete[] tileCache[i].data;
        }
        delete[] tileCache;
        tileCache = nullptr;
        cacheFreeSlot = 0;
    }
}

void pngDraw(PNGDRAW* pDraw) {
    png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
    memcpy(&tileCache[cacheIndex].data[pDraw->y * TILE_SIZE], lineBuffer, TILE_SIZE * sizeof(uint16_t));
    if (pDraw->y + ypos >= SCREEN_HEIGHT) return;
    if (xpos >= SCREEN_WIDTH) return;
    int drawWidth = pDraw->iWidth;
    if (xpos + drawWidth > SCREEN_WIDTH) { drawWidth = SCREEN_WIDTH - xpos; }
    TFT.pushImage(xpos, ypos + pDraw->y, drawWidth, 1, lineBuffer);
}

void drawPngTile(const char* filename, int x, int y) {
    const TileCacheEntry* cachedTile = findInCache(filename);
    if (cachedTile) {
        TFT.pushImage(x, y, TILE_SIZE, TILE_SIZE, cachedTile->data);
        return;
    }

    xpos = x;
    ypos = y;
    cacheIndex = cacheFreeSlot++;
    if (cacheFreeSlot == MAX_CACHE_SIZE - 1) { cacheFreeSlot = 0; }
    int16_t rc = png.open(filename, pngOpen, pngClose, pngRead, pngSeek, pngDraw);
    if (rc == PNG_SUCCESS) {
        rc = png.decode(NULL, 0);
        png.close();
        strncpy(tileCache[cacheIndex].filename, filename, MAX_FILENAME_LENGTH);
    } else {
        print("! Failed to decode PNG: ", rc);
    }
}

#endif
