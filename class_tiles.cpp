#include "global.h"
#include "class_tiles.h"
#include "dither.h"
#include "tilemap.h"
tiles tiles_main;
tiles::tiles()
{
	tileDat=(uint8_t *)calloc(32,1);
	if (tileDat == 0)
	{
		printf("Error in init class\n");//I am using printf instead of fl_alert as the gui will not be created when this is called
		exit(1);//the program will not be able to function at all without this working
	}
	truetileDat=(uint8_t *)calloc(256,1);
	if (truetileDat == 0)
	{
		printf("Error in init class\n");
		exit(1);
	}
	tileSize=32;
	tileName="default";
}
tiles::~tiles()
{
	free(tileDat);
	free(truetileDat);
}
void tiles::remove_tile_at(uint32_t tileDel)
{
	if (tiles_amount == 0)//a value of 0 for tiles_amount means 1 tile.
	{
		fl_alert("You must have atleast one tile");
		return;
	}
	if (tileDel < tiles_amount)
	{
		memmove(&(tileDat[tileDel*tileSize]),&(tileDat[(tileDel*tileSize)+tileSize]),(tiles_amount*tileSize)-(tileDel*tileSize));
		memmove(&(truetileDat[tileDel*256]),&(truetileDat[(tileDel*256)+256]),(tiles_amount*256)-(tileDel*256));
	}
	tileDat=(uint8_t *)realloc(tileDat,tiles_amount*tileSize);
	if (tileDat == 0)
	{
		show_realloc_error(tiles_amount*tileSize)
		exit(1);
	}
	truetileDat=(uint8_t *)realloc(truetileDat,tiles_amount*256);
	tiles_amount--;
	window->tile_select->maximum(tiles_amount);
	window->tile_select_2->maximum(tiles_amount);
	if (current_tile > tiles_amount)
	{
		current_tile=tiles_amount;
		window->tile_select->value(current_tile);
		window->tile_select_2->value(current_tile);
	}
	
}
void tiles::truecolor_to_tile(uint8_t palette_row,uint32_t cur_tile)
{
	//dithers a truecolor tile to tile
	unsigned int tile_256=cur_tile*256;
	unsigned int tile_32=cur_tile*32;
	unsigned int tile_16=cur_tile*16;
	unsigned char true_color_temp[256];
	/*for (uint16_t x=0;x<256;x++)
	{
		true_color_temp[x]=truetileDat[x+tile_256];
	}*/
	memcpy(true_color_temp,&truetileDat[tile_256],256);
	if (game_system == NES)
	{
		//part of the NES tile convertsion code uses the bitwise OR operation
		//we need to clear the old tile data first
		for (unsigned char x=0;x<16;x++)
		{
			tileDat[x+tile_16]=0;
		}
	}
	ditherImage(&true_color_temp[0],8,8,true);
	//now image needs to be checked for alpha
	uint8_t * truePtr=true_color_temp;
	/*for (uint16_t x=0;x<64;x++)
	{
		if (truePtr[3] == 0)
		{
			//clear pixels
			*truePtr=0;
			truePtr[1]=0;
			truePtr[2]=0;
		}
		truePtr+=4;
	}*/
	truePtr=true_color_temp;
	for (unsigned char y=0;y<8;y++)
	{
		for (unsigned char x=0;x<8;x++)
		{
			unsigned char r_old;
			unsigned char g_old;
			unsigned char b_old;
			r_old=*truePtr++;
			g_old=*truePtr++;
			b_old=*truePtr++;
			//truePtr++;
			unsigned char temp=find_near_color_from_row(palette_row,r_old,g_old,b_old);

			//get difference
			//sega genesis tile format
			//even pixel,odd pixel
			switch (game_system)
			{
				case sega_genesis:
					if (x & 1)
					{
						//this is an odd (not a strange pixel but odd number) pixel
						if (*truePtr++ != 0)
							tileDat[(y*4)+(x/2)+tile_32]|=temp;
					}
					else
					{
						//even pixel
						if (*truePtr++ != 0)
							tileDat[(y*4)+(x/2)+tile_32]=temp<<4;
						else
							tileDat[(y*4)+(x/2)+tile_32]=0;
					}
				break;
				case NES:
					if (*truePtr++ != 0)
					{
						tileDat[y+tile_16]|=(temp&1)<<(7-x);
						tileDat[y+8+tile_16]|=((temp>>1)&1)<<(7-x);
					}
				break;
			}
		}

	}//end of loop

}
void tiles::draw_truecolor(uint32_t tile,uint16_t x,uint16_t y,bool usehflip,bool usevflip,uint8_t zoom)
{
	uint8_t xx,yy,zz;
	uint8_t trueColTemp[256];
	uint8_t grid[192];
	uint8_t * grid_ptr=grid;
	uint8_t * truePtr;
	for (zz=0;zz<4;zz++)
	{
		for (xx=0;xx<4;xx++)
		{
			for (yy=0;yy<3;yy++)
				*grid_ptr++=255;
			for (yy=0;yy<3;yy++)
				*grid_ptr++=160;
		}
		for (xx=0;xx<4;xx++)
		{
			for (yy=0;yy<3;yy++)
				*grid_ptr++=160;
			for (yy=0;yy<3;yy++)
				*grid_ptr++=255;
		}
	}
	if (usehflip == false && usevflip == false)
		memcpy(trueColTemp,&truetileDat[tile*256],256);
	else if (usehflip == true && usevflip == false)
		hflip_truecolor(tile,(uint32_t *)trueColTemp);
	else if (usehflip == false && usevflip == true)
		vflip_truecolor(tile,trueColTemp);
	else
	{
		hflip_truecolor(tile,(uint32_t *)trueColTemp);
		vflip_truecolor_ptr(trueColTemp,trueColTemp);
	}
	truePtr=&trueColTemp[3];
	grid_ptr=grid;
	for (zz=0;zz<64;zz++)
	{
		for (xx=0;xx<3;xx++)
		{
			if (*truePtr != 0)//prevent divide by 0
			{
				double percent=(double)*truePtr/255.0;
				*grid_ptr++=((double)trueColTemp[(zz*4)+xx]*percent)+((double)*grid_ptr*(1.0-percent));
			}
			else
				grid_ptr++;
		}
		truePtr+=4;//next alpha value
	}
	for (yy=0;yy<8;yy++)
	{
		for (xx=0;xx<8;xx++)
		{
			//I have the tile in interleaved rgb 24
			fl_rectf(x+(xx*zoom),y+(yy*zoom),zoom,zoom,grid[(((yy*8)+xx)*3)],grid[1+(((yy*8)+xx)*3)],grid[2+(((yy*8)+xx)*3)]);
		}
	}
}
inline unsigned int cal_offset_zoom_rgb(uint16_t x,uint16_t y,uint16_t zoom,uint8_t channel)
{
	return (y*(zoom*24))+(x*3)+channel;
}
void tiles::draw_tile(uint16_t x_off,uint16_t y_off,uint32_t tile_draw,uint8_t zoom,uint8_t pal_row,bool Usehflip,bool Usevflip)
{
	if (tiles_amount < tile_draw)
	{
		fl_alert("Warning tried to draw tile # %d at X: %d y: %d\nBut there is only %d tiles (zero is first tile).",tile_draw,x_off,y_off,tiles_amount);
		return;
	}
	
	unsigned char a;
	char x,y;
	uint8_t * temp_img_ptr = (uint8_t *)malloc(((8*zoom)*(8*zoom))*3);
	if (temp_img_ptr == 0)
	{
		show_malloc_error(((8*zoom)*(8*zoom))*3)
	}
	unsigned char c,d;//used for drawing pixels to buffer
	uint8_t red_temp,green_temp,blue_temp;
	uint8_t tileTemp[32];
	if (Usehflip == true && Usevflip == false)//it is importan to make sure vflip is false or else this could be ran when vflip==true
		hflip_tile(tile_draw,tileTemp);
	else if (Usehflip == false && Usevflip == true)
		vflip_tile(tile_draw,tileTemp);
	else if (Usehflip == true && Usevflip == true)
	{
		hflip_tile(tile_draw,tileTemp);
		vflip_tile_ptr(tileTemp,tileTemp);//vflip creates temp buffer having dst and src to be the same will not cause issue
	}
	else
		memcpy(tileTemp,&tileDat[tile_draw*tileSize],tileSize);
	tile_draw*=tileSize;
	switch (game_system)
	{
		case sega_genesis:
			for (y=0;y<8;y++)
			{
				for (x=0;x<4;x++)
				{
					//get two pixels
					unsigned char temp=tileTemp[(y*4)+x];
					//split the two pixels
					unsigned char temp_1,temp_2;
					//first,second pixel
					temp_1=temp>>4;//first pixel
					temp_2=temp&15;//second pixel
					//now based on the temp_1 and temp_2 get the two colors
					red_temp=rgb_pal[(pal_row*48)+(temp_1*3)];
					green_temp=rgb_pal[(pal_row*48)+(temp_1*3)+1];
					blue_temp=rgb_pal[(pal_row*48)+(temp_1*3)+2];
					for (c=0;c<zoom;c++)//ha ha c++ bad programming pun
					{
						for (d=0;d<zoom;d++)
						{
							temp_img_ptr[cal_offset_zoom_rgb(((x*zoom)*2)+d,(y*zoom)+c,zoom,0)]=red_temp;
							temp_img_ptr[cal_offset_zoom_rgb(((x*zoom)*2)+d,(y*zoom)+c,zoom,1)]=green_temp;
							temp_img_ptr[cal_offset_zoom_rgb(((x*zoom)*2)+d,(y*zoom)+c,zoom,2)]=blue_temp;
						}
					}
					red_temp=rgb_pal[(pal_row*48)+(temp_2*3)];
					green_temp=rgb_pal[(pal_row*48)+(temp_2*3)+1];
					blue_temp=rgb_pal[(pal_row*48)+(temp_2*3)+2];
					for (c=0;c<zoom;c++)//ha ha c++ bad programming pun
					{
						for (d=0;d<zoom;d++)
						{
							temp_img_ptr[cal_offset_zoom_rgb(((x*zoom)*2)+d+zoom,(y*zoom)+c,zoom,0)]=red_temp;
							temp_img_ptr[cal_offset_zoom_rgb(((x*zoom)*2)+d+zoom,(y*zoom)+c,zoom,1)]=green_temp;
							temp_img_ptr[cal_offset_zoom_rgb(((x*zoom)*2)+d+zoom,(y*zoom)+c,zoom,2)]=blue_temp;
						}
					}
				}
			}
		break;
			case NES:
				for (y=0;y<8;y++)
				{
					for (x=0;x<8;x++)
					{
						uint8_t temp;
						temp=(tileTemp[y]>>(7-x))&1;
						temp|=((tileTemp[y+8]>>(7-x))&1)<<1;
						red_temp=rgb_pal[(pal_row*12)+(temp*3)];
						green_temp=rgb_pal[(pal_row*12)+(temp*3)+1];
						blue_temp=rgb_pal[(pal_row*12)+(temp*3)+2];
						for (c=0;c<zoom;c++)//yes the same old c++ joke I wonder how many program have it
						{
							for (d=0;d<zoom;d++)
							{
								temp_img_ptr[cal_offset_zoom_rgb((x*zoom)+d,(y*zoom)+c,zoom,0)]=red_temp;
								temp_img_ptr[cal_offset_zoom_rgb((x*zoom)+d,(y*zoom)+c,zoom,1)]=green_temp;
								temp_img_ptr[cal_offset_zoom_rgb((x*zoom)+d,(y*zoom)+c,zoom,2)]=blue_temp;
							}
						}
					}
				}
			break;
		}
	fl_draw_image(temp_img_ptr,x_off,y_off,8*zoom,8*zoom,3);
	free(temp_img_ptr);
}

