#include "global.h"
#include "callbacks_palette.h"
#include "callback_tiles.h"
#include "tilemap.h"
#include "color_convert.h"
#include "errorMsg.h"
#include "dither.h"
//using namespace std;
void fill_tile(Fl_Widget* o, void*)
{
	//fills tile with currently selected color
	unsigned char color;
	color=tileMap_pal.box_sel;
	uint8_t * tile_ptr_temp;
	if (mode_editor == tile_place)
	{
		switch (game_system)
		{
			case sega_genesis:
				tile_ptr_temp = &tiles_main.tileDat[tiles_main.current_tile*32];
				color+=color<<4;
				for (unsigned int x=0;x<32;x++)
				{
					tile_ptr_temp[x]=color;
				}
			break;
			case NES:
				tile_ptr_temp = &tiles_main.tileDat[tiles_main.current_tile*16];
				//for the NES it is different
				unsigned char col_1;
				unsigned char col_2;
				col_1=color&1;
				col_2=(color>>1)&1;
				uint32_t x;
				/*for (x=0;x<16;x++)
				{
					tile_ptr_temp[x]=0;//we will be using the OR operation later on
				}*/
				memset(tile_ptr_temp,0,16);
				for (unsigned char y=0;y<8;y++)
				{
					for (x=0;x<8;x++)
					{
						tile_ptr_temp[y]|=col_1<<x;
						tile_ptr_temp[y+8]|=col_2<<x;
					}
				}
			break;
		}
	}
	else if (mode_editor == tile_edit)
	{
		for (unsigned int x=tiles_main.current_tile*256;x<(tiles_main.current_tile*256)+256;x+=4)
		{
			tiles_main.truetileDat[x]=truecolor_temp[0];//red
			tiles_main.truetileDat[x+1]=truecolor_temp[1];//green
			tiles_main.truetileDat[x+2]=truecolor_temp[2];//blue
			tiles_main.truetileDat[x+3]=truecolor_temp[3];//alpha
		}
		tiles_main.truecolor_to_tile(tileEdit_pal.theRow,tiles_main.current_tile);
	}
	//window->redraw();
	window->damage(FL_DAMAGE_USER1);
}

void update_truecolor(Fl_Widget* o, void* v)
{
	Fl_Slider* s = (Fl_Slider*)o;
	truecolor_temp[fl_intptr_t(v)] = s->value();
	window->redraw();
}

void blank_tile(Fl_Widget* o, void*)
{
	//this will fill the current tile with zeros
	tiles_main.blank_tile(tiles_main.current_tile);
	window->damage(FL_DAMAGE_USER1);
}

void callback_resize_map(Fl_Widget* o, void*)
{
	unsigned char w,h;
	w=window->map_w->value();
	h=window->map_h->value();
	resize_tile_map(w,h);
	window->redraw();
}

void update_offset_tile_edit(Fl_Widget* o, void*)
{
	tile_zoom_edit=window->tile_size->value();
	tile_edit_offset_x=16+(tile_zoom_edit*8);
	window->redraw();
}

void set_mode_tabs(Fl_Widget* o, void*)
{

	intptr_t val=(intptr_t)(Fl_Tabs*)window->the_tabs->value();
	
	if (val==pal_id)
		mode_editor=pal_edit;
	else if (val==tile_edit_id)
		mode_editor=tile_edit;
	else if (val==tile_place_id)
		mode_editor=tile_place;
}
void set_ditherAlg(Fl_Widget*, void* typeset)
{
	if ((uintptr_t)typeset == 0)
		window->ditherPower->show();
	else
		window->ditherPower->hide();//imagine the user trying to change the power and nothing happening not fun at all
	ditherAlg=(uintptr_t)typeset;
}

void set_tile_row(Fl_Widget*, void* row)
{
	unsigned char selrow=(uintptr_t)row;
	switch (mode_editor)
	{
		case tile_edit:
			tileEdit_pal.changeRow(selrow);
			tiles_main.truecolor_to_tile(selrow,tiles_main.current_tile);
		break;
		case tile_place:
			tileMap_pal.changeRow(selrow);
		break;
	}
	
	window->redraw();//trigger a redraw so that the new row is displayed
}
void update_box_size(Fl_Widget*, void* )
{
	window->redraw();
}
void set_tile_current(Fl_Widget* o, void* )
{
	Fl_Slider* s = (Fl_Slider*)o;
	tiles_main.current_tile=s->value();
	window->redraw();
}
void set_grid(Fl_Widget*,void*)
{
	//this function will only be trigger when the check button is pressed
	//so we just need to invert the bool using xor to avoid if statments
	show_grid=show_grid^true;
	window->redraw();//redraw to reflect the updated statues of the grid
}
void set_grid_placer(Fl_Widget*,void*)
{
	show_grid_placer=show_grid_placer^true;
	window->redraw();//redraw to reflect the updated statues of the grid
}
void set_prio_callback(Fl_Widget*,void*)
{
	G_highlow_p=G_highlow_p^true;
	//window->redraw();
}
void set_hflip(Fl_Widget*,void*)
{
	G_hflip=G_hflip^true;
	window->redraw();
}
void set_vflip(Fl_Widget*,void*)
{
	G_vflip=G_vflip^true;
	window->redraw();
}
void update_map_scroll_x(Fl_Widget*,void*)
{
	map_scroll_pos_x=window->map_x_scroll->value();
	//cout << "map scroll pos x = " << (short)map_scroll_pos_x << endl;//chars needed to casted to something else
	window->redraw();
}

void update_map_scroll_y(Fl_Widget*,void*)
{
	map_scroll_pos_y=window->map_y_scroll->value();
	//cout << "map scroll pos x = " << (short)map_scroll_pos_x << endl;//chars needed to casted to something else
	window->redraw();
}

void update_map_size(Fl_Widget*,void*)
{
	unsigned short old_scroll=window->map_x_scroll->value();
	unsigned char tile_size_placer=window->place_tile_size->value();
	int map_scroll=((tile_size_placer*8)*map_size_x)-map_off_x;//size of all offscreen tiles in pixels
	//map_scroll-=(tile_size_placer*8);
	if (map_scroll < 0)
	{
		map_scroll=0;
	}
	map_scroll/=tile_size_placer*8;//now size of all tiles
	//cout << "tiles off screen: " << map_scroll << endl;
	if (old_scroll > map_scroll)
	{
		old_scroll=map_scroll;
		map_scroll_pos_x=map_scroll;
	}
	window->map_x_scroll->value(old_scroll,(map_scroll/2),0,map_scroll+(map_scroll/2));//the reason for adding map_scroll/2 to map_scroll is because without it the user will not be able to scroll the tilemap all the way
	old_scroll=window->map_y_scroll->value();
	tile_size_placer=window->place_tile_size->value();
	map_scroll=((tile_size_placer*8)*map_size_y)-map_off_y;//size of all offscreen tiles in pixels
	//map_scroll-=(tile_size_placer*8);
	if (map_scroll < 0)
	{
		map_scroll=0;
	}
	map_scroll/=tile_size_placer*8;//now size of all tiles
	//cout << "tiles off screen: " << map_scroll << endl;
	if (old_scroll > map_scroll)
	{
		old_scroll=map_scroll;
		map_scroll_pos_y=map_scroll;
	}
	window->map_y_scroll->value(old_scroll,(map_scroll/2),0,map_scroll+(map_scroll/2));
	window->redraw();
}


void save_tiles_truecolor(Fl_Widget*,void*)
{
	if (load_file_generic("Save truecolor tiles",true) == true)
	{
		FILE * myfile;
		myfile = fopen(the_file.c_str(),"wb");
		if (myfile!=0)
		{
			fwrite(tiles_main.truetileDat,1,(tiles_main.tiles_amount+1)*256,myfile);
			puts("Great Sucess File Saved!");
			fclose(myfile);
		}
		else
		{
			fl_alert("Error: can not save file %s",the_file.c_str());
		}
		
	}
}

