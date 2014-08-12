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
#pragma once
#include <stdint.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <FL/Fl_Shared_Image.H>
#include "classSprite.h"
#include "gamedef.h"
struct spriteGroup{
	std::vector<int32_t> offx;
	std::vector<int32_t> offy;
	std::vector<uint32_t> loadat;//Where the sprite will be loaded in game (useful for games like sonic which overwrite vram to animate)
	std::string name;//Useful for nice formated output
	std::vector<class sprite> list;
};
class sprites{
	private:
		void mappingItem(void*in,uint32_t id,gameType_t game);
		void DplcItem(void*in,uint32_t which,gameType_t game);
		bool alreadyLoaded(uint32_t id,uint32_t subid);
		bool checkDupdplc(uint32_t id,uint32_t&which);
		bool recttoSprite(int x0,int x1,int y0,int y1,int where,Fl_Shared_Image*loaded_image,bool grayscale,unsigned*remap,uint8_t*palMap,uint8_t*mask,bool useMask,bool useAlpha);
	public:
		uint32_t amt;//The amount of sprites
		std::string name;
		std::vector<struct spriteGroup> groups;
		sprites();
		sprites(const sprites& other);
		~sprites();
		void importSpriteSheet(void);
		void exportDPLC(gameType_t game);
		void importDPLC(gameType_t game);
		void exportMapping(gameType_t game);
		void importMapping(gameType_t game);
		void draw(uint32_t id,uint32_t x,uint32_t y,int32_t zoom,bool mode,int32_t*outx=0,int32_t*outy=0);
		void minmaxoffx(uint32_t id,int32_t&minx,int32_t&maxx);
		void minmaxoffy(uint32_t id,int32_t&miny,int32_t&maxy);
		void spriteGroupToImage(uint8_t*img,uint32_t id,int row=-1,bool alpha=true);
		void spriteImageToTiles(uint8_t*img,uint32_t id,int rowUsage,bool alpha=true);
		uint32_t width(uint32_t id);
		uint32_t height(uint32_t id);
		void importImg(uint32_t to);//the paramter counts from 0
		bool load(FILE*fp,uint32_t version);
		bool save(FILE*fp);
		void setAmt(uint32_t amtnew);
		void setAmtingroup(uint32_t id,uint32_t amtnew);
		void del(uint32_t id);
		void delingroup(uint32_t id,uint32_t subid);
		void enforceMax(unsigned wmax,unsigned hmax);
};
