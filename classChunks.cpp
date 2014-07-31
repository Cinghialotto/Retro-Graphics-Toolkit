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
   Copyright Sega16 (or whatever you wish to call me) (2012-2014)
*/
#include "global.h"
#include "compressionWrapper.h"
#include "callback_chunk.h"
#include "filemisc.h"
ChunkClass::ChunkClass(void){
	chunks.resize(256);
	amt=1;
	wi=hi=16;//16*16=256
	useBlocks=false;
}
ChunkClass::ChunkClass(const ChunkClass& other){
	wi=other.wi;
	hi=other.hi;
	amt=other.amt;
	useBlocks=other.useBlocks;
	chunks=other.chunks;
}
ChunkClass::~ChunkClass(void){
	chunks.clear();
}
void ChunkClass::setElm(uint32_t id,uint32_t x,uint32_t y,struct ChunkAttrs c){
	struct ChunkAttrs*ch=chunks.data()+(id*wi*hi)+(y*wi)+x;
	ch->flags=c.flags;
	ch->block=c.block;
}
struct ChunkAttrs ChunkClass::getElm(uint32_t id,uint32_t x,uint32_t y){
	return chunks[(id*wi*hi)+(y*wi)+x];
}
void ChunkClass::removeAt(uint32_t at){
	chunks.erase(chunks.begin()+(at*wi*hi),chunks.begin()+((at+1)*wi*hi));
	--amt;
}
void ChunkClass::resizeAmt(uint32_t amtnew){
	chunks.resize(amtnew*wi*hi);
	amt=amtnew;
}
void ChunkClass::resizeAmt(void){
	resizeAmt(amt);
}
bool ChunkClass::getPrio_t(uint32_t id,uint32_t x,uint32_t y){//The _t means based on tiles not blocks
	if(useBlocks){
		return currentProject->tileMapC->get_prio(
		x%currentProject->tileMapC->mapSizeW,
		(chunks[(id*wi*hi)+(y*wi/currentProject->tileMapC->mapSizeH)+(x/currentProject->tileMapC->mapSizeW)].block
		*currentProject->tileMapC->mapSizeH)
		+(y%currentProject->tileMapC->mapSizeH));
	}else
		return ((chunks[(id*wi*hi)+(y*wi)+x].flags>>2)&1)?true:false;
}
uint8_t ChunkClass::getTileRow_t(uint32_t id,uint32_t x,uint32_t y){
	if(useBlocks){
		return currentProject->tileMapC->get_palette_map(
		x%currentProject->tileMapC->mapSizeW,
		(chunks[(id*wi*hi)+(y*wi/currentProject->tileMapC->mapSizeH)+(x/currentProject->tileMapC->mapSizeW)].block
		*currentProject->tileMapC->mapSizeH)
		+(y%currentProject->tileMapC->mapSizeH));
	}else
		return (chunks[(id*wi*hi)+(y*wi)+x].flags>>3)&3;
}
unsigned ChunkClass::getSolid(uint32_t id,uint32_t x,uint32_t y){
	unsigned shift;
	if(useBlocks)
		shift=2;
	else
		shift=5;
	return (chunks[getOff(id,x,y)].flags>>shift)&3;
}
uint32_t ChunkClass::getBlock(uint32_t id,uint32_t x,uint32_t y){
	return chunks[getOff(id,x,y)].block;
}
bool ChunkClass::getHflip(uint32_t id,uint32_t x,uint32_t y){
	return chunks[getOff(id,x,y)].flags&1;
}
bool ChunkClass::getVflip(uint32_t id,uint32_t x,uint32_t y){
	return (chunks[getOff(id,x,y)].flags&2)>>1;
}
unsigned ChunkClass::getOff(uint32_t id,uint32_t x,uint32_t y){
	return (id*wi*hi)+(y*wi)+x;
}
bool ChunkClass::getPrio(uint32_t id,uint32_t x,uint32_t y){
	return (chunks[getOff(id,x,y)].flags&4)>>2;
}
void ChunkClass::setBlock(uint32_t id,uint32_t x,uint32_t y,uint32_t block){
	chunks[getOff(id,x,y)].block=block;
	/*This contains which block/tile to use*/
}
void ChunkClass::setFlag(uint32_t id,uint32_t x,uint32_t y,uint32_t flag){
	chunks[getOff(id,x,y)].flags=flag;
	/*!If not using blocks flags will contain the following
	bit 0 hflip
	bit 1 vflip
	bit 2 prioity
	bit 3,4 palette row
	All other bits are unsued and can be used for video game usage
	If using blocks flags will simply contain video game settings
	Here are video game settings used. If using tiles instead of blocks add 3 to bit count and ignore x and y flip
	bit 0 x-flip
	bit 1 y-flip
	bit 2,3 solidity 00 means not solid, 01 means top solid, 10 means left/right/bottom solid, and 11 means all solid.
	*/
}
void ChunkClass::setSolid(uint32_t id,uint32_t x,uint32_t y,unsigned solid){
	unsigned shift;
	if(useBlocks)
		shift=2;
	else
		shift=5;
	unsigned off=getOff(id,x,y);
	chunks[off].flags&=~(3<<shift);
	chunks[off].flags|=solid<<shift;
}
void ChunkClass::setHflip(uint32_t id,uint32_t x,uint32_t y,bool hflip){
	unsigned off=getOff(id,x,y);
	if(hflip)
		chunks[off].flags|=1;
	else
		chunks[off].flags&=~1;
}
void ChunkClass::setVflip(uint32_t id,uint32_t x,uint32_t y,bool vflip){
	unsigned off=getOff(id,x,y);
	if(vflip)
		chunks[off].flags|=2;
	else
		chunks[off].flags&=~2;
}
void ChunkClass::setPrio(uint32_t id,uint32_t x,uint32_t y,bool prio){
	unsigned off=getOff(id,x,y);
	if(prio)
		chunks[off].flags|=4;
	else
		chunks[off].flags&=~4;
}
void ChunkClass::drawChunk(uint32_t id,int xo,int yo,int zoom,int scrollX,int scrollY){
	struct ChunkAttrs * cptr=chunks.data();
	for(uint32_t y=scrollY;y<hi;++y){
		cptr=&chunks[(id*wi*hi)+(y*wi)+scrollX];
		int xoo=xo;
		for(uint32_t x=scrollX;x<wi;++x){
			if(useBlocks){
				uint32_t Ty=cptr->block*currentProject->tileMapC->mapSizeH;
				int yoo=yo;
				if(cptr->flags&2)
					yoo+=(currentProject->tileMapC->mapSizeH-1)*8*zoom;
				int xooo;
				for(uint32_t yb=0;yb<currentProject->tileMapC->mapSizeH;++yb){
					xooo=xoo;
					if(cptr->flags&1)
						xooo+=(currentProject->tileMapC->mapSizeW-1)*8*zoom;
					for(uint32_t xb=0;xb<currentProject->tileMapC->mapSizeW;++xb){
						bool hflip=currentProject->tileMapC->get_hflip(xb,Ty),vflip=currentProject->tileMapC->get_vflip(xb,Ty);
						unsigned row=currentProject->tileMapC->get_palette_map(xb,Ty);
						uint32_t tile=currentProject->tileMapC->get_tile(xb,Ty);
						if((cptr->flags&3)==3){//Both
							if(showTrueColor)
								currentProject->tileC->draw_truecolor(tile,xooo,yoo,hflip^true,vflip^true,zoom);
							else
								currentProject->tileC->draw_tile(xooo,yoo,tile,zoom,row,hflip^true,vflip^true);
						}else if(cptr->flags&2){//Y-flip
							if(showTrueColor)
								currentProject->tileC->draw_truecolor(tile,xooo,yoo,hflip,vflip^true,zoom);
							else
								currentProject->tileC->draw_tile(xooo,yoo,tile,zoom,row,hflip,vflip^true);
						}else if(cptr->flags&1){//X-flip
							if(showTrueColor)
								currentProject->tileC->draw_truecolor(tile,xooo,yoo,hflip^true,vflip,zoom);
							else
								currentProject->tileC->draw_tile(xooo,yoo,tile,zoom,row,hflip^true,vflip);
						}else{//No flip
							if(showTrueColor)
								currentProject->tileC->draw_truecolor(tile,xooo,yoo,hflip,vflip,zoom);
							else
								currentProject->tileC->draw_tile(xooo,yoo,tile,zoom,row,hflip,vflip);
						}
						if(cptr->flags&1)
							xooo-=8*zoom;
						else
							xooo+=8*zoom;
					}
					if(cptr->flags&2)
						yoo-=8*zoom;
					else
						yoo+=8*zoom;
					++Ty;
				}
				if(cptr->flags&2)
					yoo+=currentProject->tileMapC->mapSizeW*8*zoom;
				xoo+=currentProject->tileMapC->mapSizeW*8*zoom;

			}else{
				currentProject->tileC->draw_tile(xoo,yo,cptr->block,zoom,(cptr->flags>>3)&3,cptr->flags&1,(cptr->flags>>1)&1);
				xoo+=8*zoom;
			}
			cptr++;
			if((xoo)>(window->w()))
				break;
		}
		if(useBlocks)
			yo+=8*zoom*currentProject->tileMapC->mapSizeH;
		else
			yo+=8*zoom;
		if(yo>(window->h()))
			break;
	}
}
void ChunkClass::scrollChunks(void){
	unsigned oldS=window->chunkX->value();
	int zoom=window->chunk_tile_size->value();
	int off;
	if(useBlocks)
		off=(wi*currentProject->tileMapC->mapSizeW)-((window->w()-ChunkOff[0])/(zoom*8));
	else
		off=wi-((window->w()-ChunkOff[0])/(zoom*8));
	if(oldS>off)
		scrollChunks_G[0]=oldS=off;
	if(off>0){
		window->chunkX->show();
		window->chunkX->value(oldS,1,0,off+2);
	}else
		window->chunkX->hide();
	oldS=window->chunkY->value();
	if(useBlocks)
		off=(hi*currentProject->tileMapC->mapSizeH)-((window->h()-ChunkOff[1])/(zoom*8));
	else
		off=hi-((window->h()-ChunkOff[1])/(zoom*8));
	if(oldS>off)
		scrollChunks_G[1]=oldS=off;
	if(off>0){
		window->chunkY->show();
		window->chunkY->value(oldS,1,0,off+2);
	}else
		window->chunkY->hide();
}
#if _WIN32
static inline uint16_t swap_word(uint16_t w){
	uint8_t a,b;
	a=w&255;
	b=w>>8;
	return (a<<8)|b;
}
#endif
static void errorNum(void){
	fl_alert("Please enter a value greater than zero");
}
void ChunkClass::importSonic1(const char * filename,bool append){
	if(fl_ask("Custome width and height?")){
		char*ptr=(char*)fl_input("Width");
		if(!ptr)
			return;
		if(!verify_str_number_only(ptr))
			return;
		int witmp=atoi(ptr);
		if(witmp<=0){
			errorNum();
			return;
		}
		ptr=(char*)fl_input("Height");
		if(!ptr)
			return;
		if(!verify_str_number_only(ptr))
			return;
		int hitmp=atoi(ptr);
		if(hitmp<=0){
			errorNum();
			return;
		}
		if(append)
			resize(witmp,hitmp);
		wi=witmp;
		hi=hitmp;
	}else
		wi=hi=16;
	int compression=compressionAsk();
	if(compression<0)
		return;
	uint16_t* Dat;
	size_t fileSize;
	if(compression)
		Dat=(uint16_t*)decodeType(filename,fileSize,compression);
	else{
		FILE * fi=fopen(filename,"rb");
		fseek(fi,0,SEEK_END);
		fileSize=ftell(fi);
		rewind(fi);
		Dat=(uint16_t*)malloc(fileSize);
		fread(Dat,1,fileSize,fi);
		fclose(fi);
	}
	uint32_t off;
	if(append)
		off=amt;
	else
		off=0;
	window->updateChunkSize(wi,hi);
	amt=(fileSize/512)+off;
	chunks.resize(amt*wi*hi);
	struct ChunkAttrs*cptr=chunks.data();
	cptr+=off*wi*hi;
	uint16_t * DatC=Dat;
	for(uint32_t l=0;l<(fileSize/512);++l){
		for(uint32_t y=0;y<16;++y){
			for(uint32_t x=0;x<16;++x){
				#if _WIN32
				*DatC=swap_word(*DatC);
				#else
				*DatC=be16toh(*DatC);
				#endif
				cptr->block=*DatC&1023;
				cptr->flags=(*DatC>>11)&15;
				++cptr;
				++DatC;
			}
		}
	}
	free(Dat);
}
void ChunkClass::exportSonic1(void){
	FILE*fp;
	int type,compression;
	type=askSaveType();
	int clipboard;
	size_t fileSize;
	if(type){
		clipboard=clipboardAsk();
		if(clipboard==2)
			return;
	}else
		clipboard=0;
	bool pickedFile;
	if(clipboard)
		pickedFile=true;
	else
		pickedFile=load_file_generic("Save tilemap to",true);
	if(pickedFile){
		compression=compressionAsk();
		if(compression<0)
			return;
		if(clipboard)
			fp=0;
		else if(type)
			fp = fopen(the_file.c_str(),"w");
		else
			fp = fopen(the_file.c_str(),"wb");
		if (likely(fp||clipboard)){
			struct ChunkAttrs*cptr=chunks.data();
			uint16_t*tmp=(uint16_t*)malloc(wi*hi*2*amt);
			uint16_t*ptmp=tmp;
			fileSize=wi*hi*amt*2;
			for(uint32_t i=0;i<wi*hi*amt;++i){
					uint32_t temp=cptr->block;
					if(temp>1023){
						printf("Block overflow %d\n",temp);
						temp=1023;
					}
					temp|=(cptr->flags&15)<<11;
					#if _WIN32
						*ptmp++=swap_word(temp);//mingw appears not to provide htobe16 function
					#else
						*ptmp++=htobe16(temp);//needs to be big endian
					#endif
					++cptr;
			}
			if(compression){
				void*tmpold=tmp;
				tmp=(uint16_t*)encodeType(tmp,fileSize,fileSize,compression);
				free(tmpold);
			}
			if(type){
				char temp[2048];
				snprintf(temp,2048,"Width: %d Height: %d Amount: %d %s",wi,hi,amt,typeToText(compression));
				if(!saveBinAsText(tmp,fileSize,fp,type,temp,"mapDat",8)){
					free(tmp);
					return;
				}
			}else
				fwrite(tmp,1,fileSize,fp);
			free(tmp);
			if(fp)
				fclose(fp);
		}
	}
}
void ChunkClass::resize(uint32_t wnew,uint32_t hnew){
	if((wnew==wi)&&(hnew==hi))
		return;
	struct ChunkAttrs*tmp=(struct ChunkAttrs*)malloc(sizeof(struct ChunkAttrs)*wi*hi*amt);
	memcpy(tmp,chunks.data(),sizeof(struct ChunkAttrs)*wi*hi*amt);
	chunks.resize(amt*wnew*hnew);
	struct ChunkAttrs*cptr=chunks.data(),*tptr=tmp;
	for(uint32_t z=0;z<amt;++z){
		for(uint32_t y=0;y<std::min(hi,hnew);++y){
			if(wnew>wi){
				memcpy(cptr,tptr,wi*sizeof(struct ChunkAttrs));
				memset(cptr+wi,0,(wnew-wi)*sizeof(struct ChunkAttrs));
			}else
				memcpy(cptr,tptr,wnew*sizeof(struct ChunkAttrs));
			cptr+=wnew;
			tptr+=wi;
		}
		if(hnew>hi){
			for(uint32_t y=hi;y<hnew;++y){
				memset(cptr,0,wnew*sizeof(struct ChunkAttrs));
				cptr+=wnew;
			}
		}else
			tptr+=(hi-hnew)*wi;
	}
	wi=wnew;
	hi=hnew;
	free(tmp);
	scrollChunks();
}