void save_tiles(Fl_Widget*,void*)
{
	if (load_file_generic("Pick a location to save tiles",true) == true)
	{
		FILE * myfile;
		//myfile.open(the_file,ios::binary | ios::trunc);
		myfile = fopen(the_file.c_str(),"wb");
		if (myfile!=0)
		{
			fwrite(tiles_main.tileDat,1,(tiles_main.tiles_amount+1)*tiles_main.tileSize,myfile);
			
			puts("File saved");
		}
		else
		{
			//cout << "myfile.is_open() returned false that means there was an error in creating the file" << endl;
			fl_alert("Error: can not save file %s",the_file.c_str());
			//cout << "File that could not be saved: " << the_file << endl;
		}
		fclose(myfile);
	}
}

void save_map(Fl_Widget*,void*)
{
	uint16_t x,y;
	FILE * myfile;
	if (load_file_generic("Save tilemap to",true) == true)
	{
		myfile = fopen(the_file.c_str(),"wb");
		if (myfile!=0)
		{
			switch (game_system)
			{
				case sega_genesis:
				{
					uint16_t * TheMap;
					TheMap = (uint16_t *)malloc((map_size_x*map_size_y)*2);

					for (y=0;y<map_size_y;y++)
					{
						for (x=0;x<map_size_x;x++)
						{
							uint32_t tile=get_tile(x,y);
							if (tile > 2047)
							{
								printf("Warning tile value %d exceeded 2047 at x: %d y: %d\n",tile,x,y);
								tile=2047;
							}
							tile=htobe16(tile);//needs to be big endian 
							*TheMap=(uint16_t)tile_map[((y*map_size_x)+x)*4]<<8;//get attributes
							*TheMap++|=(uint16_t)tile;//add tile
						}
					}
					//TheMap--;
					TheMap-=map_size_x*map_size_y;//return to begining so it can be freeded and the file saved
					fwrite(TheMap,2,map_size_x*map_size_y,myfile);
					free(TheMap);
				}//brackets used to prevent TheMap conflict
				break;
				case NES:
				{
					uint8_t * TheMap;
					TheMap = (uint8_t *)malloc(map_size_x*map_size_y);
					for (y=0;y<map_size_y;y++)
					{
						for (x=0;x<map_size_x;x++)
						{
							uint32_t tile=get_tile(x,y);
							if (tile > 255)
							{
								printf("Warning tile value %d exceeded 255 at x: %d y: %d\n",tile,x,y);
								tile=255;
							}
							*TheMap++=tile;
							
						}
					}
					//TheMap--;
					TheMap-=map_size_x*map_size_y;//return to begining so it can be freeded and the file sized
					fwrite(TheMap,1,map_size_x*map_size_y,myfile);
					free(TheMap);
				}
				break;
			}
			fclose(myfile);
			puts("File Saved");
		}
		else
		{
			fl_alert("Error: can not save file %s",the_file.c_str());
		}
		
	}
	if (game_system == NES)
	{
		if (load_file_generic("Save attributes to",true) == true)
		{
			myfile = fopen(the_file.c_str(),"wb");
			if (myfile!=0)
			{
				uint8_t * AttrMap = (uint8_t *)malloc((map_size_x/4)*(map_size_y/4));
				uint8_t * freeAttrMap=AttrMap;
				for (y=0;y<map_size_y;y+=4)
				{
					for (x=0;x<map_size_x;x+=4)
					{
						*AttrMap++ = get_palette_map(x,y) | (get_palette_map(x+2,y)<<4) | (get_palette_map(x,y+2) << 4) | (get_palette_map(x+2,y+2) << 6);
						printf("x: %d y: %d\n",x,y);
					}
				}
				//AttrMap-=(map_size_x/4)*(map_size_y/4);
				printf("%d %d\n",AttrMap,freeAttrMap);
				fwrite(freeAttrMap,1,(map_size_x/4)*(map_size_y/4),myfile);
				
				free(freeAttrMap);
				fclose(myfile);
				puts("File Saved");
			}
			else
			{
				fl_alert("Error: can not save file %s",the_file.c_str());
			}
			
		}
	}
}

void load_tiles(Fl_Widget*,void* split)
{
	//if append==1 then we will append data but if not it will erase over current tiles
	//format row,append
	unsigned char append=(uintptr_t)split&0xFF;
	unsigned char row=(uintptr_t)split>>8;

	if (load_file_generic() == true)
	{
		FILE * myfile;
		fopen(the_file.c_str(),"rb");
		if (myfile!=0)
		{
			fseek(myfile, 0L, SEEK_END);
			file_size = ftell(myfile);//file.tellg();
			unsigned char truecolor_multiplier;
			truecolor_multiplier=256/tiles_main.tileSize;
			if ((file_size/tiles_main.tileSize)*tiles_main.tileSize != file_size)
			{
				fl_alert("Error: This is not a valid tile file each tile is %d bytes and this file is not a multiple of %d so it is not a valid tile file",tiles_main.tileSize,tiles_main.tileSize);
				fclose(myfile);
				return;//return so that the file does not get loaded
			}
			rewind(myfile);
			unsigned int offset_tiles;
			unsigned int offset_tiles_bytes;
			
			if (append == 1)
			{
				offset_tiles=tiles_main.tiles_amount+1;
				offset_tiles_bytes=offset_tiles*tiles_main.tileSize;
				tiles_main.tileDat = (unsigned char *)realloc(tiles_main.tileDat,file_size+((tiles_main.tiles_amount+1)*tiles_main.tileSize));
				if (tiles_main.tileDat == 0)
				{
					fclose(myfile);
					show_realloc_error(file_size)
				}
			}
			else
			{
				free(tiles_main.tileDat);
				tiles_main.tileDat = (unsigned char *)malloc(file_size);
				if (tiles_main.tileDat == 0)
				{
					fclose(myfile);
					show_malloc_error(file_size)
				}
				offset_tiles=0;
				offset_tiles_bytes=0;
			}
			fread(tiles_main.tileDat+offset_tiles_bytes,1,file_size,myfile);
			fclose(myfile);
			tiles_main.truetileDat = (unsigned char *)realloc(tiles_main.truetileDat,(file_size*truecolor_multiplier)+(offset_tiles_bytes*truecolor_multiplier));
			if (tiles_main.truetileDat == 0)
			{
				//file.close();
				show_malloc_error(file_size*truecolor_multiplier)
			}
			switch (game_system)
			{
				case sega_genesis:
					for (unsigned int c=offset_tiles;c<(file_size/tiles_main.tileSize)+offset_tiles;c++)
					{

						for (unsigned char y=0;y<8;y++)
						{
							for (unsigned char x=0;x<4;x++)
							{
								//even,odd
								unsigned char temp=tiles_main.tileDat[(c*32)+(y*4)+x];
								unsigned char temp_1,temp_2;
								temp_1=temp>>4;//first pixel
								temp_2=temp&15;//second pixel
								tiles_main.truetileDat[cal_offset_truecolor(x*2,y,0,c)]=rgb_pal[(row*48)+(temp_1*3)];
								tiles_main.truetileDat[cal_offset_truecolor(x*2,y,1,c)]=rgb_pal[(row*48)+(temp_1*3)+1];
								tiles_main.truetileDat[cal_offset_truecolor(x*2,y,2,c)]=rgb_pal[(row*48)+(temp_1*3)+2];

								tiles_main.truetileDat[cal_offset_truecolor((x*2)+1,y,0,c)]=rgb_pal[(row*48)+(temp_2*3)];
								tiles_main.truetileDat[cal_offset_truecolor((x*2)+1,y,1,c)]=rgb_pal[(row*48)+(temp_2*3)+1];
								tiles_main.truetileDat[cal_offset_truecolor((x*2)+1,y,2,c)]=rgb_pal[(row*48)+(temp_2*3)+2];
							}
						}
					}
				break;
				case NES:
					for (unsigned int c=offset_tiles;c<(file_size/tiles_main.tileSize)+offset_tiles;c++)
					{
						for (unsigned char y=0;y<8;y++)
						{
							for (unsigned char x=0;x<8;x++)

							{
								unsigned char temp;
								temp=(tiles_main.tileDat[(c*16)+y]>>x)&1;
								temp|=((tiles_main.tileDat[(c*16)+y+8]>>x)&1)<<1;
								tiles_main.truetileDat[cal_offset_truecolor(x,y,0,c)]=rgb_pal[(row*12)+(temp*3)];
								tiles_main.truetileDat[cal_offset_truecolor(x,y,1,c)]=rgb_pal[(row*12)+(temp*3)+1];
								tiles_main.truetileDat[cal_offset_truecolor(x,y,2,c)]=rgb_pal[(row*12)+(temp*3)+2];
							}
						}
					}
			}
			tiles_main.tiles_amount=(file_size/tiles_main.tileSize)-1;
			tiles_main.tiles_amount+=+offset_tiles-1;
			window->tile_select->maximum(tiles_main.tiles_amount);
			window->tile_select->value(0);

			window->tile_select_2->maximum(tiles_main.tiles_amount);
			window->tile_select_2->value(0);
			window->redraw();
		}
		else
		{
			//cout << "file.is_open() returnd false meaning that the file could not be read" << endl;
			fl_alert("File Cannot be loaded");
		}
	}
}
void update_all_tiles(Fl_Widget*,void*)
{
	unsigned char sel_pal;
	if (mode_editor == tile_place)
	{
		sel_pal=tileMap_pal.theRow;
	}
	else
	{
		sel_pal=tileEdit_pal.theRow;
	}
	if (tiles_main.tiles_amount > 63)
		printf("\n");
	for (unsigned int x=0;x<tiles_main.tiles_amount+1;x++)
	{
		tiles_main.truecolor_to_tile(sel_pal,x);
		if ((x % 64) == 0)
			printf("Progress: %f\r",((float)x/(float)tiles_main.tiles_amount)*100.0);
	}
	window->redraw();
}

