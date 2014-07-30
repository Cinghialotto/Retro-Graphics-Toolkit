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
#include <stdlib.h>
#include "system.h"
#include "project.h"
#include "undo.h"
#include "color_convert.h"
#include <FL/Fl_Browser.H>
static struct undoEvent*undoBuf;
static uint_fast32_t amount;
static uint_fast32_t memUsed;
static uint_fast32_t maxMen=16*1024*1024;//Limit undo buffer to 16Mb this is better than limiting by depth as each item varies in size
static int_fast32_t pos=-1;
void showMemUsageUndo(Fl_Widget*,void*){
	fl_alert("The undo stack currently uses %d bytes of ram not including any overhead\nAmount of items %d",memUsed,amount);
}
static void resizeArray(uint32_t amt){
	if(undoBuf){
		if(amt)
			undoBuf=(struct undoEvent*)realloc(undoBuf,amt*sizeof(struct undoEvent));
		else{
			free(undoBuf);
			undoBuf=0;
		}
	}else{
		if(amt)
			undoBuf=(struct undoEvent*)malloc(amt*sizeof(struct undoEvent));
	}
}
static unsigned getSzTile(tileTypeMask_t type){
	unsigned sz=0;
	if(type&tTypeTile)
		sz+=currentProject->tileC->tileSize;
	if(type&tTypeTruecolor)
		sz+=currentProject->tileC->tcSize;
	return sz;
}
static void cleanupEvent(uint32_t id){
	struct undoEvent*uptr=undoBuf+id;
	switch(uptr->type){
		case uTile:
			{struct undoTile*ut=(struct undoTile*)uptr->ptr;
			unsigned sz=getSzTile(ut->type);
			free(ut->ptr);
			memUsed-=sz;
			if(ut->ptrnew){
				free(ut->ptrnew);
				memUsed-=sz;
			}
			free(uptr->ptr);
			memUsed-=sizeof(struct undoTile);}
		break;
		case uTilePixel:
			free(uptr->ptr);
			memUsed-=sizeof(struct undoTilePixel);
		break;
		case uTileAll:
			{struct undoTileAll*ut=(struct undoTileAll*)uptr->ptr;
			unsigned sz=getSzTile(ut->type)*currentProject->tileC->amt;
			free(ut->ptr);
			memUsed-=sz;
			if(ut->ptrnew){
				free(ut->ptrnew);
				memUsed-=sz;
			}
			free(uptr->ptr);
			memUsed-=sizeof(struct undoTileAll);}
		break;
		case uTileAppend:
			//Nothing to do here
		break;
		case uTilemapEdit:
			free(uptr->ptr);
			memUsed-=sizeof(struct undoTilemapEdit);
		break;
		case uTilemap:
		case uTilemapattr:
			{struct undoTilemap*um=(struct undoTilemap*)uptr->ptr;
			uint_fast32_t sz=um->w*um->h;
			if(uptr->type==uTilemap)
				sz*=4;
			free(um->ptr);
			memUsed-=sz;
			if(um->ptrnew){
				sz=um->wnew*um->hnew;
				if(uptr->type==uTilemap)
					sz*=4;
				free(um->ptrnew);
				memUsed-=sz;
			}}
		break;
		case uPalette:
			{struct undoPalette*up=(struct undoPalette*)uptr->ptr;
			unsigned sz;
			switch(currentProject->gameSystem){
				case sega_genesis:
					sz=128;
				break;
				case NES:
					sz=16;
				break;
			}
			free(up->ptr);
			memUsed-=sz;
			if(up->ptrnew){
				free(up->ptrnew);
				memUsed-=sz;
			}
			free(uptr->ptr);
			memUsed-=sizeof(struct undoPalette);}
		break;
		case uPaletteEntry:
			free(uptr->ptr);
			memUsed-=sizeof(struct undoPaletteEntry);
		break;
	}
}
static void pushEventPrepare(void){
	++pos;
	if((pos<=amount)&&amount){
		for(uint_fast32_t i=pos;i<amount;++i){
			cleanupEvent(i);
			memUsed-=sizeof(struct undoEvent);
		}
	}
	amount=pos;
	resizeArray(++amount);
	memUsed+=sizeof(struct undoEvent);
}
static void tilesTo(uint8_t*ptr,uint32_t id,tileTypeMask_t type){
	if(type&tTypeTile){
		memcpy(ptr,currentProject->tileC->tDat.data()+(id*currentProject->tileC->tileSize),currentProject->tileC->tileSize);
		ptr+=currentProject->tileC->tileSize;
	}
	if(type&tTypeTruecolor)
		memcpy(ptr,currentProject->tileC->truetDat.data()+(id*currentProject->tileC->tcSize),currentProject->tileC->tcSize);
}
static void tilesToU(uint8_t*ptr,uint32_t id,tileTypeMask_t type){
	if(type&tTypeTile){
		memcpy(currentProject->tileC->tDat.data()+(id*currentProject->tileC->tileSize),ptr,currentProject->tileC->tileSize);
		ptr+=currentProject->tileC->tileSize;
	}
	if(type&tTypeTruecolor)
		memcpy(currentProject->tileC->truetDat.data()+(id*currentProject->tileC->tcSize),ptr,currentProject->tileC->tcSize);
}
static void cpyAllTiles(uint8_t*ptr,unsigned amt,tileTypeMask_t type){
	if(type&tTypeTile){
		memcpy(ptr,currentProject->tileC->tDat.data(),amt*currentProject->tileC->tileSize);
		ptr+=amt*currentProject->tileC->tileSize;
	}
	if(type&tTypeTruecolor)
		memcpy(ptr,currentProject->tileC->truetDat.data(),amt*currentProject->tileC->tcSize);
}
static void cpyAllTilesU(uint8_t*ptr,unsigned amt,tileTypeMask_t type){
	if(amt!=currentProject->tileC->amt){
		currentProject->tileC->resizeAmt(amt);
		updateTileSelectAmt();
	}
	if(type&tTypeTile){
		memcpy(currentProject->tileC->tDat.data(),ptr,amt*currentProject->tileC->tileSize);
		ptr+=amt*currentProject->tileC->tileSize;
	}
	if(type&tTypeTruecolor)
		memcpy(currentProject->tileC->truetDat.data(),ptr,amt*currentProject->tileC->tcSize);
}
static void attrCpy(uint8_t*dst,uint8_t*src,uint_fast32_t n){
	while(n--){
		*dst++=*src;
		src+=4;
	}
}
static void attrCpyU(uint8_t*dst,uint8_t*src,uint_fast32_t n){
	while(n--){
		*dst=*src++;
		dst+=4;
	}
}
void UndoRedo(bool redo){
	if((pos<0)&&(!redo))
		return;
	if(!amount)
		return;
	if(redo&&(pos>=(int_fast32_t(amount)-1)))
		return;
	if(redo&&(pos<=int_fast32_t(amount)))
		++pos;
	struct undoEvent*uptr=undoBuf+pos;
	switch(uptr->type){
		case uTile:
			{struct undoTile*ut=(struct undoTile*)uptr->ptr;
			if(redo){
				if(ut->type&tTypeDeleteFlag){
					currentProject->tileC->remove_tile_at(ut->id);
					updateTileSelectAmt();
				}else
					tilesToU((uint8_t*)ut->ptrnew,ut->id,ut->type);
			}else{
				if((!ut->ptrnew)&&(!(ut->type&tTypeDeleteFlag))){
					unsigned sz=getSzTile(ut->type);
					ut->ptrnew=malloc(sz);
					memUsed+=sz;
				}
				if(ut->type&tTypeDeleteFlag){
					currentProject->tileC->insertTile(ut->id);
					updateTileSelectAmt();
				}else
					tilesTo((uint8_t*)ut->ptrnew,ut->id,ut->type);
				tilesToU((uint8_t*)ut->ptr,ut->id,ut->type);
			}}
		break;
		case uTilePixel:
			{struct undoTilePixel*ut=(struct undoTilePixel*)uptr->ptr;
			if(ut->type==tTypeTruecolor){
				if(redo)
					currentProject->tileC->setPixelTc(ut->id,ut->x,ut->y,ut->valnew);
				else{
					ut->valnew=currentProject->tileC->getPixelTc(ut->id,ut->x,ut->y);
					currentProject->tileC->setPixelTc(ut->id,ut->x,ut->y,ut->val);
				}
			}else{
				if(redo)
					currentProject->tileC->setPixel(ut->id,ut->x,ut->y,ut->valnew);
				else{
					ut->valnew=currentProject->tileC->getPixel(ut->id,ut->x,ut->y);
					currentProject->tileC->setPixel(ut->id,ut->x,ut->y,ut->val);
				}
			}}
		break;
		case uTileAll:
			{struct undoTileAll*ut=(struct undoTileAll*)uptr->ptr;
			if(redo)
				cpyAllTilesU((uint8_t*)ut->ptrnew,ut->amtnew,ut->type);
			else{
				if(!ut->ptrnew){
					ut->amtnew=currentProject->tileC->amt;
					unsigned sz=getSzTile(ut->type)*ut->amtnew;
					ut->ptrnew=malloc(sz);
					memUsed+=sz;
				}
				cpyAllTiles((uint8_t*)ut->ptrnew,ut->amtnew,ut->type);
				cpyAllTilesU((uint8_t*)ut->ptr,ut->amt,ut->type);
			}}
		break;
		case uTileAppend:
			if(redo)
				currentProject->tileC->appendTile();
			else
				currentProject->tileC->resizeAmt(currentProject->tileC->amt-1);
			updateTileSelectAmt();
		break;
		case uTilemapEdit:
			{struct undoTilemapEdit*um=(struct undoTilemapEdit*)uptr->ptr;
			if(redo)
				currentProject->tileMapC->setRaw(um->x,um->y,um->valnew);
			else{
				um->valnew=currentProject->tileMapC->getRaw(um->x,um->y);
				currentProject->tileMapC->setRaw(um->x,um->y,um->val);
			}
			if(tileEditModePlace_G)
				window->updateTileMapGUI(um->x,um->y);}
		break;
		case uTilemap:
		case uTilemapattr:
			{struct undoTilemap*um=(struct undoTilemap*)uptr->ptr;
			if(redo){
				if(uptr->type==uTilemapattr)
					attrCpyU(currentProject->tileMapC->tileMapDat,(uint8_t*)um->ptrnew,um->wnew*um->hnew);
				else{
					currentProject->tileMapC->resize_tile_map(um->wnew,um->hnew);
					memcpy(currentProject->tileMapC->tileMapDat,um->ptrnew,um->wnew*um->hnew*4);
				}
			}else{
				if(!um->ptrnew){
					um->wnew=currentProject->tileMapC->mapSizeW;
					um->hnew=currentProject->tileMapC->mapSizeHA;
					if(uptr->type==uTilemapattr){
						um->ptrnew=malloc(um->wnew*um->hnew);
						attrCpy((uint8_t*)um->ptrnew,currentProject->tileMapC->tileMapDat,um->wnew*um->hnew);
					}else{
						um->ptrnew=malloc(um->wnew*um->hnew*4);
						memcpy(um->ptrnew,currentProject->tileMapC->tileMapDat,um->wnew*um->hnew*4);
					}
				}
				if(uptr->type==uTilemapattr){
					attrCpyU(currentProject->tileMapC->tileMapDat,(uint8_t*)um->ptr,um->w*um->h);
				}else{
					currentProject->tileMapC->resize_tile_map(um->w,um->h);
					memcpy(currentProject->tileMapC->tileMapDat,um->ptr,um->w*um->h*4);
				}
			}}
		break;
		case uPalette:
			{struct undoPalette*up=(struct undoPalette*)uptr->ptr;
			unsigned sz,el;
			switch(currentProject->gameSystem){
				case sega_genesis:
					sz=128;
					el=64;
				break;
				case NES:
					el=sz=16;
				break;
			}
			if(redo)
				memcpy(currentProject->palDat,up->ptrnew,sz);
			else{
				if(!up->ptrnew){
					up->ptrnew=malloc(sz);
					memUsed+=sz;
				}
				memcpy(up->ptrnew,currentProject->palDat,sz);
				memcpy(currentProject->palDat,up->ptr,sz);
			}
			for(unsigned i=0;i<el;++i)
				updateRGBindex(i);
			}
			palEdit.updateSlider();
			tileEdit_pal.updateSlider();
			tileMap_pal.updateSlider();
		break;
		case uPaletteEntry:
			{struct undoPaletteEntry*up=(struct undoPaletteEntry*)uptr->ptr;
			switch(currentProject->gameSystem){
				case sega_genesis:
					{uint16_t*ptr=(uint16_t*)currentProject->palDat+up->id;
					if(redo)
						*ptr=up->valnew;
					else{
						up->valnew=*ptr;
						*ptr=up->val;
					}}
				break;
				case NES:
					if(redo)
						currentProject->palDat[up->id]=up->valnew;
					else{
						up->valnew=currentProject->palDat[up->id];
						currentProject->palDat[up->id]=up->val;
					}
				break;
			}
			updateRGBindex(up->id);
			switch (mode_editor){
				case pal_edit:
					palEdit.box_sel=up->id%palEdit.perRow;
					palEdit.changeRow(up->id/palEdit.perRow);
				break;
				case tile_edit:
					tileEdit_pal.box_sel=up->id%palEdit.perRow;
					tileEdit_pal.changeRow(up->id/tileEdit_pal.perRow);
					{unsigned focus=0;
					for(unsigned i=0;i<4;++i)
						focus|=Fl::focus()==window->palRTE[i];
					for(unsigned i=0;i<4;++i){
						if(focus&&(i==tileEdit_pal.theRow))
							Fl::focus(window->palRTE[i]);
						window->palRTE[i]->value(i==tileEdit_pal.theRow);
					}}
				break;
				case tile_place:
					tileMap_pal.box_sel=up->id%tileMap_pal.perRow;
					tileMap_pal.changeRow(up->id/tileMap_pal.perRow);
					{unsigned focus=0;
					for(unsigned i=0;i<4;++i)
						focus|=Fl::focus()==window->palRTE[i+4];
					for(unsigned i=0;i<4;++i){
						if(focus&&(i==tileMap_pal.theRow))
							Fl::focus(window->palRTE[i+4]);
						window->palRTE[i+4]->value(i==tileMap_pal.theRow);
					}}
				break;
			}}
		break;

	}
	if(!redo)
		--pos;
	window->redraw();
}
void pushTile(uint32_t id,tileTypeMask_t type){
	pushEventPrepare();
	struct undoEvent*uptr=undoBuf+pos;
	uptr->type=uTile;
	uptr->ptr=malloc(sizeof(struct undoTile));
	memUsed+=sizeof(struct undoTile);
	unsigned sz=getSzTile(type);
	struct undoTile*ut=(struct undoTile*)uptr->ptr;
	ut->ptr=malloc(sz);
	ut->ptrnew=0;
	ut->id=id;
	ut->type=type;
	memUsed+=sz;
	tilesTo((uint8_t*)ut->ptr,id,type);
}
void pushTilePixel(uint32_t id,uint32_t x,uint32_t y,tileTypeMask_t type){
	pushEventPrepare();
	struct undoEvent*uptr=undoBuf+pos;
	uptr->type=uTilePixel;
	uptr->ptr=malloc(sizeof(struct undoTilePixel));
	memUsed+=sizeof(struct undoTilePixel);
	struct undoTilePixel*ut=(struct undoTilePixel*)uptr->ptr;
	ut->id=id;
	ut->x=x;
	ut->y=y;
	ut->type=type;
	if(type==tTypeTruecolor)
		ut->val=currentProject->tileC->getPixelTc(id,x,y);
	else
		ut->val=currentProject->tileC->getPixel(id,x,y);
}
void pushTilesAll(tileTypeMask_t type){
	pushEventPrepare();
	struct undoEvent*uptr=undoBuf+pos;
	uptr->type=uTileAll;
	uptr->ptr=malloc(sizeof(struct undoTileAll));
	memUsed+=sizeof(struct undoTileAll);
	struct undoTileAll*ut=(struct undoTileAll*)uptr->ptr;
	ut->amt=currentProject->tileC->amt;
	unsigned sz=getSzTile(type)*ut->amt;
	ut->ptr=malloc(sz);
	memUsed+=sz;
	ut->type=type;
	ut->ptrnew=0;
	cpyAllTiles((uint8_t*)ut->ptr,ut->amt,type);
}
void pushTileAppend(void){
	pushEventPrepare();
	struct undoEvent*uptr=undoBuf+pos;
	uptr->type=uTileAppend;
}
void pushTilemapAll(bool attrOnly){
	pushEventPrepare();
	struct undoEvent*uptr=undoBuf+pos;
	if(attrOnly)
		uptr->type=uTilemapattr;
	else
		uptr->type=uTilemap;
	uptr->ptr=malloc(sizeof(struct undoTilemap));
	memUsed+=sizeof(struct undoTilemap);
	struct undoTilemap*um=(struct undoTilemap*)uptr->ptr;
	um->w=currentProject->tileMapC->mapSizeW;
	um->h=currentProject->tileMapC->mapSizeHA;
	um->ptrnew=0;
	if(attrOnly){
		um->ptr=malloc(um->w*um->h);
		attrCpy((uint8_t*)um->ptr,currentProject->tileMapC->tileMapDat,um->w*um->h);
	}else{
		um->ptr=malloc(um->w*um->h*4);
		memcpy(um->ptr,currentProject->tileMapC->tileMapDat,um->w*um->h*4);
	}
}
void pushTilemapEdit(uint32_t x,uint32_t y){
	pushEventPrepare();
	struct undoEvent*uptr=undoBuf+pos;
	uptr->type=uTilemapEdit;
	uptr->ptr=malloc(sizeof(struct undoTilemapEdit));
	memUsed+=sizeof(struct undoTilemapEdit);
	struct undoTilemapEdit*um=(struct undoTilemapEdit*)uptr->ptr;
	um->x=x;
	um->y=y;
	um->val=currentProject->tileMapC->getRaw(x,y);
}
void pushPaletteAll(void){
	pushEventPrepare();
	struct undoEvent*uptr=undoBuf+pos;
	uptr->type=uPalette;
	uptr->ptr=malloc(sizeof(struct undoPalette));
	memUsed+=sizeof(struct undoPalette);
	struct undoPalette*up=(struct undoPalette*)uptr->ptr;
	switch(currentProject->gameSystem){
		case sega_genesis:
			up->ptr=malloc(128);
			memcpy(up->ptr,currentProject->palDat,128);
			memUsed+=128;
		break;
		case NES:
			up->ptr=malloc(16);
			memcpy(up->ptr,currentProject->palDat,16);
			memUsed+=16;
		break;
	}
	up->ptrnew=0;
}
void pushPaletteEntry(uint32_t id){
	pushEventPrepare();
	struct undoEvent*uptr=undoBuf+pos;
	uptr->type=uPaletteEntry;
	uptr->ptr=malloc(sizeof(struct undoPaletteEntry));
	memUsed+=sizeof(struct undoPaletteEntry);
	struct undoPaletteEntry*up=(struct undoPaletteEntry*)uptr->ptr;
	up->id=id;
	switch(currentProject->gameSystem){
		case sega_genesis:
			{uint16_t*ptr=(uint16_t*)currentProject->palDat+id;
			up->val=*ptr;}
		break;
		case NES:
			up->val=(int32_t)currentProject->palDat[id];
		break;
	}
}
static Fl_Window * win;
static void closeHistory(Fl_Widget*,void*){
	win->hide();
}
void historyWindow(Fl_Widget*,void*){
	win=new Fl_Window(350,450,"History");
	win->begin();
	Fl_Button * Close=new Fl_Button(143,418,64,24,"Close");
	Close->callback(closeHistory);
	Fl_Browser*hist=new Fl_Browser(8,32,336,386);
	char tmp[2048];
	snprintf(tmp,2048,"%d items sorted from oldest to newest\nPosition selected: %d (can be -1)",amount,pos);
	hist->copy_label(tmp);
	hist->align(FL_ALIGN_TOP);
	for(unsigned n=0;n<amount;++n){
		struct undoEvent*uptr=undoBuf+n;
		switch(uptr->type){
			case uTile:
				{struct undoTile*ut=(struct undoTile*)uptr->ptr;
				if(ut->type&tTypeDeleteFlag)
					snprintf(tmp,2048,"Delete tile %d",ut->id);
				else
					snprintf(tmp,2048,"Change tile %d",ut->id);
				}
			break;
			case uTilePixel:
				{struct undoTilePixel*ut=(struct undoTilePixel*)uptr->ptr;
				if(ut->type&tTypeTruecolor)
					snprintf(tmp,2048,"Edit truecolor tile pixel X: %d Y: %d",ut->x,ut->y);
				else
					snprintf(tmp,2048,"Edit tile pixel X: %d Y: %d",ut->x,ut->y);
				}
			break;
			case uTileAll:
				{struct undoTileAll*ut=(struct undoTileAll*)uptr->ptr;
				snprintf(tmp,2048,"Change all tiles amount: %d",ut->amt);}
			break;
			case uTileAppend:
				strcpy(tmp,"Append tile");
			break;
			case uTilemap:
				strcpy(tmp,"Change tilemap");
			break;
			case uTilemapattr:
				strcpy(tmp,"Change tilemap attributes");
			break;
			case uTilemapEdit:
				{struct undoTilemapEdit*um=(struct undoTilemapEdit*)uptr->ptr;
				snprintf(tmp,2048,"Edit tilemap X: %d Y: %d",um->x,um->y);}	
			break;
			case uPalette:
				strcpy(tmp,"Change entire palette");
			break;
			case uPaletteEntry:
				{struct undoPaletteEntry*up=(struct undoPaletteEntry*)uptr->ptr;
				snprintf(tmp,2048,"Change palette entry: %d",up->id);}
			break;
			default:
				snprintf(tmp,2048,"TODO unhandled %d",uptr->type);
		}
		hist->add(tmp);
	}
	hist->select(pos+1);
	win->end();
	win->set_modal();
	win->show();
	while(win->shown())
		Fl::wait();
	delete win;
}
