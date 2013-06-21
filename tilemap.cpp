/*
Stuff related to tilemap operations goes here*/
#include "global.h"
#include "quant.h"
#include "color_convert.h"
#include "dither.h"
#include "spatial_color_quant.h"
tileMap::tileMap()
{
	mapSizeW=2;
	mapSizeH=2;
	selection=false;
	tileMapDat=(uint8_t *)calloc(16,1);
}
tileMap::~tileMap()
{
	free(tileMapDat);
}
bool tileMap::get_hflip(uint16_t x,uint16_t y)
{
	return (tileMapDat[((y*mapSizeW)+x)*4]>>3)&1;
}
bool tileMap::get_vflip(uint16_t x,uint16_t y)
{
	return (tileMapDat[((y*mapSizeW)+x)*4]>>4)&1;
}
bool tileMap::get_prio(uint16_t x,uint16_t y)
{
	return (tileMapDat[((y*mapSizeW)+x)*4]>>7)&1;
}
uint8_t tileMap::get_palette_map(uint16_t x,uint16_t y)
{
	return (tileMapDat[((y*mapSizeW)+x)*4]>>5)&3;
}
uint32_t tileMap::get_tile(uint16_t x,uint16_t y)
{
	//first calulate which tile we want
	if (mapSizeW < x || mapSizeH < y) {
		fl_alert("Error tried to get a non-existent tile on the map");
		return 0;
	}
	uint32_t selected_tile=((y*mapSizeW)+x)*4;
	//get both bytes
	uint8_t temp_1,temp_2,temp_3;
	temp_1=tileMapDat[selected_tile+1];//least sigficant is stored in the lowest address
	temp_2=tileMapDat[selected_tile+2];
	temp_3=tileMapDat[selected_tile+3];//most sigficant
	return (temp_1<<16)+(temp_2<<8)+temp_3;
}
int32_t tileMap::get_tileRow(uint16_t x,uint16_t y,uint8_t useRow)
{
	//first calulate which tile we want
	uint32_t selected_tile=((y*mapSizeW)+x)*4;
	//get both bytes
	if (((tileMapDat[selected_tile]>>5)&3) == useRow) {
		uint8_t temp_1,temp_2,temp_3;
		temp_1=tileMapDat[selected_tile+1];//least sigficant is stored in the lowest address
		temp_2=tileMapDat[selected_tile+2];
		temp_3=tileMapDat[selected_tile+3];//most sigficant
		return (temp_1<<16)+(temp_2<<8)+temp_3;
	}
	else
		return -1;
}
#if _WIN32
inline uint16_t swap_word(uint16_t w)
{
	uint8_t a,b;
	a=w&255;
	b=w>>8;
	return (a<<8)|b;
}
#endif
bool tileMap::saveToFile()
{
	/*!
	Saves tilemap to file returns true on success or cancelation
	returns false if there was an error but remeber if the user cancles this it is not an error
	*/
	//first see how this file should be saved
	uint16_t x,y;
	FILE * myfile;
	uint32_t fileSize;
	uint8_t type,compression, * mapptr;
	if (load_file_generic("Save tilemap to",true) == true){
		type=fl_choice("How would like this file saved?","Binary","C header",0);
		compression=fl_choice("In what format would you like this tilemap saved","Uncompressed","Enigma Compression",0);
		if (type == 1) {
			char temp[2048];
			myfile = fopen(the_file.c_str(),"w");
			sprintf(temp,"//Width %d Height %d",mapSizeW,mapSizeH);
			fputs((const char *)temp,myfile);
			if(compression==1)
				fputs(" Enigma compressed",myfile);
			fputc('\n',myfile);
			fputs("const uint8_t mapDat[]={",myfile);
		}
		else
			myfile = fopen(the_file.c_str(),"wb");
		if (likely(myfile!=0)) {
			switch (game_system) {
				case sega_genesis:
					{
					uint16_t * TheMap;
					fileSize=(mapSizeW*mapSizeH)*2;
					TheMap = (uint16_t*)malloc(fileSize);
					for (y=0;y<mapSizeH;y++){
						for (x=0;x<mapSizeW;x++){
							uint32_t tile=get_tile(x,y);
							if (tile > 2047){
								printf("Warning tile value %d exceeded 2047 at x: %d y: %d\n",tile,x,y);
								tile=2047;
							}
							#if _WIN32
							tile=swap_word(tile);//mingw appears not to provide htobe16 function
							#else
							tile=htobe16(tile);//needs to be big endian
							#endif
							*TheMap=(uint16_t)tileMapDat[((y*mapSizeW)+x)*4];//get attributes
							*TheMap++|=(uint16_t)tile;//add tile
						}
					}
					TheMap-=mapSizeW*mapSizeH;//return to begining so it can be freeded and the file saved
					mapptr=(uint8_t*)TheMap;
				}//brackets used to prevent TheMap conflict
				break;
				case NES:
				{
					uint8_t * TheMap;
					fileSize=mapSizeW*mapSizeH;
					TheMap = (uint8_t *)malloc(fileSize);
					for (y=0;y<mapSizeH;y++){
						for (x=0;x<mapSizeW;x++){
							uint32_t tile=get_tile(x,y);
							if (tile > 255) {
								printf("Warning tile value %d exceeded 255 at x: %d y: %d\n",tile,x,y);
								tile=255;
							}
							*TheMap++=tile;
						}
					}
					TheMap-=mapSizeW*mapSizeH;//return to begining so it can be freeded and the file sized
					mapptr=TheMap;
				}
				break;
			}
			if(compression==1){
				string input,output;
				std::ostringstream outcomp;
				enigma ecomp;
				input.assign((const char*)mapptr,fileSize);
				std::stringstream iss(input);
				ecomp.encode(iss,outcomp);
				output=outcomp.str();
				fileSize=outcomp.str().length();
				mapptr=(uint8_t*)realloc(mapptr,fileSize);
				output.copy((char*)mapptr,fileSize);
				printf("compressed to %d bytes\n",fileSize);
			}
			if (type == 1){
				if (saveBinAsText(mapptr,fileSize,myfile)==false){
					free(mapptr);
					return false;
				}
				fputs("};",myfile);
			}else
				fwrite(mapptr,1,fileSize,myfile);
			free(mapptr);
			fclose(myfile);
			puts("File Saved");
		}
		else
			return false;
	}
	if (game_system == NES){
		if (load_file_generic("Save attributes to",true) == true) {
			if (type == 1){
				myfile = fopen(the_file.c_str(),"w");
				fputs("const uint8_t attrDat[]={",myfile);
			}
			else
				myfile = fopen(the_file.c_str(),"wb");
			if (likely(myfile!=0)) {
				uint8_t * AttrMap = (uint8_t *)malloc((mapSizeW/4)*(mapSizeH/4));
				uint8_t * freeAttrMap=AttrMap;
				for (y=0;y<mapSizeH;y+=4) {
					for (x=0;x<mapSizeW;x+=4) {
						*AttrMap++ = get_palette_map(x,y) | (get_palette_map(x+2,y)<<2) | (get_palette_map(x,y+2) << 4) | (get_palette_map(x+2,y+2) << 6);
						printf("x: %d y: %d\n",x,y);
					}
				}
				//AttrMap-=(mapSizeW/4)*(mapSizeH/4);
				printf("%d %d\n",AttrMap,freeAttrMap);
				if (type == 1){
					if (saveBinAsText(freeAttrMap,(mapSizeW/4)*(mapSizeH/4),myfile)==false)
							return false;
						fputs("};",myfile);
				}
				else
					fwrite(freeAttrMap,1,(mapSizeW/4)*(mapSizeH/4),myfile);		
				free(freeAttrMap);
				fclose(myfile);
				puts("File Saved");
			}
			else
				return false;
		}
	}
	return true;
}
void zero_error_tile_map(int32_t x)
{/*! this is a long string I do not want it stored more than once*/
	fl_alert("Please enter value greater than zero you on the other hand entered %d",x);
}
bool tileMap::loadFromFile()
{
//start by loading the file
	/*Only will return false when there is a malloc error or file error
	the file saving user cancalation and not entering the number correctly return true*/
	uint32_t file_size;
	if (load_file_generic("Load tile map data") == true)
	{
		uint8_t compression=fl_choice("What kind of compression is this tilemap?","Uncompressed","Enigma Compressed",0);
		//get width and height
		string tilemap_file=the_file;
		int32_t w,h;
		char str[16];
		char * str_ptr;
		str_ptr=&str[0];
		str_ptr=(char *)fl_input("Enter Width");
		if (str_ptr == 0)
			return true;
		if (verify_str_number_only(str_ptr) == false)
			return true;
		w=atoi(str_ptr);
		if (w <= 0) {
			zero_error_tile_map(w);
			return true;
		}
		if (game_system == NES && (w & 1)) {
			fl_alert("Error unlike in sega genesis mode NES mode needs the width and height to be a multiple to 2");
			return true;
		}
		str_ptr=&str[0];
		str_ptr=(char *)fl_input("Enter Height");
		if (str_ptr == 0)
			return true;
		if (verify_str_number_only(str_ptr) == false)
			return true;
		h=atoi(str_ptr);
		if (h <= 0) {
			zero_error_tile_map(h);
			return true;
		}
		if (game_system == NES && (h & 1)) {
			fl_alert("Error unlike in sega genesis mode NES mode needs the width and height the be a multiple to 2");
			return true;
		}
		//we can now load the map
		str_ptr=&str[0];
		str_ptr=(char *)fl_input("Enter offset\nIf the tile map is generated by this program enter zero\nIf not then the first tile may not be zero\nLets say that the first tile is 200 you would enter -200 that makes the first tile zero\nIf you want the first tile to have an offset enter a positive number\nFor example if the tilemap has the first tile set to zero and you enter 5 the first tile will be tile 5");
		if (str_ptr == 0)
			return true;
		if (verify_str_number_only(str_ptr) == false)
			return true;
		int32_t offset=atoi(str_ptr);
		ifstream file (tilemap_file.c_str(), ios::in|ios::binary|ios::ate);
		file_size = file.tellg();
		file.seekg (0, ios::beg);//return to the beginning of the file
		uint32_t size_temp;
		switch (game_system) {
			case sega_genesis:
				size_temp=(w*h)*2;
			break;
			case NES:
				size_temp=w*h;
			break;
		}
		std::ostringstream outDecomp;
		if (compression==1) {
			enigma decomp;
			std::stringstream iss;
			iss << file.rdbuf();
			decomp.decode(iss,outDecomp);
			file_size=outDecomp.str().length();
			printf("Decompressed to %d bytes\n",file_size);
		}
		if (size_temp > file_size)
			fl_alert("Warning: The Tile map that you are attempting to load is smaller than a tile map that has the width and height that you specified\nThis missing data will be padded with tile zero");
		//start converting to tile
		//free(tile_map);
		tileMapDat = (uint8_t *) realloc(tileMapDat,(w*h)*4);
		uint8_t * tempMap = (uint8_t *) malloc(size_temp);
		if (unlikely(tileMapDat == 0))
			show_malloc_error(size_temp)
		if (compression==1) {
			string output=outDecomp.str();
			output.copy((char *)tempMap, file_size);
		}
		if (compression==0)
			file.read ((char *)tempMap, size_temp);
		file.close();
		window->map_w->value(w);
		window->map_h->value(h);
		mapSizeW=w;
		mapSizeH=h;
		uint16_t x,y;
		switch (game_system) {
			case sega_genesis:
				for (y=0;y<h;y++) {
					for (x=0;x<w;x++) {
						if (((x+(y*w)+1)*2) <= file_size) {
							uint16_t temp=*tempMap++;
							//set attributes
							tileMapDat[((y*mapSizeW)+x)*4]=(uint8_t)temp&0xF8;
							temp&=7;
							temp<<=8;
							temp|=(uint16_t)*tempMap++;
							if (temp+offset > 0)
								set_tile((int32_t)temp+offset,x,y);
							else
								set_tile(0,x,y);
							printf("Tile value %d at %d %d\n",temp+offset,x,y);
						}
						else
							set_tile(0,x,y);
					}
				}
			break;
			case NES:
				for (y=0;y<h;y++) {
					for (x=0;x<w;x++) {
						if ((x+(y*w)+1) <= file_size) {
							uint8_t temp=*tempMap++;
							if (temp+offset > 0)
								set_tile((int32_t)temp+offset,x,y);
							else
								set_tile(0,x,y);
							printf("Tile value %d at %d %d\n",temp+offset,x,y);
						}
						else
							set_tile(0,x,y);
					}
				}
				//now load attributes
				if (load_file_generic("Load Attribtues") == true){
					FILE * fp=fopen(the_file.c_str(),"wb");
					fseek(fp, 0L, SEEK_END);
					uint32_t sz=ftell(fp);
					rewind(fp);
					uint8_t* tempbuf=(uint8_t*)alloca(sz);
					fread(tempbuf,1,sz,fp);
					for (y=0;y<h;y+=4) {
						for (x=0;x<w;x+=4) {
							set_pal_row(x,y,*tempbuf&3);
							set_pal_row(x,y+1,*tempbuf&3);
							set_pal_row(x+1,y,*tempbuf&3);
							set_pal_row(x+1,y+1,*tempbuf&3);
							
							set_pal_row(x+2,y,((*tempbuf)>>2)&3);
							set_pal_row(x+2,y+1,((*tempbuf)>>2)&3);
							set_pal_row(x+3,y,((*tempbuf)>>2)&3);
							set_pal_row(x+3,y+1,((*tempbuf)>>2)&3);

							++tempbuf;
						}
					}
				}
			break;
		}
		tempMap-=file_size;
		free(tempMap);
		window->redraw();
	}
}
void tileMap::sub_tile_map(uint32_t oldTile,uint32_t newTile,bool hflip,bool vflip)
{
	uint16_t x,y;
	int32_t temp;
	for (y=0;y<mapSizeH;y++){
		for (x=0;x<mapSizeW;x++){
			temp=get_tile(x,y);
			if (temp == oldTile){
				set_tile(newTile,x,y);
				if (hflip == true)
					set_hflip(x,y,true);
				if (vflip == true)
					set_vflip(x,y,true);
			}
			else if (temp > oldTile){
				temp--;
				if (temp < 0)
					temp=0;
				set_tile(temp,x,y);
			}
		}
	}
}
bool truecolor_to_image(uint8_t * the_image,int8_t useRow,bool useAlpha)
{
	/*!
	the_image pointer to image must be able to hold the image using rgba 32bit
	useRow what row to use or -1 for no row
	*/
	if (the_image == 0) {
		fl_alert("Error malloc must be called before generating this image");
		return false;
	}
	//puts("Truecolor to image starting");
	uint32_t w,h;
	w=currentProject->tileMapC->mapSizeW*8;
	h=currentProject->tileMapC->mapSizeH*8;
	uint16_t x_tile=0,y_tile=0;
	uint32_t truecolor_tile_ptr=0;
	uint8_t pixelSize,pSize2;
	if (useAlpha) {
		pixelSize=4;
		pSize2=32;
	}else{
		pixelSize=3;
		pSize2=24;
	}
	if (useRow != -1) {
		for (uint64_t a=0;a<(h*w*pixelSize)-w*pixelSize;a+=w*pixelSize*8)//a tiles y
		{
			for (uint32_t b=0;b<w*pixelSize;b+=pSize2)//b tiles x
			{
				truecolor_tile_ptr=currentProject->tileMapC->get_tileRow(x_tile,y_tile,useRow)*256;
				if (truecolor_tile_ptr != -256){
					for (uint32_t y=0;y<w*pSize2;y+=w*pixelSize)//pixels y
					{
						if (useAlpha)
							memcpy(&the_image[a+b+y],&currentProject->tileC->truetileDat[truecolor_tile_ptr],32);
						else{
							uint8_t xx=0;
							for (uint8_t x=0;x<32;x+=4)//pixels x
							{
								the_image[a+b+y+xx]=currentProject->tileC->truetileDat[truecolor_tile_ptr+x];
								the_image[a+b+y+xx+1]=currentProject->tileC->truetileDat[truecolor_tile_ptr+x+1];
								the_image[a+b+y+xx+2]=currentProject->tileC->truetileDat[truecolor_tile_ptr+x+2];
								xx+=3;
							}
						}
						truecolor_tile_ptr+=32;
					}
				}
				else
				{
					for (uint32_t y=0;y<w*pSize2;y+=w*pixelSize)//pixels y
						memset(&the_image[a+b+y],0,pSize2);
				}
				x_tile++;
			}
			x_tile=0;
			y_tile++;
		}
	}else{
		for (uint64_t a=0;a<(h*w*pixelSize)-w*pixelSize;a+=w*pixelSize*8)//a tiles y
		{
			for (uint32_t b=0;b<w*pixelSize;b+=pSize2)//b tiles x
			{				
				truecolor_tile_ptr=currentProject->tileMapC->get_tile(x_tile,y_tile)*256;
				for (uint32_t y=0;y<w*pixelSize*8;y+=w*pixelSize)//pixels y
				{
					if (useAlpha)
						memcpy(&the_image[a+b+y],&currentProject->tileC->truetileDat[truecolor_tile_ptr],32);
					else{
						uint8_t xx=0;
						for (uint8_t x=0;x<32;x+=4)//pixels x
						{
							the_image[a+b+y+xx]=currentProject->tileC->truetileDat[truecolor_tile_ptr+x];
							the_image[a+b+y+xx+1]=currentProject->tileC->truetileDat[truecolor_tile_ptr+x+1];
							the_image[a+b+y+xx+2]=currentProject->tileC->truetileDat[truecolor_tile_ptr+x+2];
							xx+=3;
						}
					}
					truecolor_tile_ptr+=32;
				}
				x_tile++;
			}
			x_tile=0;
			y_tile++;
		}
	}
	//puts("Done");
	return true;
}
double max3(double a,double b,double c)
{
	if ((a > b) && (a > c))
		return a;
	if (b > c)
		return b;
	return c;
}
double min3(double a,double b,double c)
{
	if ((a < b) && (a < c))
		return a;
	if (b < c)
		return b;
	return c;
}
/**
 * Converts an RGB color value to HSL. Conversion formula
 * adapted from http://en.wikipedia.org/wiki/HSL_color_space.
 * Assumes r, g, and b are contained in the set [0, 255] and
 * returns h, s, and l in the set [0, 1].
 *
 * @param   Number  r       The red color value
 * @param   Number  g       The green color value
 * @param   Number  b       The blue color value
 * @return  Array           The HSL representation
 */