void tiles::hflip_truecolor(uint32_t id,uint32_t * out)
{
	//out must contaian at least 256 bytes
	uint8_t y;
	uint32_t * trueColPtr=(uint32_t *)&truetileDat[id*256];
	trueColPtr+=7;//32-4 28/4
	for (y=0;y<8;y++)
	{
		*out++=*trueColPtr--;//this is 32bit so only one copy needed
		*out++=*trueColPtr--;
		*out++=*trueColPtr--;
		*out++=*trueColPtr--;
		*out++=*trueColPtr--;
		*out++=*trueColPtr--;
		*out++=*trueColPtr--;
		*out++=*trueColPtr--;
		trueColPtr+=16;//64/4 next line
	}
}
void tiles::vflip_truecolor_ptr(uint8_t * in,uint8_t * out)
{//this needs to be a seperate function as the out of hflip may be inputted here to form vhfliped tile
	uint16_t y;
	uint8_t temp[256];
	memcpy(temp,in,256);
	for (y=0;y<256;y+=32)
	{
		memcpy(&out[224-y],&temp[y],32);
	}
}
void tiles::vflip_truecolor(uint32_t id,uint8_t * out)
{
	vflip_truecolor_ptr(&truetileDat[id*256],out);
}
inline uint8_t swap_4bit(uint8_t in)
{
	return ((in >> 4) & 0x0f) | ((in << 4) & 0xf0);
}
inline uint8_t reverse_bits(uint8_t b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}
void tiles::hflip_tile(uint32_t id,uint8_t * out)
{
	uint8_t x,y;
	uint8_t * tilePtr=&tileDat[id*tileSize];
	switch (game_system)
	{
		case sega_genesis:
			memcpy(out,tilePtr,32);
			for (y=0;y<8;y++)
			{
				for (x=0;x<4;x++)
					*out++=swap_4bit(*out);
			}
			out-=tileSize;
			uint8_t temp[4];
			for (y=0;y<8;y++)
			{
				memcpy(temp,out,4);
				*out++=temp[3];
				*out++=temp[2];
				*out++=temp[1];
				*out++=temp[0];
			}
		break;
		case NES:
			memcpy(out,tilePtr,16);
			for (y=0;y<16;y++)
				*out++=reverse_bits(*out);
		break;
	}
}
void tiles::vflip_tile_ptr(uint8_t * in,uint8_t * out)
{
	uint8_t y;
	switch (game_system)
	{
		case sega_genesis:
		{//brackts are used so compiler knows how long they are in scope
			uint8_t temp[32];
			memcpy(temp,in,32);
			for (y=0;y<32;y+=4)
			{
				memcpy(&out[28-y],&temp[y],4);
			}
		}
		break;
		case NES:
		{
			uint8_t temp[16];
			memcpy(temp,in,16);
			//NES uses different format seperated into 2 peices
			for (y=0;y<8;y++)
				out[y]=temp[7-y];
			for (y=0;y<8;y++)
				out[y+8]=temp[7-y+8];
		}
		break;
	}
}
void tiles::vflip_tile(uint32_t id,uint8_t * out)
{
	vflip_tile_ptr(&tileDat[id*tileSize],out);
}
void tiles::blank_tile(uint32_t tileUsage)
{
	if (mode_editor == tile_edit)
	{
		memset(&truetileDat[tileUsage*256],0,256);
		truecolor_to_tile(tileEdit_pal.theRow,tileUsage);
	}
	else
	{
		memset(&tileDat[tileUsage*tileSize],0,tileSize);
	}
}
void tiles::remove_duplicate_tiles()
{
	uint32_t tile_remove_c=0;
	int32_t cur_tile,curT;
	puts("Pass 1");
	for (cur_tile=0;cur_tile<=tiles_amount;cur_tile++)
	{
		for (curT=tiles_amount;curT>=0;curT--)
		{
			if (cur_tile == curT)//dont compare it's self
				continue;
			#if __LP64__
			if (cmp_tiles(cur_tile,(uint64_t *)&tileDat[curT*tileSize]))
			#else
			if (cmp_tiles(cur_tile,(uint32_t *)&tileDat[curT*tileSize]))
			#endif
			{
				sub_tile_map(curT,cur_tile,false,false);
				remove_tile_at(curT);
				tile_remove_c++;
				printf("Deleted tile %d\nRemoved %d tiles\n",curT,tile_remove_c);
			}
		}
		printf("On tile %d Removed %d\n",cur_tile,tile_remove_c);
	}
	printf("Removed %d tiles\n",tile_remove_c);
	tile_remove_c=0;
}
#if __LP64__
bool tiles::cmp_trueC(uint32_t one,uint64_t * two)
{//this should be faster than memcmp as it returns as soon as there is a difference
	uint64_t * onePtr =(uint64_t *)&truetileDat[one*256];
	for (uint8_t x=0;x<32;x++)
	{
		if (*onePtr++ != *two++)
			return false;
	}
	return true;
}
#else
bool tiles::cmp_trueC(uint32_t one,uint32_t * two)
{//this should be faster than memcmp as it returns as soon as there is a difference
	uint32_t * onePtr =(uint32_t *)&truetileDat[one*256];
	for (uint8_t x=0;x<64;x++)
	{
		if (*onePtr++ != *two++)
			return false;
	}
	return true;
}
#endif
#if __LP64__
bool tiles::cmp_tiles(uint32_t one,uint64_t * two)
{
	uint64_t * onePtr =(uint64_t *)&tileDat[one*tileSize];
	for (uint8_t x=0;x<tileSize;x+=8)
	{
		if (*onePtr++ != *two++)
			return false;
	}
	return true;
}
#else
bool tiles::cmp_tiles(uint32_t one,uint32_t * two)
{
	uint32_t * onePtr =(uint32_t *)&tileDat[one*tileSize];
	for (uint8_t x=0;x<tileSize;x+=4)
	{
		if (*onePtr++ != *two++)
			return false;
	}
	return true;
}
#endif
void tiles::get_tiles(uint8_t * fromTiles,uint8_t * fromTrueTiles,uint32_t theAmount)
{
	switch(game_system)
	{
		case sega_genesis:
			tileSize=32;
		break;
		case NES:
			tileSize=16;
		break;
	}
	if (tileDat != 0)
		free(tileDat);
	if (truetileDat != 0)
		free(truetileDat);
	tileDat=(uint8_t *)malloc(theAmount*tileSize);
	truetileDat=(uint8_t *)malloc(theAmount*256);
	memcpy(tileDat,fromTiles,theAmount*tileSize);
	memcpy(truetileDat,fromTrueTiles,theAmount*256);
	tiles_amount=theAmount;
}
