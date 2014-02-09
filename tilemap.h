/*
 This file is part of Retro Graphics Toolkit

    Retro Graphics Toolkit is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or any later version.

    Retro Graphics Toolkit is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Retro Graphics Toolkit.  If not, see <http://www.gnu.org/licenses/>.
    Copyright Sega16 (or whatever you wish to call me (2012-2014)
*/
#pragma once
#include <inttypes.h>
void sub_tile_map(uint32_t oldTile,uint32_t newTile,bool hflip=false,bool vflip=false);
bool truecolor_to_image(uint8_t * the_image,int8_t useRow=-1,bool useAlpha=true);
void generate_optimal_palette(Fl_Widget*,void * row);
void truecolorimageToTiles(uint8_t * image,int8_t rowusage,bool useAlpha=true);