void rgbToHls(double r,double g,double b,double * hh,double * ll,double * ss)
{
    r /= 255.0;
    g /= 255.0;
    b /= 255.0;
    double max = max3(r, g, b);
    double min = min3(r, g, b);
    double h, s, l = (max + min) / 2.0;

    if(max == min){
        h = s = 0.0; // achromatic
    }else{
        double d = max - min;
        s = l > 0.5 ? d / (2.0 - max - min) : d / (max + min);
        if (max == r)
			h = (g - b) / d + (g < b ? 6 : 0);
		else if (max == g)
			h = (b - r) / d + 2.0;
		else
			h = (r - g) / d + 4.0;
        
        h /= 6.0;
    }
	*hh=h;
	*ll=l;
	*ss=s;
}
void rgbtohsv(uint8_t R,uint8_t G,uint8_t B,double * hh,double * ss,double * vv)
{
	double var_R =  R / 255.0;                     //RGB from 0 to 255
	double var_G =  G / 255.0;
	double var_B =  B / 255.0;

	double var_Min = min3( var_R, var_G, var_B );	//Min. value of RGB
	double var_Max = max3( var_R, var_G, var_B );	//Max. value of RGB
	double del_Max = var_Max - var_Min;				//Delta RGB value 
	double V = var_Max;
	double H,S;
	if ( del_Max == 0.0)                     //This is a gray, no chroma...
	{
		H = 0.0;                                //HSV results from 0 to 1
		S = 0.0;
	}
	else                                    //Chromatic data...
	{
		S = del_Max / var_Max;
		double del_R = (((var_Max - var_R ) / 6.0 ) + (del_Max / 2.0 )) / del_Max;
		double del_G = (((var_Max - var_G ) / 6.0 ) + (del_Max / 2.0 )) / del_Max;
		double del_B = (((var_Max - var_B ) / 6.0 ) + (del_Max / 2.0 )) / del_Max;
		if (var_R == var_Max)
			H = del_B - del_G;
		else if (var_G == var_Max)
			H = (1.0 / 3.0) + del_R - del_B;
		else if (var_B == var_Max)
			H = (2.0 / 3.0) + del_G - del_R;
		if (H < 0.0)
			H += 1.0;
		if (H > 1.0)
			H -= 1.0;
	}
	*hh=H;
	*ss=S;
	*vv=V;
}
void tileMap::set_pal_row(uint16_t x,uint16_t y,uint8_t row)
{
	tileMapDat[((y*mapSizeW)+x)*4]&= ~(3 << 5);
	tileMapDat[((y*mapSizeW)+x)*4]|=row<<5;
}
void tileMap::pickRow(uint8_t amount)
{
	uint8_t type=fl_choice("How would you describe this image","Varying hue","Varying brightness","Varying saturation");
	double divide=(double)amount;//convert to double
	double h,l,s;
	h=0.0;
	uint16_t z;
	uint32_t x,y;
	uint32_t cnt[4];
	double weightPal[4];
	double maxPal=divide;
	for (y=0;y<mapSizeH;++y){
		for (x=0;x<mapSizeW;++x){
			uint32_t cur_tile=get_tile(x,y);
			uint8_t * truePtr=&currentProject->tileC->truetileDat[cur_tile*256];
			double hh=0.0;
			for (z=0;z<256;z+=4) {
				rgbToHls(truePtr[0],truePtr[1],truePtr[2],&h,&l,&s);
				truePtr+=4;
				switch(type){
					case 0:
						hh+=h;
					break;
					case 1:
						hh+=l;
					break;
					case 2:
						hh+=s;
					break;
				}
			}
			hh/=64.0/divide;
			if (hh >= maxPal) {
				printf("hh >= %f %f %d\n",maxPal,hh,(int)hh);
				hh=divide-0.5;
			}
			set_pal_row(x,y,hh);
		}
	}

}
void tileMap::allRowZero(void)
{
	uint32_t x,y;
	for (y=0;y<mapSizeH;y++) {
		for (x=0;x<mapSizeW;x++)
			set_pal_row(x,y,0);
	}
}
inline uint8_t pick4Delta(double * d)
{
	if ((d[0] <= d[1]) && (d[0] <= d[2]) && (d[0] <= d[3]))
		return 0;
	if ((d[1] <= d[2]) && (d[1] <= d[3]))
		return 1;
	if (d[2] <= d[3])
		return 2;
	return 3;
}
inline uint8_t pick4Deltai(uint32_t * d)
{
	if ((d[0] <= d[1]) && (d[0] <= d[2]) && (d[0] <= d[3]))
		return 0;
	if ((d[1] <= d[2]) && (d[1] <= d[3]))
		return 1;
	if (d[2] <= d[3])
		return 2;
	return 3;
}
inline uint32_t sqri(int fun)
{
	return fun*fun;
}
inline double pickIt(double h,double l,double s,uint8_t type)
{
	switch(type){
		case 0:
			return h;
		break;
		case 1:
			return l;
		break;
		case 2:
			return s;
		break;
	}
}
void tileMap::pickRowDelta(bool showProgress,Fl_Progress *progress)
{
	uint8_t alg=fl_choice("Which method do you think works better for this image (try both)","ciede2000","Root mean squared error",0);
	if(fl_ask("Would you like the palette to be ordered by hue or light or saturation")){
		uint16_t x,y;
		uint8_t type=fl_choice("What do you want it ordered by","Hue","Light","Saturation");
		for(x=0;x<palEdit.perRow*12;x+=3){
			double h,l,s,cmp,cmp2;
			uint16_t best=x;
			rgbToHls(currentProject->rgbPal[x],currentProject->rgbPal[x+1],currentProject->rgbPal[x+2],&h,&l,&s);
			cmp=pickIt(h,l,s,type);
			for(y=x+3;y<palEdit.perRow*12;y+=3){
				rgbToHls(currentProject->rgbPal[y],currentProject->rgbPal[y+1],currentProject->rgbPal[y+2],&h,&l,&s);
				cmp2=pickIt(h,l,s,type);
				if(cmp2<=cmp)
					best=y;
			}
			swapEntry(x/3,best/3);
		}
	}
	uint8_t type_temp=palTypeGen;
	uint8_t tempSet=0;
	double d[4];
	uint32_t di[4];
	uint32_t x,y;
	uint8_t t;
	uint16_t p;
	uint8_t temp[256];
	uint32_t w,h;
	w=mapSizeW*8;
	h=mapSizeH*8;
	uint8_t * imagein=(uint8_t*)malloc(w*h*4);
	truecolor_to_image(imagein,-1);
	uint8_t **imageout=(uint8_t**)malloc(4*sizeof(void*));
	uint32_t xtile,ytile;
	xtile=ytile=0;
	if(showProgress){
		progress->maximum(12);
		progress->value(0);
	}
	for(x=0;x<4;x++){
		if(showProgress){
			sprintf((char*)temp,"Dithering %d",x);
			progress->label((char*)temp);
			Fl::check();
		}
		imageout[x]=(uint8_t*)malloc(w*h*4);
		truecolor_to_image(imageout[x],-1);
		if(showProgress){
			progress->value((x*3)+1);
			Fl::check();
		}
		ditherImage(imageout[x],w,h,true,true,true,x);
		if(showProgress){
			progress->value((x*3)+2);
			Fl::check();
		}
		ditherImage(imageout[x],w,h,true,false,true,x);
		if(showProgress){
			progress->value((x*3)+3);
			Fl::check();
		}
	}
	if(showProgress){
		progress->maximum(mapSizeH);
		progress->label("Picking tiles based on delta");
	}
	for (uint32_t a=0;a<(h*w*4)-w*4;a+=w*4*8){//a tiles y
		for (uint32_t b=0;b<w*4;b+=32){//b tiles x
			uint32_t cur_tile=get_tile(xtile,ytile);
			if(alg)
				memset(di,0,4*sizeof(uint32_t));
			else{
				for (t=0;t<4;t++)
					d[t]=0.0;
			}
			if ((type_temp != 0) && (game_system == sega_genesis)){
				tempSet=(currentProject->tileMapC->get_prio(xtile,ytile)^1)*8;
				set_palette_type(tempSet);
			}
			for (t=0;t<4;t++){
				for (uint32_t y=0;y<w*4*8;y+=w*4){//pixels y
					if(alg){
						for(x=0;x<32;x+=4)
							di[t]+=sqri(imagein[a+b+y+x]-imageout[t][a+b+y+x])+sqri(imagein[a+b+y+x+1]-imageout[t][a+b+y+x+1])+sqri(imagein[a+b+y+x+2]-imageout[t][a+b+y+x+2]);
					}else{
						for(x=0;x<32;x+=4)
							d[t]+=ciede2000rgb(imagein[a+b+y+x],imagein[a+b+y+x+1],imagein[a+b+y+x+2],imageout[t][a+b+y+x],imageout[t][a+b+y+x+1],imageout[t][a+b+y+x+2]);
					}
				}
				//di[t]=sqrt(di[t]);//not needed only relative error matters in this case
			}
			
			uint16_t truecolor_tile_ptr=0;
			
			uint8_t sillyrow;
			if(alg)
				sillyrow=pick4Deltai(di);
			else
				sillyrow=pick4Delta(d);
			set_pal_row(xtile,ytile,sillyrow);
			for (uint32_t y=0;y<w*4*8;y+=w*4){//pixels y
				memcpy(&temp[truecolor_tile_ptr],&imageout[sillyrow][a+b+y],32);
				truecolor_tile_ptr+=32;
			}
			currentProject->tileC->truecolor_to_tile_ptr(sillyrow,cur_tile,temp,false);
			xtile++;
		}
		if(showProgress){
			progress->value(ytile);
			window->redraw();
			Fl::check();
		}
		xtile=0;
		ytile++;
	}
	free(imagein);
	free(imageout[0]);
	free(imageout[1]);
	free(imageout[2]);
	free(imageout[3]);
	free(imageout);
	if (game_system == sega_genesis)
		set_palette_type(type_temp);
}
inline uint8_t Clamp255(int n)
{
    n = n>255 ? 255 : n;
    return n<0 ? 0 : n;
}
void reduceImage(uint8_t * image,uint8_t * found_colors,int8_t row,uint8_t offsetPal,Fl_Progress *progress,uint8_t maxCol,uint8_t yuv,uint8_t alg)
{
	progress->maximum(1.0);
	uint8_t off2=offsetPal*2;
	uint8_t off3=offsetPal*3;
	uint32_t colors_found;
	uint32_t w,h;
	uint8_t maxPal;
	switch(game_system){
		case sega_genesis:
			maxPal=64;
		break;
		case NES:
			maxPal=16;
		break;
	}
	w=currentProject->tileMapC->mapSizeW*8;
	h=currentProject->tileMapC->mapSizeH*8;
	truecolor_to_image(image,row,false);
	progress->label("Dithering to colorspace");
	Fl::check();
	if(!yuv)
		ditherImage(image,w,h,false,true);
	progress->label("Quantizing image");
	Fl::check();
	colors_found=count_colors(image,w,h,&found_colors[0],false);
	printf("Unique colors %d\n",colors_found);
	if (colors_found <= maxCol){
		printf("%d colors\n",colors_found);
		for (uint8_t x=0;x<colors_found;x++){
			uint8_t r,g,b;
againFun:
			if (currentProject->palType[x+offsetPal]){
				offsetPal++;
				off3+=3;
				off2+=2;
				if (offsetPal >= maxPal)
					break;
				goto againFun;
			}
			r=found_colors[(x*3)];
			g=found_colors[(x*3)+1];
			b=found_colors[(x*3)+2];
			switch(game_system){
				case sega_genesis:
					printf("R=%d G=%d B=%d\n",r,g,b);
					r=nearest_color_index(r);
					g=nearest_color_index(g);
					b=nearest_color_index(b);
					currentProject->rgbPal[(x*3)+off3]=palTab[r];
					currentProject->rgbPal[(x*3)+1+off3]=palTab[g];
					currentProject->rgbPal[(x*3)+2+off3]=palTab[b];
					//bgr
					r-=palTypeGen;
					g-=palTypeGen;
					b-=palTypeGen;
					currentProject->palDat[(x*2)+off2]=b<<1;
					currentProject->palDat[(x*2)+1+off2]=(r<<1)|(g<<5);
				break;
				case NES:
					printf("R=%d G=%d B=%d\n",r,g,b);
					{uint8_t temp = to_nes_color_rgb(r,g,b);
					currentProject->palDat[x+offsetPal]=temp;}
				break;
				default:
					show_default_error
				break;
			}
		}
		if(game_system==NES)
			update_emphesis(0,0);
		window->redraw();
	}else{
		printf("More than %d colors reducing to %d colors\n",maxCol,maxCol);
		uint8_t user_pal[3][256];			
		uint8_t rgb_pal2[768];
		uint8_t rgb_pal3[768];
		uint16_t colorz=maxCol;
		bool can_go_again=true;
		uint8_t*imageuse;
		uint8_t*output;
		if(alg==1)
			output=(uint8_t*)malloc(w*h*3);
		if(yuv){
			imageuse=(uint8_t*)malloc(w*h*3);
			uint32_t x,y;
			uint8_t*imageptr=image;
			uint8_t*outptr=imageuse;
			for(y=0;y<h;y++){
				for(x=0;x<w;x++){//conversion formula from http://en.wikipedia.org/wiki/YCbCr
					outptr[0]=Clamp255(16+((65.738*(double)imageptr[0]+129.057*(double)imageptr[1]+25.064*(double)imageptr[2])/256.0));
					outptr[1]=Clamp255(128-((-37.945*(double)imageptr[0]-74.494*(double)imageptr[1]+112.439*(double)imageptr[2])/256.0));
					outptr[2]=Clamp255(128+((112.439*(double)imageptr[0]-94.154*(double)imageptr[1]-18.285*(double)imageptr[2])/256.0));
					imageptr+=3;
					outptr+=3;
				}
			}
		}
		else
			imageuse=image;
try_again_color:
		if(alg==1)
			scolorq_wrapper(imageuse,output,user_pal,w,h,colorz);
		else
			dl3quant(imageuse,w,h,colorz,user_pal,true,progress,yuv);/*this uses denesis lee's v3 color quant which is fonund at http://www.gnu-darwin.org/www001/ports-1.5a-CURRENT/graphics/mtpaint/work/mtpaint-3.11/src/quantizer.c*/
		for (uint16_t x=0;x<colorz;x++){
			uint8_t r,g,b;
			r=user_pal[0][x];
			g=user_pal[1][x];
			b=user_pal[2][x];
			if(yuv){
				r=((298.082*(double)r+408.583*(double)b)/256.0)-222.921;
				g=((298.082*(double)r-100.291*(double)g-208.120*(double)b)/256.0)+135.576;
				b=((298.082*(double)r-516.412*(double)g)/256.0)-276.836;
			}
			switch(game_system){
				case sega_genesis:
					r=nearest_color_index(r);
					g=nearest_color_index(g);
					b=nearest_color_index(b);
					rgb_pal2[(x*3)]=palTab[r];
					rgb_pal2[(x*3)+1]=palTab[g];
					rgb_pal2[(x*3)+2]=palTab[b];
				break;
				case NES:
					uint8_t temp=to_nes_color_rgb(r,g,b);
					uint32_t temp_rgb = MakeRGBcolor(temp);
					rgb_pal2[(x*3)]=(temp_rgb>>16)&255;
					rgb_pal2[(x*3)+1]=(temp_rgb>>8)&255;
					rgb_pal2[(x*3)+2]=temp_rgb&255;
				break;
			}
		}
		uint8_t new_colors = count_colors(rgb_pal2,colorz,1,&rgb_pal3[off3]);
		printf("Unique colors in palette %d\n",new_colors);
			if (new_colors < maxCol){
				if (can_go_again == true){
					if (colorz != 512)
						colorz++;
					else
						can_go_again=false;
					printf("Trying again at %d needs more color\n",colorz);
					goto try_again_color;
				}
			}
		if (new_colors > maxCol){
			can_go_again=false;
			puts("Woops too many colors");
			colorz--;
			goto try_again_color;
		}
		uint8_t off3o=off3;
		for (uint8_t x=0;x<maxCol;x++){
			uint8_t r,g,b;
againNerd:
			if (currentProject->palType[x+offsetPal]){
				offsetPal++;
				off3+=3;
				off2+=2;
				if (offsetPal >= maxPal)
					break;
				//printf("%d %d %d\n",offsetPal,off2,off3);
				goto againNerd;
			}
			memcpy(currentProject->rgbPal+off3+(x*3),&rgb_pal3[off3o+(x*3)],3);
			switch(game_system){
				case sega_genesis:
					r=currentProject->rgbPal[(x*3)+off3];
					g=currentProject->rgbPal[(x*3)+1+off3];
					b=currentProject->rgbPal[(x*3)+2+off3];
					r=nearest_color_index(r)-palTypeGen;
					g=nearest_color_index(g)-palTypeGen;
					b=nearest_color_index(b)-palTypeGen;
					currentProject->palDat[(x*2)+off2]=b<<1;
					currentProject->palDat[(x*2)+1+off2]=(r<<1)|(g<<5);
				break;
				case NES:
					currentProject->palDat[x+offsetPal]=to_nes_color(x+offsetPal);
				break;
			}
		}
		if(game_system==NES)
			update_emphesis(0,0);
		if(alg==1){
			truecolorimageToTiles(output,row,false);
			free(output);
		}
		if(yuv)
			free(imageuse);
	}
}
void generate_optimal_palette(Fl_Widget*,void*)
{
	uint8_t perRow[4];
	char temp[4];
	uint8_t rowSize;
	uint8_t rows;
	switch (game_system){
		case sega_genesis:
			strcpy(temp,"64");
			rowSize=16;
		break;
		case NES:
			strcpy(temp,"16");
			rowSize=4;
		break;
	}
	char * returned=(char *)fl_input("How many colors would you like?",temp);
	if (returned==0)
		return;
	if (verify_str_number_only(returned) == false)
			return;
	int8_t colors=atoi(returned);
	int8_t colorstotal=colors;
	uint8_t asdf;
	for (asdf=0;asdf<4;asdf++) {
		perRow[asdf]=colors > rowSize ? rowSize:colors;
		colors-=rowSize;
		printf("Colors %d\n",colors);
		if (colors <= 0)
			break;
	}
	rows=asdf+1;
	printf("Using %d rows\n",rows);
	Fl_Window *win;
	Fl_Progress *progress;
	win = new Fl_Window(250,45,"Progress");           // access parent window
	win->begin();                                // add progress bar to it..
	progress = new Fl_Progress(25,7,200,30);
	progress->minimum(0.0);                      // set progress range to be 0.0 ~ 1.0
	progress->maximum(1.0);
	progress->color(0x88888800);               // background color
	progress->selection_color(0x4444ff00);     // progress bar color
	progress->labelcolor(FL_WHITE);            // percent text color
	win->end();                                  // end adding to window
	win->show();
	/*
	This function is one of the more importan features of the program
	This will look at the tile map and based on that find an optimal palette
	*/
	uint8_t * image;
	//uint8_t * colors;
	uint32_t w,h;
	w=currentProject->tileMapC->mapSizeW*8;
	h=currentProject->tileMapC->mapSizeH*8;
	uint32_t colors_found;
	//uint8_t * found_colors;
	uint8_t found_colors[768];
	uint8_t rowAuto;
	if (rows==1)
		rowAuto = fl_ask("Would you like all tiles on the tilemap to be set to row 0? (This is where all generated colors will apear)");
	else
		 rowAuto = fl_choice("How would you like the palette map to be handled","Don't change anythin","Pick based on hue","Generate contiguous palette then pick based on delta");
	uint8_t fun_palette;
	uint8_t alg=fl_choice("What color reduction algorithm would you like used","Densise Lee v3","scolorq",0);
	uint8_t yuv;
	yuv=fl_ask("You you like the image to be calculated in YCbCr color space\nHint: No is the better option");
	switch (game_system){
		case sega_genesis:
			fun_palette=16;
		break;
		case NES:
			fun_palette=4;
		break;
		default:
			show_default_error
		break;
	}
	image = (uint8_t *)malloc(w*h*3);
	if (rows==1){
		if (rowAuto)
			currentProject->tileMapC->allRowZero();
		reduceImage(image,found_colors,-1,0,progress,perRow[0],yuv,alg);
		window->damage(FL_DAMAGE_USER1);
		Fl::check();
	}else{
		if(rowAuto==2){
			reduceImage(image,found_colors,-1,0,progress,colorstotal,yuv,alg);
			currentProject->tileMapC->pickRowDelta(true,progress);
			window->damage(FL_DAMAGE_USER1);
			Fl::check();
		}else{
			if (rowAuto)
				currentProject->tileMapC->pickRow(rows);
			for (uint8_t nerdL=0;nerdL<rows;nerdL++){
				reduceImage(image,found_colors,nerdL,nerdL*fun_palette,progress,perRow[nerdL],yuv,alg);
				window->damage(FL_DAMAGE_USER1);
				Fl::check();
			}
		}		
	}
	free(image);
	win->remove(progress);// remove progress bar from window
	delete(progress);// deallocate it
	//w->draw();
	delete win;
	palEdit.updateSlider();
	tileEdit_pal.updateSlider();
	tileMap_pal.updateSlider();
	window->redraw();
	Fl::check();
}
void truecolorimageToTiles(uint8_t * image,int8_t rowusage,bool useAlpha)
{
	uint8_t type_temp=palTypeGen;
	uint8_t tempSet=0;
	uint8_t truecolor_tile[256];
	uint32_t x_tile=0;
	uint32_t y_tile=0;
	uint8_t pSize=useAlpha ? 4:3;
	uint8_t pTile=useAlpha ? 32:24;
	uint32_t w=currentProject->tileMapC->mapSizeW*8;
	uint32_t h=currentProject->tileMapC->mapSizeH*8;
	uint16_t truecolor_tile_ptr;
	for (uint32_t a=0;a<(h*w*pSize)-w*pSize;a+=w*pSize*8){//a tiles y
		for (uint32_t b=0;b<w*pSize;b+=pTile){//b tiles x
			uint8_t temp;
			int32_t current_tile;
			if(rowusage==-1){
				current_tile=currentProject->tileMapC->get_tile(x_tile,y_tile);
			}else{
				current_tile=currentProject->tileMapC->get_tileRow(x_tile,y_tile,rowusage);
				if (current_tile == -1)
					goto dont_convert_tile;
			}
			truecolor_tile_ptr=0;
			for (uint32_t y=0;y<w*pSize*8;y+=w*pSize)//pixels y
			{
				if(useAlpha){
					memcpy(&truecolor_tile[truecolor_tile_ptr],&image[a+b+y],32);
					truecolor_tile_ptr+=32;
				}else{
					for(uint8_t xx=0;xx<24;xx+=3){
						memcpy(&truecolor_tile[truecolor_tile_ptr],&image[a+b+y+xx],3);
						truecolor_tile_ptr+=3;
						truecolor_tile[truecolor_tile_ptr]=255;
						++truecolor_tile_ptr;
					}
				}
			}
			//convert back to tile
			uint8_t * TileTempPtr;
			if ((type_temp != 0) && (game_system == sega_genesis)){
				tempSet=(currentProject->tileMapC->get_prio(x_tile,y_tile)^1)*8;
				set_palette_type(tempSet);
			}
			currentProject->tileC->truecolor_to_tile_ptr(currentProject->tileMapC->get_palette_map(x_tile,y_tile),current_tile,truecolor_tile,false);
dont_convert_tile:
		x_tile++;	
		}
	x_tile=0;
	y_tile++;
	}
	if (game_system == sega_genesis)
		set_palette_type(type_temp);
}
