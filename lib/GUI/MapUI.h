#ifndef MAP_COMPONENT_H
#define MAP_COMPONENT_H

#include <globals.h>
#include <secrets.h>
#include <vector>

void Map_init(Location center = {INIT_LON,INIT_LAT}, int initZoom = INIT_ZOOM, Location target = {0.0, 0.0}, float distance = 0.0, std::vector<Location> route = {});

#endif // MAP_COMPONENT_H
