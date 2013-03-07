/*! \file global.h
Header for globals included with all other files.
*/ 
#pragma once
#include "includes.h"
#include "class_global.h"
#include "class_palette.h"
#include "errorMsg.h"
#include "class_tiles.h"
//functions
/*!
This function calculates pointer offset for truecolor tiles
for example cal_offset_truecolor(8,8,0,32);
would edit the red pixel (8,8) located on tile 32 
*/
uint32_t cal_offset_truecolor(uint16_t x,uint16_t y,uint16_t rgb,uint32_t tile);
//void draw_tile(uint16_t x_off,uint16_t y_off,uint32_t tile_draw,uint8_t zoom,uint8_t pal_row,bool hflip_tile,bool vflip_tile);
//void draw_truecolor(uint32_t tile,uint16_t x,uint16_t y,bool usehflip,bool usevflip,uint8_t zoom);
bool load_file_generic(const char * the_tile="Pick a file",bool save_file=false);
void truecolor_to_tile(uint8_t palette_row,uint32_t tile);
uint8_t find_near_color_from_row(uint8_t row,uint8_t r,uint8_t g,uint8_t b);
uint8_t find_near_color_from_row_rgb(uint8_t row,uint8_t r,uint8_t g,uint8_t b);
//map related functions
void resize_tile_map(uint16_t new_x,uint16_t new_y);
bool get_hflip(uint16_t x,uint16_t y);
bool get_vflip(uint16_t x,uint16_t y);
bool get_prio(uint16_t x,uint16_t y);
void set_tile_full(uint32_t tile,uint16_t x,uint16_t y,uint8_t palette_row,bool use_hflip,bool use_vflip,bool highorlow_prio);
void set_tile(uint32_t tile,uint16_t x,uint16_t y);
void set_prio(uint16_t x,uint16_t y,bool prio_set);
void set_hflip(uint16_t x,uint16_t y,bool hflip_set);
void set_vflip(uint16_t x,uint16_t y,bool vflip_set);
uint8_t get_palette_map(uint16_t x,uint16_t y);
uint32_t get_tile(uint16_t x,uint16_t y);
int32_t get_tileRow(uint16_t x,uint16_t y,uint8_t useRow);

void set_palette_type(uint8_t type);
uint32_t MakeRGBcolor(uint32_t pixel,
                          float saturation = 1.1f, float hue_tweak = 0.0f,
                          float contrast = 1.0f, float brightness = 1.0f,
                          float gamma = 2.2f);
//varibles and defines
#define sega_genesis 0
#define NES 1
extern uint8_t game_system;/*!< sets which game system is in use*/
extern Fl_Group * shadow_highlight_switch;
extern bool shadow_highlight;
extern uint16_t map_scroll_pos_x;
extern uint16_t map_scroll_pos_y;
//tabs group id
extern  intptr_t pal_id;
extern  intptr_t tile_edit_id;
extern  intptr_t tile_place_id;
extern bool show_grid_placer;
//uint32_t get_sel_tile();
/*#define get_sel_tile if (mode_editor == tile_place) sel_tile=current_tile_placer; \
	else if (mode_editor == tile_edit) sel_tile=current_tile; \
	else \
	{ \
		fl_alert("To prevent accidently altering tiles be in the tile editor or tile placer"); \
		return; \
	}*/

extern uint8_t tile_zoom_edit;
extern uint8_t palette[128];
extern uint8_t truecolor_temp[4];
//extern uint8_t * truecolor_tiles;
extern string the_file;//this is for tempory use only
extern uint8_t mode_editor;/*!< Importan varible is used to determin which "mode" the user is in for example palette editing or map editing*/
#define pal_edit 0
#define tile_edit 1
#define tile_place 2
#define default_map_off_x 304
extern uint16_t map_off_x,map_off_y;
#define default_map_off_y 232
#define tile_placer_tile_offset_x 120
#define default_tile_placer_tile_offset_y 208
#define tile_place_buttons_x_off 16
extern uint16_t tile_placer_tile_offset_y;
#define palette_bar_offset_x 16
#define default_palette_bar_offset_y 56
#define palette_preview_box_x 408
#define palette_preview_box_y 208
//extern uint8_t palette_bar_offset_y;
#define default_tile_edit_offset_x 344
//#define tile_edit_offset_x 472
#define default_tile_edit_offset_y 224
extern uint16_t tile_edit_offset_y;
extern uint16_t tile_edit_offset_x;
#define default_tile_edit_truecolor_off_x 8;
#define default_tile_edit_truecolor_off_y 224;
extern uint16_t tile_edit_truecolor_off_x,tile_edit_truecolor_off_y;
#define true_color_box_size 48
#define default_true_color_box_y 112
#define default_true_color_box_x 320
extern uint16_t true_color_box_x,true_color_box_y;
//uint16_t tile_edit_offset_y;
extern bool show_grid;
extern uint8_t palette_muliplier;
extern bool G_hflip;
extern bool G_vflip;
extern bool G_highlow_p;
extern uint8_t palette_adder;
extern uint8_t rgb_pal[192];
//extern uint8_t palette_entry;
//extern uint8_t rgb_temp[3];
//extern uint8_t tile_palette_row;//sets which palette row the tile displays
//extern uint8_t tile_palette_row_placer;//sets which palette row the tile placer displays
//tile editor
//extern uint8_t * tiles;
extern uint8_t * tile_map;
//extern uint32_t tiles_amount;//tiles there are minus one
//extern uint32_t current_tile;//current tile that we are editing minus one
//extern uint32_t current_tile_placer;
extern uint32_t file_size;
extern uint16_t map_size_x;
extern uint16_t map_size_y;
extern uint8_t * attr_nes;
extern uint8_t ditherAlg;
