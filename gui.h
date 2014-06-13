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
#define default_map_off_x 256
#define default_map_off_y 256
#define tile_placer_tile_offset_x 88
#define default_tile_placer_tile_offset_y 208
#define tile_place_buttons_x_off 8
#define palette_bar_offset_x 16
#define default_palette_bar_offset_y 56
#define palette_preview_box_x 408
#define palette_preview_box_y 208
#define default_tile_edit_offset_x 344
#define default_tile_edit_truecolor_off_x 8
#define default_tile_edit_truecolor_off_y 224
#define default_tile_edit_offset_y 224
#define true_color_box_size 48
#define default_true_color_box_y 188
#define default_true_color_box_x 732
#define DefaultChunckX 208
#define DefaultChunckY 80

extern uint32_t map_scroll_pos_x,map_scroll_pos_y;
extern uint32_t map_off_x,map_off_y;
extern uint16_t tile_edit_offset_x,tile_edit_offset_y;
extern uint16_t tile_placer_tile_offset_y;
extern uint16_t tile_edit_truecolor_off_x,tile_edit_truecolor_off_y;
extern uint16_t true_color_box_x,true_color_box_y;
extern unsigned ChunckOff[2];
extern unsigned scrollChunks[2];
extern uint_fast32_t editChunck_G[3];
#define pal_edit 0
#define tile_edit 1
#define tile_place 2
#define chunckEditor 3
#define settingsTab 4

int MenuPopup(const char * title,const char * text,unsigned num,...);