bool verify_str_number_only(char * str)
{
/*
Fltk provides an input text box that makes it easy for the user to type text however as a side effect they can accidently enter non number characters that may be handled weird by atoi()
this function address that issue by error checking the string and it also gives the user feedback so they are aware that the input box takes only numbers
this function returns true when the string contains only numbers 0-9 and false when there is other stuff
it will also allow the use of the - symbol as negative
*/
	while(*str++)
	{
		if (*str != 0 && *str != '-')
		{
			if (*str < '0')
			{
				fl_alert("Please enter only numbers in decimal format\nCharacter entered %c",*str);
				return false;
			}
			if (*str > '9')
			{
				fl_alert("Please enter only numbers in decimal format\nCharacter entered %c",*str);
				return false;
			}
		}
	}
	return true;
}

void load_truecolor_tiles(Fl_Widget*,void*)
{
	//start by loading the file
	if (load_file_generic() == true)
	{
		ifstream file (the_file.c_str(), ios::in|ios::binary|ios::ate);
		file_size = file.tellg();
		if ((file_size/256)*256 != file_size)
		{
			fl_alert("Error: this file is not a multiple of 256 it is not a valid truecolor tiles. The file size is: %d",file_size);
			file.close();
			return;
		}
		free(tiles_main.truetileDat);
		free(tiles_main.tileDat);
		tiles_main.truetileDat = (uint8_t *)malloc(file_size);
		if (tiles_main.truetileDat == 0)
		{
			show_malloc_error(file_size)
		}
		switch (game_system)
		{
			case sega_genesis:
				tiles_main.tileDat = (uint8_t *)malloc(file_size/6);
			break;
			case NES:
				tiles_main.tileDat = (uint8_t *)malloc(file_size/12);
			break;
		}
		if (tiles_main.tileDat == 0)
		{
			show_malloc_error(file_size/6)
		}

		file.seekg (0, ios::beg);//return to the beginning of the file
		file.read ((char *)tiles_main.truetileDat, file_size);
		file.close();
		tiles_main.tiles_amount=file_size/256;
		tiles_main.tiles_amount--;
		window->tile_select->maximum(tiles_main.tiles_amount);
		window->tile_select_2->maximum(tiles_main.tiles_amount);
		window->redraw();
	}
}

void fill_tile_map_with_tile(Fl_Widget*,void*)
{
	if (mode_editor != tile_place)
	{
		fl_alert("To prevent aciddental modifaction to the tile map be in plane editing mode");
		return;
	}
	for (uint16_t y=0;y<map_size_y;y++)
	{
		for (uint16_t x=0;x<map_size_x;x++)
		{
			set_tile_full(tiles_main.current_tile,x,y,tileMap_pal.theRow,G_hflip,G_vflip,G_highlow_p);
		}
	}
	window->redraw();
}

void zero_error_tile_map(int x)
{//this is a long string I do not want it stored more than once
	fl_alert("Please enter value greater than zero you on the other hand entered %d",x);
}

void load_tile_map(Fl_Widget*,void*)
{
	//start by loading the file
	if (load_file_generic("Load tile map data") == true)
	{
		//get width and height
		string tilemap_file=the_file;
		int w,h;
		char str[16];
		char * str_ptr;
		str_ptr=&str[0];
		str_ptr=(char *)fl_input("Enter Width");
		if (str_ptr == 0)
			return;
		if (verify_str_number_only(str_ptr) == false)
			return;
		w=atoi(str_ptr);
		if (w <= 0)
		{
			zero_error_tile_map(w);
			return;
		}
		if (game_system == NES && (w & 1))
		{
			fl_alert("Error unlike in sega genesis mode NES mode needs the width and height to be a multiple to 2");
			return;
		}
		str_ptr=&str[0];
		str_ptr=(char *)fl_input("Enter Height");
		if (str_ptr == 0)
			return;
		if (verify_str_number_only(str_ptr) == false)
			return;
		h=atoi(str_ptr);
		if (h <= 0)
		{
			zero_error_tile_map(h);
			return;
		}
		if (game_system == NES && (h & 1))
		{
			fl_alert("Error unlike in sega genesis mode NES mode needs the width and height the be a multiple to 2");
			return;
		}

		//we can now load the map

		str_ptr=&str[0];
		str_ptr=(char *)fl_input("Enter offset\nIf the tile map is generated by this program enter zero\nIf not then the first tile may not be zero\nLets say that the first tile is 200 you would enter -200 that makes the first tile zero\nIf you want the first tile to have an offset enter a positive number for example if the tilemap has the first tile set to zero and you enter 5 the first tile will be tile 5");
		if (str_ptr == 0)
		{
			//cout << "canceled by user" << endl;
			return;
		}
		if (verify_str_number_only(str_ptr) == false)
		{
			return;
		}
		int offset=atoi(str_ptr);
		ifstream file (tilemap_file.c_str(), ios::in|ios::binary|ios::ate);
		file_size = file.tellg();
		uint32_t size_temp;
		switch (game_system)
		{
			case sega_genesis:
				size_temp=(w*h)*2;
			break;
			case NES:
				size_temp=w*h;
			break;
		}
		if (size_temp > file_size)
		{
			fl_alert("Error: The Tile map that you are attempting to load is smaller than a tile map that has the width and height that you specified");
			file.close();//even though there was an error the file was open so it still needs to be closed
			return;//return so that the file does not get loaded
		}
		file.seekg (0, ios::beg);//return to the beginning of the file
		//start converting to tile
		//free(tile_map);
		tile_map = (uint8_t *) realloc(tile_map,(w*h)*4);
		uint8_t * tempMap = (uint8_t *) malloc(size_temp);
		if (tile_map == 0)
		{
			show_malloc_error(size_temp)
		}
		file.read ((char *)tempMap, size_temp);
		file.close();
		window->map_w->value(w);
		window->map_h->value(h);
		map_size_x=w;
		map_size_y=h;
		uint16_t x,y;
		switch (game_system)
		{
			case sega_genesis:
				for (y=0;y<h;y++)
				{
					for (x=0;x<w;x++)
					{
						uint16_t temp=*tempMap++;
						//set attributes
						tile_map[((y*map_size_x)+x)*4]=(uint8_t)temp&0xF8;
						temp&=7;
						temp<<=8;
						temp|=(uint16_t)*tempMap++;
						if (temp+offset > 0)
							set_tile((int32_t)temp+offset,x,y);
						else
							set_tile(0,x,y);
						printf("Tile value %d at %d %d\n",temp+offset,x,y);
						
					}
				}
			break;
			case NES:
				for (y=0;y<h;y++)
				{
					for (x=0;x<w;x++)
					{
						uint8_t temp=*tempMap++;
						if (temp+offset > 0)
							set_tile((int32_t)temp+offset,x,y);
						else
							set_tile(0,x,y);
						printf("Tile value %d at %d %d\n",temp+offset,x,y);
					}
				}
				//now load attributes
				if (load_file_generic("Load Attribtues") == true)
				{
					puts("done");
				}
			break;
		}
		tempMap-=size_temp;
		free(tempMap);
		window->redraw();
	}
}
void shadow_highligh_findout(Fl_Widget*,void*)
{
	if (game_system != sega_genesis)
	{
		fl_alert("Y u no use genesis\n");
		return;
	}
	//this function will see if 3 or less pixels are above 126+9=135 and if so set prioity to low or set priority to high is bright tile
	uint16_t x,y;
	uint32_t xx;
	for (y=0;y<map_size_y;y++)
	{
		for (x=0;x<map_size_x;x++)
		{
			uint32_t cur_tile=get_tile(x,y);
			uint8_t over=0;
			for (xx=cur_tile*256;xx<cur_tile*256+256;xx+=4)
			{
				if ((tiles_main.truetileDat[xx] > 135) || (tiles_main.truetileDat[xx+1] > 135) || (tiles_main.truetileDat[xx+2] > 135))
					over++;
			}
			if (over > 3)
				set_prio(x,y,true);
			else
				set_prio(x,y,false);
		}
	}
	window->redraw();
}

