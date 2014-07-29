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
static void cleanupEvent(uint32_t id){
	struct undoEvent*uptr=undoBuf+id;
	switch(uptr->type){
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
void popUndoRedo(bool redo){
	if((pos<0)&&(!redo))
		return;
	if(!amount)
		return;
	if(redo&&(pos>=int_fast32_t(amount)))
		return;
	if(redo&&(pos<=int_fast32_t(amount)))
		++pos;
	struct undoEvent*uptr=undoBuf+pos;
	switch(uptr->type){
		case uTile:
			{struct undoTile*ut=(struct undoTile*)uptr->ptr;
			if(ut->type&tTypeDelete){

			}}
		break;
		case uPaletteEntry://old=new-delta new=old+delta
			{struct undoPaletteEntry*up=(struct undoPaletteEntry*)uptr->ptr;
			switch(currentProject->gameSystem){
				case sega_genesis:
					{uint16_t*ptr=(uint16_t*)currentProject->palDat+up->id;
					if(redo)
						*ptr=up->valnew;
					else{
						up->valnew=*ptr;
						*ptr=up->val;
					}
					//printf("R: %d G: %d B: %d\n",(*ptr>>9)&7,(*ptr>>13)&7,(*ptr>>1)&1);
					currentProject->rgbPal[up->id*3+2]=palTab[((*ptr>>1)&7)+palTypeGen];//Blue note that bit shifting is different due to little endian
					currentProject->rgbPal[up->id*3+1]=palTab[((*ptr>>13)&7)+palTypeGen];//Green
					currentProject->rgbPal[up->id*3]=palTab[((*ptr>>9)&7)+palTypeGen];//Red
					}
				break;
				case NES:
					if(redo)
						currentProject->palDat[up->id]=up->valnew;
					else{
						up->valnew=currentProject->palDat[up->id];
						currentProject->palDat[up->id]=up->val;
					}
					{uint32_t rgb_out=MakeRGBcolor(currentProject->palDat[up->id]);
					currentProject->rgbPal[up->id*3+2]=rgb_out&255;//blue
					currentProject->rgbPal[up->id*3+1]=(rgb_out>>8)&255;//green
					currentProject->rgbPal[up->id*3]=(rgb_out>>16)&255;//red
					}
				break;
			}
			switch (mode_editor){
				case pal_edit:
					palEdit.box_sel=up->id%palEdit.perRow;
					palEdit.changeRow(up->id/palEdit.perRow);
				break;
				case tile_edit:
					tileEdit_pal.box_sel=up->id%palEdit.perRow;
					tileEdit_pal.changeRow(up->id/tileEdit_pal.perRow);
				break;
				case tile_place:
					tileMap_pal.box_sel=up->id%tileMap_pal.perRow;
					tileMap_pal.changeRow(up->id/tileMap_pal.perRow);
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
	struct undoTile*ut=(struct undoTile*)uptr->ptr;
	uint32_t sz=0;
	if(type&tTypeTile)
		sz+=currentProject->tileC->tileSize;
	if(type&tTypeTruecolor)
		sz+=currentProject->tileC->tcSize;
	ut->ptr=malloc(sz);
	ut->id=id;
	memUsed+=sz;
}
void pushTilePixel(uint32_t tile,uint32_t x,uint32_t y,uint32_t type){

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
