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
#include "project.h"
#include "includes.h"
uint32_t currentChunk;
unsigned solidBits_G;
bool tileEditModeChunk_G;
unsigned ChunkOff[2]={DefaultChunkX,DefaultChunkY};
unsigned scrollChunks_G[2];
uint_fast32_t editChunk_G[2];//x,y
uint32_t selBlock;
void saveChunkS1CB(Fl_Widget*o,void*){
	currentProject->Chunk->exportSonic1();
}
void resizeChunkCB(Fl_Widget*o,void*){
	currentProject->Chunk->resize(window->chunksize[0]->value(),window->chunksize[1]->value());
	window->updateChunkSizeSliders();
	currentProject->Chunk->scrollChunks();
	window->redraw();
}
void selBlockCB(Fl_Widget*o,void*){
	Fl_Slider*s=(Fl_Slider*)o;
	selBlock=(uintptr_t)s->value();
	if(tileEditModeChunk_G){
		currentProject->Chunk->setBlock(currentChunk,editChunk_G[0],editChunk_G[1],selBlock);
		window->redraw();
	}
}
void solidCB(Fl_Widget*o,void*){
	Fl_Choice*s=(Fl_Choice*)o;
	solidBits_G=(uintptr_t)s->value();
	if(tileEditModeChunk_G){
		currentProject->Chunk->setSolid(currentChunk,editChunk_G[0],editChunk_G[1],solidBits_G);
		window->redraw();
	}
}
void ImportS1CBChunks(Fl_Widget*,void*a){
	bool append=(uintptr_t)a?true:false;
	if(load_file_generic("Pick chunk data from sonic 1")){
		currentProject->Chunk->importSonic1(the_file.c_str(),append);
		window->chunk_select->maximum(currentProject->Chunk->amt-1);
		window->redraw();
	}
}
void currentChunkCB(Fl_Widget*,void*){
	currentChunk=window->chunk_select->value();
	tileEditModeChunk_G=false;
	window->redraw();
}
void useBlocksCB(Fl_Widget*o,void*){
	Fl_Check_Button*b=(Fl_Check_Button*)o;
	bool use=b->value()?true:false;
	if(use){
		if(!(currentProject->tileMapC->isBlock)){
			fl_alert("You cannot use blocks without blocks");
			b->value(0);
			window->redraw();
			return;
		}
		window->tile_select_3->label("Block select");
	}else
		window->tile_select_3->label("Tile select");
	currentProject->Chunk->useBlocks=use;
	updateTileSelectAmt();
	window->redraw();
}
void scrollChunkCB(Fl_Widget*,void*){
	currentProject->Chunk->scrollChunks();
	window->redraw();
}
void scrollChunkX(Fl_Widget*,void*){
	scrollChunks_G[0]=window->chunkX->value();
	window->redraw();
}
void scrollChunkY(Fl_Widget*,void*){
	scrollChunks_G[1]=window->chunkY->value();
	window->redraw();
}