void dither_tilemap_as_image(Fl_Widget*,void*)
{
	//normally this program dithers all tiles individully this is not always desirable
	//to fix this I created this function It convertes the tilemap to image and dithers all tiles
	//so first create ram for image
	uint8_t * image;
	unsigned int w,h;
	uint8_t useHiL=palette_muliplier;
	uint8_t type_temp;
	bool tempSet;
	if (palette_adder==0)
	{
		type_temp=1;
	}
	else
	{
		type_temp=2;
	}
	w=map_size_x*8;
	h=map_size_y*8;
	image = (uint8_t *)malloc(w*h*4);
	if (image==0)
		show_malloc_error(w*h*4)
	unsigned int truecolor_tile_ptr=0;
	uint32_t x_tile=0,y_tile=0;
	unsigned char truecolor_tile[256];
	for (uint8_t rowz=0;rowz<4;rowz++)
	{
	printf("Row %d\n",rowz);
	puts("Starting");
	printf("Stage 1 %%: 0\n");
	truecolor_to_image(image,rowz);
	printf("Stage 1 %%: %f\n",(1.0f/3.0f)*100.0f);
	ditherImage(image,w,h,true);
	printf("Stage 2 %%: %f\n",(2.0f/3.0f)*100.0f);
	//convert back to tiles
	x_tile=0;
	y_tile=0;
	puts("Stage 3 starting");
	for (uint64_t a=0;a<(h*w*4)-w*4;a+=w*4*8)//a tiles y
	{
		for (uint32_t b=0;b<w*4;b+=32)//b tiles x
		{	
			uint8_t temp;
			int32_t current_tile=get_tileRow(x_tile,y_tile,rowz);
			if (current_tile == -1)
				goto dont_convert_tile;
			truecolor_tile_ptr=0;
			for (unsigned int y=0;y<w*4*8;y+=w*4)//pixels y
			{
				memcpy(&truecolor_tile[truecolor_tile_ptr],&image[a+b+y],32);
				truecolor_tile_ptr+=32;
			}
			//convert back to tile
			switch (game_system)
			{
				case sega_genesis:
					current_tile*=32;
					if (useHiL == 9)
					{
						tempSet=get_prio(x_tile,y_tile)^true;
						set_palette_type(tempSet);
					}
					for (unsigned char y=0;y<8;y++)
					{
						for (unsigned char x=0;x<32;x+=4)
						{
							//even,odd
							if (x & 4)
							{
								//odd
								if (truecolor_tile[(y*32)+x+3] != 0)
								{
									temp=find_near_color_from_row(get_palette_map(x_tile,y_tile),truecolor_tile[(y*32)+x],truecolor_tile[(y*32)+x+1],truecolor_tile[(y*32)+x+2]);
									tiles_main.tileDat[current_tile+(x/8)+(y*4)]|=temp;
								}
							}
							else
							{
								//even
								if (truecolor_tile[(y*32)+x+3] != 0)
								{
									temp=find_near_color_from_row(get_palette_map(x_tile,y_tile),truecolor_tile[(y*32)+x],truecolor_tile[(y*32)+x+1],truecolor_tile[(y*32)+x+2]);
									tiles_main.tileDat[current_tile+(x/8)+(y*4)]=temp<<4;
								}
								else
									tiles_main.tileDat[current_tile+(x/8)+(y*4)]=0;
							}
						}

					}
				break;
				case NES:
					current_tile*=16;
					for (unsigned char x=0;x<16;x++)
					{
						tiles_main.tileDat[current_tile+x]=0;
					}
					for (unsigned char y=0;y<8;y++)
					{
						for (unsigned char x=0;x<8;x++)
						{
							if (truecolor_tile[(y*32)+(x*4)+3] != 0)
							{
								temp=find_near_color_from_row(get_palette_map(x_tile,y_tile),truecolor_tile[(y*32)+(x*4)],truecolor_tile[(y*32)+(x*4)+1],truecolor_tile[(y*32)+(x*4)+2]);
								tiles_main.tileDat[y+current_tile]|=(temp&1)<<(7-x);
								tiles_main.tileDat[y+8+current_tile]|=((temp>>1)&1)<<(7-x);
							}
						}
					}
				break;
			}
dont_convert_tile:
		x_tile++;	
		}
	x_tile=0;
	y_tile++;
	//update progress map_size_y
	printf("Stage 3 %%: %f\r",(((float)y_tile/(float)map_size_y/3.0f)+(2.0f/3.0f))*100.0f);
	}
	puts("");
	puts("Done with image");
	}
	free(image);
	if (game_system == sega_genesis && useHiL == 9)
		set_palette_type(type_temp);
	window->redraw();
}

void load_image_to_tilemap(Fl_Widget*,void*)
{
	Fl_Shared_Image * loaded_image;
	if (load_file_generic("Load image") == true)
	{
		//cout << "about to load image" << endl;
		loaded_image=Fl_Shared_Image::get(the_file.c_str());
		if (loaded_image == 0)
		{
			fl_alert("Error loading image");
			return;
		}
		unsigned int w,h;
		w=loaded_image->w();
		h=loaded_image->h();
		cout << "image width: " << w << "image height: " << h << endl;
		if ((w/8)*8 != w && (h/8)*8 != h)
		{
			fl_alert("Error both width and height are not a multiple of 8");
			return;
		}
		if ((w/8)*8 != w)
		{
			fl_alert("Error width is not a multiple of 8");
			return;
		}
		if ((h/8)*8 != h)
		{
			fl_alert("Error height is not a multiple of 8");
			return;
		}
		//start by copying the data
		unsigned char * img_ptr=(unsigned char *)loaded_image->data()[0];
		//printf("First Pixel Red: %d Green: %d Blue: %d\n",img_ptr[0],img_ptr[1],img_ptr[2]);
		//now we can convert to tiles
		if (loaded_image->d() != 3 && loaded_image->d() != 4)
		{
			fl_alert("Please use color depth of 3 or 4\nYou Used %d",loaded_image->d());
			loaded_image->release();
			return;
		}
		unsigned long truecolor_tile_ptr=0;
		tiles_main.truetileDat = (uint8_t *)realloc(tiles_main.truetileDat,((w/8)*(h/8))*256);
		tiles_main.tileDat = (uint8_t *)realloc(tiles_main.tileDat,((w/8)*(h/8))*tiles_main.tileSize);
		tiles_main.tiles_amount=(w/8)*(h/8);
		tiles_main.tiles_amount--;
		window->tile_select->maximum(tiles_main.tiles_amount);
		window->tile_select_2->maximum(tiles_main.tiles_amount);
		//uint8_t sizeTemp,sizeTemp2;
		switch (loaded_image->d())
		{
			case 3:
				for (unsigned long a=0;a<(h*w*3)-w*3;a+=w*3*8)//a tiles y
				{
					for (unsigned int b=0;b<w*3;b+=24)//b tiles x
					{
						for (unsigned int y=0;y<w*3*8;y+=w*3)//pixels y
						{
							uint8_t xx=0;
							for (unsigned char x=0;x<32;x+=4)//pixels x
							{
								tiles_main.truetileDat[truecolor_tile_ptr+x]=img_ptr[a+b+y+xx];
								tiles_main.truetileDat[truecolor_tile_ptr+x+1]=img_ptr[a+b+y+xx+1];
								tiles_main.truetileDat[truecolor_tile_ptr+x+2]=img_ptr[a+b+y+xx+2];
								tiles_main.truetileDat[truecolor_tile_ptr+x+3]=255;//solid
								xx+=3;
							}
							truecolor_tile_ptr+=32;
						}
					}
				}
			break;
			case 4:
				for (unsigned long a=0;a<(h*w*4)-w*4;a+=w*4*8)//a tiles y
				{
					for (unsigned int b=0;b<w*4;b+=32)//b tiles x
					{
						for (unsigned int y=0;y<w*4*8;y+=w*4)//pixels y
						{
							for (unsigned char x=0;x<32;x+=4)//pixels x
							{
								tiles_main.truetileDat[truecolor_tile_ptr+x]=img_ptr[a+b+y+x];
								tiles_main.truetileDat[truecolor_tile_ptr+x+1]=img_ptr[a+b+y+x+1];
								tiles_main.truetileDat[truecolor_tile_ptr+x+2]=img_ptr[a+b+y+x+2];
								tiles_main.truetileDat[truecolor_tile_ptr+x+3]=img_ptr[a+b+y+x+3];
							}
							truecolor_tile_ptr+=32;
						}
					}
				}
			break;
			default:
				fl_alert("HUGE ERROR ENDING FUNCTION\n");
				loaded_image->release();
				return;
			break;
		}
		loaded_image->release();
		resize_tile_map(w/8,h/8);
		window->map_w->value(w/8);
		window->map_h->value(h/8);
		uint32_t tilecounter=0;
		for (unsigned short y=0;y<h/8;y++)
		{
			for (unsigned short x=0;x<w/8;x++)
			{
				set_tile(tilecounter,x,y);
				tilecounter++;
			}
		}
		window->redraw();
		
	}
}
void set_palette_type_callback(Fl_Widget*,void* type)
{
	set_palette_type((uintptr_t)type);
	window->redraw();
}

void create_shadow_highlight_buttons()
{
	{ shadow_highlight_switch = new Fl_Group(0, 0, 800, 480);
		{ Fl_Round_Button* o = new Fl_Round_Button(128, 288, 64, 32, "Normal");
		o->type(102);
		o->tooltip("This is the default sega genesis color.When shadow/hilight mode is disabled all tiles will look like this however when enabling shadow higligh mode and a tile is set to high prioraty you will the tile will use these set of colors");
		o->down_box(FL_ROUND_DOWN_BOX);
		o->callback((Fl_Callback*) set_palette_type_callback,(void *)0);
		} // Fl_Round_Button* o
		{ Fl_Round_Button* o = new Fl_Round_Button(192, 288, 64, 32, "Shadow");
		o->tooltip("This mode uses the color sets that the vdp uses when shadow highlight mode is enabled by setting bit 3 (the LSB being bit 0) to 1 in the vdp register 0C also for the tile to be shadowed the tile's prioraty must be set at 0 or low prioraty");
		o->type(102);
        o->down_box(FL_ROUND_DOWN_BOX);
       o->callback((Fl_Callback*) set_palette_type_callback,(void *)1);
      } // Fl_Round_Button* o
      { Fl_Round_Button* o = new Fl_Round_Button(256, 288, 64, 32, "Highlight");
	  o->tooltip("This mode uses the color sets that a highlighted sprite or tile uses to make a tile highlighted use a mask sprite");
        o->type(102);
        o->down_box(FL_ROUND_DOWN_BOX);
        o->callback((Fl_Callback*) set_palette_type_callback,(void *)2);
      } // Fl_Round_Button* o
      shadow_highlight_switch->end();
		} // Fl_Group* o
}


void remove_duplicate_tiles(Fl_Widget*,void*)
{
	tiles_main.remove_duplicate_tiles();
}

void remove_duplicate_truecolor(Fl_Widget*,void*)
{
	//sub_tile_map
	uint32_t tile_remove_c=0;
	int32_t cur_tile,curT;
	puts("Pass 1");
	for (cur_tile=0;cur_tile<=tiles_main.tiles_amount;cur_tile++)
	{
		for (curT=tiles_main.tiles_amount;curT>=0;curT--)
		{
			if (cur_tile == curT)//dont compare it's self
				continue;
			if (tiles_main.cmp_trueC(cur_tile,(uint32_t *)&tiles_main.truetileDat[curT*256]))
			{
				sub_tile_map(curT,cur_tile,false,false);
				tiles_main.remove_tile_at(curT);
				tile_remove_c++;
			}
		}
		//if ((cur_tile % 4096) == 0)
			printf("On tile %d Removed %d\r",cur_tile,tile_remove_c);
		
	}
	printf("Removed %d tiles\n",tile_remove_c);
	tile_remove_c=0;
	puts("Pass 2 h-flip");
	uint8_t trueColTemp[256];
	for (cur_tile=0;cur_tile<=tiles_main.tiles_amount;cur_tile++)
	{
		for (curT=tiles_main.tiles_amount;curT>=0;curT--)
		{
			if (cur_tile == curT)//dont compare it's self
				continue;
			tiles_main.hflip_truecolor(curT,(uint32_t *)&trueColTemp[0]);
			if (tiles_main.cmp_trueC(cur_tile,(uint32_t *)&trueColTemp[0]))
			{
				sub_tile_map(curT,cur_tile,true,false);
				tiles_main.remove_tile_at(curT);
				tile_remove_c++;
			}
		}
		//if ((cur_tile % 4096) == 0)
			printf("On tile %d Removed %d\r",cur_tile,tile_remove_c);
		
	}
	printf("Removed %d tiles\n",tile_remove_c);
	tile_remove_c=0;
	puts("Pass 3 v-flip");
	for (cur_tile=0;cur_tile<=tiles_main.tiles_amount;cur_tile++)
	{
		for (curT=tiles_main.tiles_amount;curT>=0;curT--)
		{
			if (cur_tile == curT)//dont compare it's self
				continue;
			tiles_main.vflip_truecolor(curT,&trueColTemp[0]);
			if (tiles_main.cmp_trueC(cur_tile,(uint32_t *)&trueColTemp[0]))
			{
				sub_tile_map(curT,cur_tile,false,true);
				tiles_main.remove_tile_at(curT);
				tile_remove_c++;
			}
		}
		//if ((cur_tile % 4096) == 0)
			printf("On tile %d Removed %d\r",cur_tile,tile_remove_c);
		
	}
	printf("Removed %d tiles\n",tile_remove_c);
	tile_remove_c=0;
	puts("Pass 4 vh-flip");
	for (cur_tile=0;cur_tile<=tiles_main.tiles_amount;cur_tile++)
	{
		for (curT=tiles_main.tiles_amount;curT>=0;curT--)
		{
			if (cur_tile == curT)//dont compare it's self
				continue;
			tiles_main.hflip_truecolor(curT,(uint32_t *)&trueColTemp[0]);
			tiles_main.vflip_truecolor_ptr(trueColTemp,trueColTemp);
			if (tiles_main.cmp_trueC(cur_tile,(uint32_t *)&trueColTemp[0]))
			{
				sub_tile_map(curT,cur_tile,true,true);
				tiles_main.remove_tile_at(curT);
				tile_remove_c++;
			}
		}
		//if ((cur_tile % 4096) == 0)
			printf("On tile %d Removed %d\r",cur_tile,tile_remove_c);
		
	}
	printf("Removed %d tiles\n",tile_remove_c);
	tile_remove_c=0;
	window->redraw();
}


void rgb_pal_to_entry(Fl_Widget*,void*)
{
	//this function will convert a rgb value to the nearst palette entry
	if (mode_editor != tile_edit)
	{
		fl_alert("Be in Tile editor to use this");
		return;
	}
}



void set_game_system(Fl_Widget*,void* selection)
{
	if ((uintptr_t)selection == game_system)
	{
		fl_alert("You are already in that mode");
		return;
	}
//	unsigned int rgb_out;
	switch((uintptr_t)selection)
	{
		case sega_genesis:
			//fl_alert("Sega genesis Mode");
			game_system=sega_genesis;
			tiles_main.tileSize=32;
			//create_shadow_highlight_buttons();
			shadow_highlight_switch->show();

			{//for varibles to be declared inside of switch statment I must put brackes around so the compiler knows when to free them
				uint8_t pal_temp[128];
				uint8_t c;
				for (c=0;c<64;c++)
				{
					unsigned short temp=to_sega_genesis_color(c);
					pal_temp[c*2]=temp>>8;
					pal_temp[(c*2)+1]=temp&255;
				}
				for (c=0;c<64;c++)
				{
					palette[c*2]=pal_temp[c*2];
					palette[(c*2)+1]=pal_temp[(c*2)+1];
				}
			}
			palEdit.changeSystem();
			tileEdit_pal.changeSystem();
			tileMap_pal.changeSystem();
			tiles_main.tileDat = (unsigned char *)realloc(tiles_main.tileDat,(tiles_main.tiles_amount+1)*32);
		break;
		case NES:
			game_system=NES;
			tiles_main.tileSize=16;
			shadow_highlight_switch->hide();
			for (unsigned char c=0;c<16;c++)
			{
				palette[c]=to_nes_color(c);
			}
			palEdit.changeSystem();
			tileEdit_pal.changeSystem();
			tileMap_pal.changeSystem();

			tiles_main.tileDat = (unsigned char *)realloc(tiles_main.tileDat,(tiles_main.tiles_amount+1)*16);
		break;
		default:
			show_default_error
			return;
		break;
	}
	window->redraw();
}

void tilemap_remove_callback(Fl_Widget*,void*)
{
		char str[16];
		char * str_ptr;
		str_ptr=&str[0];
		str_ptr=(char *)fl_input("Enter Tile");
		if (str_ptr == 0)
		{
			return;
		}

		if (verify_str_number_only(str_ptr) == false)
		{
			return;
		}
		int tile=atoi(str_ptr);
		sub_tile_map(tile,tile-1);
		window->redraw();
}


void editor::_editor()
{
	//create the window
	menu = new Fl_Menu_Bar(0,0,800,25);		// Create menubar, items..
	menu->add("&File/&Open character palette",(int)0, Butt_CB,(void *)0,(int)0);
	menu->add("&File/&Open level palette",  (int)0, Butt_CB,(void *)32,(int)0);
	menu->add("&File/&Open tiles",(int)0,load_tiles,(void*)0,(int)0);
	menu->add("&File/&Open Truecolor Tiles",(int)0,load_truecolor_tiles,0,(int)0);
	menu->add("&File/&Append tiles",(int)0,load_tiles,(void*)1,(int)0);
	menu->add("&File/&Open tile map and if NES attrabiuts",(int)0,load_tile_map,(void *)0,(int)0);
	menu->add("&File/&import image to tilemap",(int)0,load_image_to_tilemap,(void *)0,(int)0);

	menu->add("&File/&Save character palette",  0, save_palette,(void*)0x0020);
	menu->add("&File/&Save level palette", 0, save_palette,(void*)0x2080);
	menu->add("&File/&Save all palette to one file",  0, save_palette,(void*)0x0080);
	menu->add("&File/&Save tiles",0,save_tiles,0,0);
	menu->add("&File/&Save truecolor tiles",0,save_tiles_truecolor,0,0);
	menu->add("&File/&Save tile map and if nes attributes",0,save_map,0,0);

	menu->add("&Action/&update dither all tiles",0,update_all_tiles,0,0);
	menu->add("&Action/&Fill tile with selected color",0,fill_tile,(void *)0,(int)0);
	menu->add("&Action/&Delete currently selected tile",0,delete_tile_at_location,(void *)0,(int)0);
	menu->add("&Action/&File tile map with selection includeing attrabutes",0,fill_tile_map_with_tile,(void *)0,(int)0);
	menu->add("&Action/&Dither tilemap as image",0,dither_tilemap_as_image,(void *)0,(int)0);
	menu->add("&Action/&generate optimal palette with one row",0,generate_optimal_palette,(void *)0,(int)0);
	menu->add("&Action/&generate optimal palette with four rows",0,generate_optimal_palette,(void *)4,(int)0);
	menu->add("&Action/&Auto determin if use shadow highlight",0,shadow_highligh_findout,(void *)0,(int)0);

	menu->add("&Tile Actions/&Append blank tile to end of buffer",0,new_tile,0,0);
	menu->add("&Tile Actions/&Fill tile with color 0",0,blank_tile,0,0);
	menu->add("&Tile Actions/&Remove duplicate truecolor tiles",0,remove_duplicate_truecolor,0,0);
	menu->add("&Tile Actions/&Remove duplicate tiles",0,remove_duplicate_tiles,0,0);
	menu->add("&TileMap Actions/&Remove tiles after x",0,tilemap_remove_callback,0,0);

	tile_placer_tile_offset_y=default_tile_placer_tile_offset_y;
	true_color_box_x=default_true_color_box_x;
	true_color_box_y=default_true_color_box_y;
	tile_edit_truecolor_off_x=default_tile_edit_truecolor_off_x;
	tile_edit_truecolor_off_y=default_tile_edit_truecolor_off_y;
	{ /*Fl_Tabs**/ the_tabs = new Fl_Tabs(0, 24, 800, 576);
	the_tabs->callback(set_mode_tabs);
		{Fl_Group* o = new Fl_Group(0, 48, 800, 576, "palette editor");
			//cout << "palette editor as group: " << o->as_group() << endl;
			pal_id=(intptr_t)o->as_group();
			//stuff realed to this group should go here
			
			palEdit.more_init(4);
			pal_size = new Fl_Hor_Value_Slider(128,384,320,24,"Palette box size");
			pal_size->minimum(1); pal_size->maximum(42);
			pal_size->step(1);
			pal_size->value(32);
			pal_size->align(FL_ALIGN_LEFT);
			pal_size->callback(update_box_size);
			
			ditherPower = new Fl_Hor_Value_Slider(128,416,320,24,"Dither Power");
			ditherPower->minimum(1); ditherPower->maximum(64);
			ditherPower->step(1);
			ditherPower->value(16);
			ditherPower->align(FL_ALIGN_LEFT);
			
			create_shadow_highlight_buttons();
		
			{
				Fl_Group *o = new Fl_Group(0, 0, 800, 480);
				{
					Fl_Round_Button* o = new Fl_Round_Button(128, 320, 96, 32, "Sega Genesis");
					o->tooltip("Sets the editing mode to Sega Genesis or Sega Mega Drive");
					o->type(102);
					o->down_box(FL_ROUND_DOWN_BOX);
					o->callback((Fl_Callback*) set_game_system,(void *)sega_genesis);
				} // Fl_Round_Button* o
				{
					Fl_Round_Button* o = new Fl_Round_Button(256, 320, 64, 32, "NES");
					o->tooltip("Sets the editing mode to Nintendo Entertamint System or Famicon");
					o->type(102);

					o->down_box(FL_ROUND_DOWN_BOX);
					o->callback((Fl_Callback*) set_game_system,(void *)NES);
				} // Fl_Round_Button* o
				o->end();
			} // End of buttons

			{
				Fl_Group *o = new Fl_Group(0, 0, 800, 500);
				{
					Fl_Round_Button* o = new Fl_Round_Button(128, 440, 96, 32, "Floyd Steinberg");
					o->type(102);
					o->down_box(FL_ROUND_DOWN_BOX);
					o->callback((Fl_Callback*) set_ditherAlg,(void *)0);
				} // Fl_Round_Button* o
				{
					Fl_Round_Button* o = new Fl_Round_Button(256, 440, 64, 32, "Reimesha");
					o->type(102);
					o->down_box(FL_ROUND_DOWN_BOX);
					o->callback((Fl_Callback*) set_ditherAlg,(void *)1);
				} // Fl_Round_Button* o
				{
					Fl_Round_Button* o = new Fl_Round_Button(384, 440, 64, 32, "Nearest Color");
					o->type(102);
					o->down_box(FL_ROUND_DOWN_BOX);
					o->callback((Fl_Callback*) set_ditherAlg,(void *)2);
				} // Fl_Round_Button* o
				o->end();
			} // End of buttons


      			o->end();
			} // Fl_Group* o


		{ Fl_Group* o = new Fl_Group(5, 48, 800, 567, "Tile Editor");
			//stuff realed to this group should go here
			tile_edit_id=(intptr_t)o->as_group();
		//o->callback(set_mode_tabs);
			{ Fl_Group* o = new Fl_Group(0, 0, 800, 567);
				{ Fl_Round_Button* o = new Fl_Round_Button(384, default_palette_bar_offset_y+40, 56, 32, "Row 0");
					//o->tooltip("Radio button, only one button is set at a time, in the corresponding group.");
					o->type(102);
					o->down_box(FL_ROUND_DOWN_BOX);
					o->callback((Fl_Callback*) set_tile_row,(void *)0);
				} // Fl_Round_Button* o
				{ Fl_Round_Button* o = new Fl_Round_Button(448, default_palette_bar_offset_y+40, 56, 32, "Row 1");
					//o->tooltip("Radio button, only one button is set at a time, in the corresponding group.");
					o->type(102);
					o->down_box(FL_ROUND_DOWN_BOX);
					o->callback((Fl_Callback*) set_tile_row,(void *)1);
				} // Fl_Round_Button* o
				{ Fl_Round_Button* o = new Fl_Round_Button(512, default_palette_bar_offset_y+40, 56, 32, "Row 2");
					//o->tooltip("Radio button, only one button is set at a time, in the corresponding group.");
					o->type(102);
					o->down_box(FL_ROUND_DOWN_BOX);
					o->callback((Fl_Callback*) set_tile_row,(void *)2);
				} // Fl_Round_Button* o
				{ Fl_Round_Button* o = new Fl_Round_Button(576, default_palette_bar_offset_y+40, 56, 32, "Row 3");
					//o->tooltip("Radio button, only one button is set at a time, in the corresponding group.");
					o->type(102);
					o->down_box(FL_ROUND_DOWN_BOX);
					o->callback((Fl_Callback*) set_tile_row,(void *)3);
				} // Fl_Round_Button* o

			o->end();
		} // Fl_Group* o
			{ Fl_Check_Button* o = new Fl_Check_Button(640,default_palette_bar_offset_y+40,120,32,"Show grid?");
				o->callback(set_grid);
				o->tooltip("This button Toggles wheater or not you which to see a grid while editing your tiles. A grid can help you see the spacing betwen each pixel.");
			}
			{ Fl_Button *o = new Fl_Button(540, default_palette_bar_offset_y, 120, 32, "New Tile");//these button should be inline with the palette bar
				o->tooltip("This will append a blank tile to the tile buffer in the ram.");
				o->callback(new_tile);
			}
			{ Fl_Button *o = new Fl_Button(668, default_palette_bar_offset_y, 128, 32, "Delete Selected Tile");
				o->tooltip("This button will delete the curretly selected tile");
				o->callback(delete_tile_at_location);
			}

			tileEdit_pal.more_init();

			//The reason what makes sega genesis toolkit the best grapics solution is that it will allow for true color images and automaticlly dither them

			rgb_red = new Fl_Hor_Value_Slider(48,default_palette_bar_offset_y+136,128,24,"RGB red");
			rgb_red->minimum(0);
			rgb_red->maximum(255);
			rgb_red->step(1);

			rgb_red->value(0);
			rgb_red->align(FL_ALIGN_LEFT);
			rgb_red->callback(update_truecolor,(void *)0);

			rgb_green = new Fl_Hor_Value_Slider(184,default_palette_bar_offset_y+136,128,24,"Green");
			rgb_green->minimum(0);
			rgb_green->maximum(255);
			rgb_green->step(1);
			rgb_green->value(0);

			rgb_green->align(FL_ALIGN_LEFT);
			rgb_green->callback(update_truecolor,(void *)1);

			rgb_blue = new Fl_Hor_Value_Slider(320,default_palette_bar_offset_y+136,128,24,"Blue");
			rgb_blue->minimum(0);
			rgb_blue->maximum(255);
			rgb_blue->step(1);
			rgb_blue->value(0);
			rgb_blue->align(FL_ALIGN_LEFT);
			rgb_blue->callback(update_truecolor,(void *)2);

			rgb_alpha = new Fl_Hor_Value_Slider(456,default_palette_bar_offset_y+136,128,24,"Alpha");
			rgb_alpha->minimum(0);
			rgb_alpha->maximum(255);
			rgb_alpha->step(1);
			rgb_alpha->value(0);
			rgb_alpha->align(FL_ALIGN_LEFT);
			rgb_alpha->callback(update_truecolor,(void *)3);

			tile_edit_offset_x=default_tile_edit_offset_x;

			tile_edit_offset_y=default_tile_edit_offset_y;
			tile_size = new Fl_Hor_Value_Slider(496,default_palette_bar_offset_y+72,304,24,"Tile Zoom Factor");
			tile_size->tooltip("This slider sets magification a value of 10 would mean the image is being displayed 10 times larger");
			tile_size->minimum(1);
			tile_size->maximum(64);
			tile_size->step(1);
			tile_size->value(46);
			tile_size->align(FL_ALIGN_LEFT);
			tile_size->callback(update_offset_tile_edit);

			//now for the tile select slider
			tile_select = new Fl_Hor_Value_Slider(480,default_palette_bar_offset_y+104,320,24,"Tile Select");
			tile_select->tooltip("This slider selects which tile that you are editing the first tile is zero");
			tile_select->minimum(0);
			tile_select->maximum(0);
			tile_select->step(1);
			tile_select->value(0);
			tile_select->align(FL_ALIGN_LEFT);
			tile_select->callback(set_tile_current);
			o->end();
		}
		{ Fl_Group* o = new Fl_Group(5, 48, 640, 480, "Plane Mapping Editor");
			//o->callback(set_mode_tabs);
			tile_place_id=(intptr_t)o->as_group();


			{ Fl_Group* o = new Fl_Group(0, 0, 800, 567);

				{ Fl_Round_Button* o = new Fl_Round_Button(tile_place_buttons_x_off, 208, 60, 32, "Row 0");
					//o->tooltip("Radio button, only one button is set at a time, in the corresponding group.");
					o->type(102);
					o->down_box(FL_ROUND_DOWN_BOX);
					o->callback((Fl_Callback*) set_tile_row,(void *)0);
				} // Fl_Round_Button* o
				{ Fl_Round_Button* o = new Fl_Round_Button(tile_place_buttons_x_off, 240, 60, 32, "Row 1");
					//o->tooltip("Radio button, only one button is set at a time, in the corresponding group.");
					o->type(102);
					o->down_box(FL_ROUND_DOWN_BOX);
					o->callback((Fl_Callback*) set_tile_row,(void *)1);
				} // Fl_Round_Button* o
				{ Fl_Round_Button* o = new Fl_Round_Button(tile_place_buttons_x_off, 272, 60, 32, "Row 2");
					//o->tooltip("Radio button, only one button is set at a time, in the corresponding group.");
					o->type(102);
					o->down_box(FL_ROUND_DOWN_BOX);
					o->callback((Fl_Callback*) set_tile_row,(void *)2);
				} // Fl_Round_Button* o
				{ Fl_Round_Button* o = new Fl_Round_Button(tile_place_buttons_x_off, 304, 60, 32, "Row 3");
					//o->tooltip("Radio button, only one button is set at a time, in the corresponding group.");
					o->type(102);
					o->down_box(FL_ROUND_DOWN_BOX);
					o->callback((Fl_Callback*) set_tile_row,(void *)3);
				} // Fl_Round_Button* o

			o->end();
		} // Fl_Group* o
			//callback_resize_map
			map_w = new Fl_Hor_Value_Slider(480,default_palette_bar_offset_y+72,312,24,"Map width");
			map_w->minimum(1);
			map_w->maximum(0xFFFF);
			map_w->step(1);
			map_w->value(2);
			map_w->align(FL_ALIGN_LEFT);
			map_w->callback(callback_resize_map);

			map_h = new Fl_Hor_Value_Slider(480,default_palette_bar_offset_y+104,312,24,"Map height");
			map_h->minimum(1);
			map_h->maximum(0xFFFF);
			map_h->step(1);
			map_h->value(2);
			map_h->align(FL_ALIGN_LEFT);
			map_h->callback(callback_resize_map);

			map_x_scroll = new Fl_Scrollbar(default_map_off_x-32, default_map_off_y-42, 800-8-default_map_off_x, 24);
			map_x_scroll->value(0,0,0,0);
			map_x_scroll->type(FL_HORIZONTAL);
			map_x_scroll->tooltip("Use this scroll bar to move around the tile map if you are zoomed in and there is not enough room to display the entire tilemap at once. This scroll bar will move the map left and right.");
			map_x_scroll->callback(update_map_scroll_x);

			map_y_scroll = new Fl_Scrollbar(default_map_off_x-32, default_map_off_y, 24, 600-8-default_map_off_y);
			map_y_scroll->value(0,0,0,0);
			//map_x_scroll->type(FL_HORIZONTAL);
			map_y_scroll->tooltip("Use this scroll bar to move around the tile map if you are zoomed in and there is not enough room to display the entire tilemap at once. This scroll bar will move the map up and down.");
			map_y_scroll->callback(update_map_scroll_y);


			//now for the tile select slider
			tile_select_2 = new Fl_Hor_Value_Slider(480,default_palette_bar_offset_y+40,312,24,"Tile Select");
			tile_select_2->tooltip("This slider allows you to choice which tile you would like to place on the map remember you can both horizontally and verticlly flip the tile once placed on the map and select which row the tile uses");
			tile_select_2->minimum(0);
			tile_select_2->maximum(0);
			tile_select_2->step(1);
			tile_select_2->value(0);
			tile_select_2->align(FL_ALIGN_LEFT);
			tile_select_2->callback(set_tile_current);

			tileMap_pal.more_init();

			//buttons for tile settings
			{ Fl_Check_Button* o = new Fl_Check_Button(tile_place_buttons_x_off,336,64,32,"hflip");
				o->callback(set_hflip);
				o->tooltip("This sets whether or not the tile is flipped horizontally");
			}
			{ Fl_Check_Button* o = new Fl_Check_Button(tile_place_buttons_x_off,368,64,32,"vflip");
				o->callback(set_vflip);
				o->tooltip("This sets whether or not the tile is flipped vertically");
			}
			{ Fl_Check_Button* o = new Fl_Check_Button(tile_place_buttons_x_off,400,68,32,"priority");
				o->callback(set_prio_callback);
				o->tooltip("If checked tile is high priority");
			}
			{ Fl_Check_Button* o = new Fl_Check_Button(tile_place_buttons_x_off,432,96,32,"Show grid?");
				o->callback(set_grid_placer);
				o->tooltip("This button Toggles wheater or not you a grid is visable over the tilemap this will allow you to easilly see were each tile is");
			}
			place_tile_size = new Fl_Hor_Value_Slider(tile_place_buttons_x_off+96,496,128,24,"Tile Zoom Factor");
			place_tile_size->minimum(1);
			place_tile_size->maximum(16);
			place_tile_size->step(1);
			place_tile_size->value(12);
			place_tile_size->align(FL_ALIGN_LEFT);
			place_tile_size->callback(update_map_size);
			place_tile_size->tooltip("By chaning this slider you are changing the magnification of the tile for example if this slider was set to 10 that would mean that the tile is magnificafied by a factor of 10");
			o->end();
		}
	}

}

int main(int argc, char **argv)
{
	printf("Welcome to Retro graphics Toolkit\nWritten by sega16/nintendo8\nBuild %s %s\n",__DATE__,__TIME__);
	window->resizable(window);
	Fl::scheme("plastic");
	fl_register_images();
	tile_map=(unsigned char *)malloc(16);
	if (tile_map == 0)
	{
		show_malloc_error(12)
	}
	map_size_x=2;
	map_size_y=2;
	unsigned char abc;
	for (abc=0;abc<16;abc++)
	{
		tile_map[abc]=0x00;
	}
	palette_muliplier=18;
	palette_adder=0;
	//// For a nicer looking browser under linux, call Fl_File_Icon::load_system_icons();
	//// (If you do this, you'll need to link with fltk_images)
	//// NOTE: If you do not load the system icons, the file chooser will still work, but
	////       no icons will be shown. However, this means you do not need to link in the
	////       fltk_images library, potentially reducing the size of your executable.
	//// Loading the system icons is not required by the OSX or Windows native file choosers.


	//note I have not personnly tested this on another operation system just windows these were from sample FLTK code
#if !defined(WIN32) && !defined(__APPLE__)
  Fl_File_Icon::load_system_icons();
#endif

  int argn = 1;
#ifdef __APPLE__
  // OS X may add the process number as the first argument - ignore
  if (argc>argn && strncmp(argv[1], "-psn_", 5)==0)
    argn++;
#endif

 // window.end();
  window->show(argc,argv);
	return Fl::run();
}
