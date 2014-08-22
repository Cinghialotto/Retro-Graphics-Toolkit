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
#include <FL/Fl.H>
#include <FL/Fl_Hor_Value_Slider.H>
#include "color_convert.h"
#include "zlibwrapper.h"
struct Project ** projects;
uint32_t projects_count;//holds how many projects there are this is needed for realloc when adding or removing function
struct Project * currentProject;
Fl_Slider* curPrj;
static const char * defaultName="Add a description here.";
uint32_t curProjectID;
bool containsDataProj(uint32_t prj,uint32_t mask){
	unsigned off=__builtin_ctz(mask);
	return ((projects[prj]->useMask&mask)||(projects[prj]->share[off]>0))?true:false;
}
bool containsDataCurProj(uint32_t mask){
	return containsDataProj(curProjectID,mask);
}
void compactPrjMem(void){
	int Cold=0,Cnew=0;//Old and new capacity
	for(uint_fast32_t i=0;i<projects_count;++i){
		if(containsDataProj(i,pjHaveTiles)){
			Cold+=projects[i]->tileC->tDat.capacity();
			Cold+=projects[i]->tileC->truetDat.capacity();
			projects[i]->tileC->tDat.shrink_to_fit();
			projects[i]->tileC->truetDat.shrink_to_fit();
			Cnew+=projects[i]->tileC->tDat.capacity();
			Cnew+=projects[i]->tileC->truetDat.capacity();
		}
		if(containsDataProj(i,pjHaveChunks)){
			Cold+=projects[i]->Chunk->chunks.capacity();
			projects[i]->Chunk->chunks.shrink_to_fit();
			Cnew+=projects[i]->Chunk->chunks.capacity();
		}
		if(containsDataProj(i,pjHaveSprites)){
			for(uint32_t n=0;n<projects[i]->spritesC->amt;++n){
				Cold+=projects[i]->spritesC->groups[n].list.capacity();
				Cold+=projects[i]->spritesC->groups[n].list.capacity();
				Cold+=projects[i]->spritesC->groups[n].offx.capacity();
				Cold+=projects[i]->spritesC->groups[n].offy.capacity();
				projects[i]->spritesC->groups[n].loadat.shrink_to_fit();
				projects[i]->spritesC->groups[n].offx.shrink_to_fit();
				projects[i]->spritesC->groups[n].offy.shrink_to_fit();
				projects[i]->spritesC->groups[n].loadat.shrink_to_fit();
				Cnew+=projects[i]->spritesC->groups[n].list.capacity();
				Cnew+=projects[i]->spritesC->groups[n].list.capacity();
				Cnew+=projects[i]->spritesC->groups[n].offx.capacity();
				Cnew+=projects[i]->spritesC->groups[n].offy.capacity();
			}
			Cold+=projects[i]->spritesC->groups.capacity();
			projects[i]->spritesC->groups.shrink_to_fit();
			Cnew+=projects[i]->spritesC->groups.capacity();
		}
	}
	printf("Old capacity: %d New capacity: %d saved %d bytes\n",Cold,Cnew,Cold-Cnew);
}
void initProject(void){
	projects = (struct Project **) malloc(sizeof(void *));
	projects[0] = new struct Project;
	currentProject=projects[0];
	projects_count=1;
	currentProject->gameSystem=sega_genesis;
	currentProject->subSystem=3;
	currentProject->settings=0;
	currentProject->tileC=new tiles;
	currentProject->tileMapC=new tileMap;
	currentProject->Chunk=new ChunkClass;
	currentProject->spritesC=new sprites;
	currentProject->rgbPal=(uint8_t*)calloc(1,256);
	currentProject->palDat=(uint8_t*)calloc(1,128);
	currentProject->palType=(uint8_t*)calloc(1,64);
	currentProject->Name.assign(defaultName);
	std::fill(currentProject->share,&currentProject->share[shareAmtPj],-1);//Note always check to see if less than 0 do not use == -1
	currentProject->useMask=pjDefaultMask;
}
void setHaveProject(uint32_t id,uint32_t mask,bool set){
	/*This function will allocate/free data if and only if it is not being shared
	if have is already enabled no new data will be allocated
	if have was enabled data will be freeded*/
	if((mask&pjHavePal)&&(projects[id]->share[0]<0)){
		if(set){
			if(!(projects[id]->useMask&pjHavePal)){
				projects[id]->rgbPal=(uint8_t*)calloc(1,256);
				projects[id]->palType=(uint8_t*)calloc(1,64);
				projects[id]->useMask|=pjHavePal;
				switch(projects[id]->gameSystem){
					case sega_genesis:
						projects[id]->palDat=(uint8_t*)calloc(1,128);
						set_palette_type(0);
					break;
					case NES:
						projects[id]->palDat=(uint8_t*)malloc(128);
						memset(projects[id]->palDat,15,32);
						updateNesTab(0,false);
						updateNesTab(0,true);
						for(int temp_entry=0;temp_entry<32;++temp_entry){
							uint32_t rgb_out;
							rgb_out=MakeRGBcolor(projects[id]->palDat[temp_entry]);
							projects[id]->rgbPal[temp_entry*3+2]=rgb_out&255;//blue
							projects[id]->rgbPal[temp_entry*3+1]=(rgb_out>>8)&255;//green
							projects[id]->rgbPal[temp_entry*3]=(rgb_out>>16)&255;//red
						}
					break;
				}
				palEdit.changeSystem();
				tileEdit_pal.changeSystem();
				tileMap_pal.changeSystem();
				spritePal.changeSystem();
				if(projects[id]->gameSystem==NES)
					update_emphesis(0,0);
			}
		}else{
			if(projects[id]->useMask&pjHavePal){
				free(projects[id]->rgbPal);
				free(projects[id]->palDat);
				free(projects[id]->palType);
				projects[id]->useMask&=~pjHavePal;
			}
		}
	}
	if((mask&pjHaveTiles)&&(projects[id]->share[1]<0)){
		if(set){
			if(!(projects[id]->useMask&pjHaveTiles)){
				projects[id]->tileC = new tiles;
				projects[id]->useMask|=pjHaveTiles;
			}
		}else{
			if(projects[id]->useMask&pjHaveTiles){
				delete projects[id]->tileC;
				projects[id]->useMask&=~pjHaveTiles;
			}
		}
	}
	if((mask&pjHaveMap)&&(projects[id]->share[2]<0)){
		if(set){
			if(!(projects[id]->useMask&pjHaveMap)){
				projects[id]->tileMapC = new tileMap;
				projects[id]->useMask|=pjHaveMap;
			}
		}else{
			if(projects[id]->useMask&pjHaveMap){
				delete projects[id]->tileMapC;
				projects[id]->useMask&=~pjHaveMap;
			}
		}
	}
	if((mask&pjHaveChunks)&&(projects[id]->share[3]<0)){
		if(set){
			if(!(projects[id]->useMask&pjHaveChunks)){
				projects[id]->Chunk=new ChunkClass;
				projects[id]->useMask|=pjHaveChunks;
			}
		}else{
			if(projects[id]->useMask&pjHaveChunks){
				delete projects[id]->Chunk;
				projects[id]->useMask&=~pjHaveChunks;
			}
		}
	}
	if((mask&pjHaveSprites)&&(projects[id]->share[4]<0)){
		if(set){
			if(!(projects[id]->useMask&pjHaveSprites)){
				window->spriteglobaltxt->show();
				projects[id]->spritesC=new sprites;
				projects[id]->useMask|=pjHaveSprites;
			}
		}else{
			if(projects[id]->useMask&pjHaveSprites){
				window->spriteglobaltxt->hide();
				delete projects[id]->spritesC;
				projects[id]->useMask&=~pjHaveSprites;
			}
		}
	}
}
void shareProject(uint32_t share,uint32_t with,uint32_t what,bool enable){
	/*! share is the project that will now point to with's data
	what uses, use masks
	This function will not alter gui*/
	if(share==with){
		fl_alert("One does not simply share with itself");
		return;
	}
	if(enable){
		if(what&pjHavePal){
			if((projects[share]->share[0]<0)&&(projects[share]->useMask&pjHavePal)){
				free(projects[share]->rgbPal);
				free(projects[share]->palDat);
				free(projects[share]->palType);
			}
			projects[share]->share[0]=with;
			projects[share]->rgbPal=projects[with]->rgbPal;
			projects[share]->palDat=projects[with]->palDat;
			projects[share]->palType=projects[with]->palType;
		}
		if(what&pjHaveTiles){
			if((projects[share]->share[1]<0)&&(projects[share]->useMask&pjHaveTiles))
				delete projects[share]->tileC;
			projects[share]->share[1]=with;
			projects[share]->tileC=projects[with]->tileC;
		}
		if(what&pjHaveMap){
			if((projects[share]->share[2]<0)&&(projects[share]->useMask&pjHaveMap))
				delete projects[share]->tileMapC;
			projects[share]->share[2]=with;
			projects[share]->tileMapC=projects[with]->tileMapC;
		}
		if(what&pjHaveChunks){
			if((projects[share]->share[chunkEditor]<0)&&(projects[share]->useMask&pjHaveChunks))
				delete projects[share]->Chunk;
			projects[share]->share[chunkEditor]=with;
			projects[share]->Chunk=projects[with]->Chunk;
		}
		if(what&pjHaveSprites){
			if((projects[share]->share[spriteEditor]<0)&&(projects[share]->useMask&pjHaveSprites))
				delete projects[share]->spritesC;
			projects[share]->share[spriteEditor]=with;
			projects[share]->spritesC=projects[with]->spritesC;
		}
	}else{
		if(what&pjHavePal){
			if((projects[share]->share[0]>=0)&&(projects[share]->useMask&pjHavePal)){
				projects[share]->rgbPal=(uint8_t*)malloc(256);
				projects[share]->palDat=(uint8_t*)malloc(128);
				projects[share]->palType=(uint8_t*)malloc(64);
				memcpy(projects[share]->rgbPal,projects[with]->rgbPal,256);
				memcpy(projects[share]->palDat,projects[with]->palDat,128);
				memcpy(projects[share]->palType,projects[with]->palType,64);
			}
			projects[share]->share[0]=-1;
		}
		if(what&pjHaveTiles){
			if((projects[share]->share[1]>=0)&&(projects[share]->useMask&pjHaveTiles))//Create a copy of the shared data
				projects[share]->tileC = new tiles(*projects[with]->tileC);
			projects[share]->share[1]=-1;
		}
		if(what&pjHaveMap){
			if((projects[share]->share[2]>=0)&&(projects[share]->useMask&pjHaveMap))
				projects[share]->tileMapC = new tileMap(*projects[with]->tileMapC);
			projects[share]->share[2]=-1;//Even if we don't have the data sharing can still be disabled
		}
		if(what&pjHaveChunks){
			if((projects[share]->share[3]>=0)&&(projects[share]->useMask&pjHaveChunks))
				projects[share]->Chunk = new ChunkClass(*projects[with]->Chunk);
			projects[share]->share[3]=-1;//Even if we don't have the data sharing can still be disabled
		}
		if(what&pjHaveSprites){
			if((projects[share]->share[4]>=0)&&(projects[share]->useMask&pjHaveSprites))
				projects[share]->spritesC = new sprites(*projects[with]->spritesC);
			projects[share]->share[4]=-1;//Even if we don't have the data sharing can still be disabled
		}
	}
}
bool appendProject(void){
	projects = (struct Project **) realloc(projects,(projects_count+1)*sizeof(void *));
	if (!projects){
		show_realloc_error((projects_count+1)*sizeof(void *))
		return false;
	}
	projects[projects_count]= new struct Project;
	projects[projects_count]->tileC=new tiles;
	projects[projects_count]->tileMapC=new tileMap;
	projects[projects_count]->Chunk=new ChunkClass;
	projects[projects_count]->spritesC=new sprites;
	projects[projects_count]->Name.assign(defaultName);
	projects[projects_count]->rgbPal=(uint8_t*)calloc(1,256);
	projects[projects_count]->palDat=(uint8_t*)calloc(1,128);
	projects[projects_count]->palType=(uint8_t*)calloc(1,64);
	projects[projects_count]->gameSystem=sega_genesis;
	projects[projects_count]->subSystem=3;
	projects[projects_count]->settings=0;
	std::fill(projects[projects_count]->share,&projects[projects_count]->share[shareAmtPj],-1);
	projects[projects_count]->useMask=pjDefaultMask;
	++projects_count;
	//Realloc could have changed address
	currentProject=projects[curProjectID];
	window->projectSelect->maximum(projects_count-1);
	for(int x=0;x<shareAmtPj;++x)
		window->shareWith[x]->maximum(projects_count-1);
	return true;
}
bool removeProject(uint32_t id){
	//removes selected project
	if (projects_count<=1){
		fl_alert("You must have atleast one project.");
		return false;
	}
	if((projects[id]->share[tile_edit]<0)&&(projects[id]->useMask&pjHaveTiles))
		delete projects[id]->tileC;
	if((projects[id]->share[tile_place]<0)&&(projects[id]->useMask&pjHaveMap))
		delete projects[id]->tileMapC;
	if((projects[id]->share[chunkEditor]<0)&&(projects[id]->useMask&pjHaveChunks))
		delete projects[id]->Chunk;
	if((projects[id]->share[spriteEditor]<0)&&(projects[id]->useMask&pjHaveSprites))
		delete projects[id]->spritesC;
	if((projects[id]->share[pal_edit]<0)&&(projects[id]->useMask&pjHavePal)){
		free(projects[id]->rgbPal);
		free(projects[id]->palDat);
		free(projects[id]->palType);
	}
	delete projects[id];
	if((id+1)!=projects_count)//Are we not removing the project last on the list?
		memmove(projects+id,projects+id+1,sizeof(void*)*(projects_count-id-1));
	projects_count--;
	projects = (struct Project **) realloc(projects,projects_count*sizeof(void *));
	window->projectSelect->maximum(projects_count-1);
	for(int x=0;x<shareAmtPj;++x)
		window->shareWith[x]->maximum(projects_count-1);
	return true;
}
static void invaildProject(void){
	fl_alert("This is not a vaild Retro Graphics Toolkit project");
}
void switchProject(uint32_t id){
	window->TxtBufProject->text(projects[id]->Name.c_str());//Make editor displys new text
	window->GameSys[projects[id]->gameSystem]->setonly();
	switch(projects[id]->gameSystem){
		case sega_genesis:
			if(containsDataProj(id,pjHaveTiles))
				projects[id]->tileC->tileSize=32;
			shadow_highlight_switch->show();
			if(containsDataProj(id,pjHavePal)){
				palEdit.changeSystem();
				tileEdit_pal.changeSystem();
				tileMap_pal.changeSystem();
				spritePal.changeSystem();
				unsigned paltype;
				switch(projects[id]->subSystem&sgSHmask){
					case 0:
						paltype=0;
						window->genSHbtns[0]->set();
						window->genSHbtns[1]->clear();
						window->genSHbtns[2]->clear();
					break;
					case sgSon:
						paltype=8;
						window->genSHbtns[0]->clear();
						window->genSHbtns[1]->set();
						window->genSHbtns[2]->clear();
					break;
					case sgSHmask:
						paltype=16;
						window->genSHbtns[0]->clear();
						window->genSHbtns[1]->clear();
						window->genSHbtns[2]->set();
					break;
					default:
						show_default_error
				}
				set_palette_type(paltype);
			}
		break;
		case NES:
			if(containsDataProj(id,pjHaveTiles))
				projects[id]->tileC->tileSize=16;
			shadow_highlight_switch->hide();
			if(containsDataProj(id,pjHavePal)){
				updateNesTab(0,false);
				updateNesTab(0,true);
				for(int temp_entry=0;temp_entry<64;++temp_entry){
					uint32_t rgb_out;
					rgb_out=MakeRGBcolor(projects[id]->palDat[temp_entry]);
					projects[id]->rgbPal[temp_entry*3+2]=rgb_out&255;//blue
					projects[id]->rgbPal[temp_entry*3+1]=(rgb_out>>8)&255;//green
					projects[id]->rgbPal[temp_entry*3]=(rgb_out>>16)&255;//red
				}
				palEdit.changeSystem();
				tileEdit_pal.changeSystem();
				tileMap_pal.changeSystem();
				spritePal.changeSystem();
				update_emphesis(0,0);
			}
			window->subSysC->value(currentProject->subSystem&1);
		break;
	}
	//Make sure sliders have correct values
	if(containsDataProj(id,pjHaveMap)){
		window->updateMapWH(projects[id]->tileMapC->mapSizeW,projects[id]->tileMapC->mapSizeH);
		char tmp[16];
		snprintf(tmp,16,"%u",projects[id]->tileMapC->amt);
		window->map_amt->value(tmp);
	}
	if(containsDataProj(id,pjHaveTiles))
		updateTileSelectAmt(projects[id]->tileC->amt);
	for(int x=0;x<shareAmtPj;++x){
		window->sharePrj[x]->value(projects[id]->share[x]<0?0:1);
		window->havePrj[x]->value(projects[id]->useMask>>x&1);
		if(projects[id]->share[x]<0)
			window->havePrj[x]->show();
		else
			window->havePrj[x]->hide();
		if(projects[id]->useMask>>x&1){
			if(window->tabsHidden[x]){
				window->the_tabs->insert(*window->TabsMain[x],x);
				window->tabsHidden[x]=false;
			}
		}else{
			if(!window->tabsHidden[x]){
				if(projects[id]->share[x]<0){
					window->the_tabs->remove(window->TabsMain[x]);
					window->tabsHidden[x]=true;
				}
			}
		}
	}
	if(containsDataProj(id,pjHaveMap))
		window->BlocksCBtn->value(projects[id]->tileMapC->isBlock?1:0);
	if(containsDataProj(id,pjHaveChunks)){
		window->chunk_select->maximum(projects[id]->Chunk->amt-1);
		window->updateChunkSize(projects[id]->Chunk->wi,projects[id]->Chunk->hi);
	}
	if(containsDataProj(id,pjHaveMap))
		projects[id]->tileMapC->toggleBlocks(projects[id]->tileMapC->isBlock);
	if(containsDataProj(id,pjHaveChunks))
		window->updateBlockTilesChunk(id);
	if(projects[id]->gameSystem==NES)
		window->subSysC->show();
	else
		window->subSysC->hide();
	if(containsDataProj(id,pjHaveSprites)){
		window->updateSpriteSliders(id);
		window->spriteglobaltxt->show();
		window->spriteglobaltxt->value(currentProject->spritesC->name.c_str());
	}else{
		window->spriteglobaltxt->hide();
	}
	window->redraw();
}
static bool loadProjectFile(uint32_t id,FILE * fi,bool loadVersion=true,uint32_t version=currentProjectVersionNUM){
	if(fgetc(fi)!='R'){
		invaildProject();
		fclose(fi);
		return false;
	}
	if(fgetc(fi)!='P'){
		invaildProject();
		fclose(fi);
		return false;
	}
	char d=fgetc(fi);
	if(d){
		projects[id]->Name.clear();
		do{
			projects[id]->Name.push_back(d);
		}while(d=fgetc(fi));
	}else
		projects[id]->Name.assign(defaultName);
	if(loadVersion)
		fread(&version,1,sizeof(uint32_t),fi);
	printf("Read as version %d\n",version);
	if(version>currentProjectVersionNUM){
		fl_alert("The latest project version Retro Graphics Toolkit supports is %d but you are opening %d",currentProjectVersionNUM,version);
		fclose(fi);
		return false;
	}
	if(version)
		fread(&projects[id]->useMask,1,sizeof(uint32_t),fi);
	else
		projects[id]->useMask=pjHavePal|pjHaveTiles|pjHaveMap;
	fread(&projects[id]->gameSystem,1,sizeof(uint32_t),fi);
	if(version>=4){
		fread(&projects[id]->subSystem,1,sizeof(uint32_t),fi);
		if((version<6)&&(projects[id]->gameSystem==sega_genesis))
			projects[id]->subSystem=3;//Old projects were storing the wrong number for 4bit graphics even though that is what is stored
		if((version==4)&&(projects[id]->gameSystem==NES)){
			projects[id]->subSystem^=1;//Fix the fact that NES2x2 and NES1x1 were switched around in version 4
			projects[id]->subSystem|=2;//Default to 2 bit
		}
	}else
		projects[id]->subSystem=3;
	if(version>=8)
		fread(&projects[id]->settings,1,sizeof(uint32_t),fi);
	else
		projects[id]->settings=0;
	int entries,eSize;
	switch(projects[id]->gameSystem){
		case sega_genesis:
			entries=64;
			eSize=2;
			projects[id]->tileC->tileSize=32;
			projects[id]->tileC->tcSize=256;
		break;
		case NES:
			if(version>=7)
				entries=32;
			else
				entries=16;
			eSize=1;
			projects[id]->tileC->tileSize=16;
			projects[id]->tileC->tcSize=256;
		break;
	}
	if(projects[id]->useMask&pjHavePal){
		if(projects[id]->share[0]<0){
			fread(projects[id]->palDat,eSize,entries,fi);
			fread(projects[id]->palType,1,entries,fi);
			if((projects[id]->gameSystem==NES)&&(version<7)){
				memset(projects[id]->palDat+16,0,16);
				memset(projects[id]->palType+16,0,16);
			}
		}
	}
	if(projects[id]->useMask&pjHaveTiles){
		if(projects[id]->share[1]<0){
			fread(&projects[id]->tileC->amt,1,sizeof(uint32_t),fi);
			if(version<6)
				++projects[id]->tileC->amt;
			projects[id]->tileC->resizeAmt();
			decompressFromFile(projects[id]->tileC->tDat.data(),projects[id]->tileC->tileSize*(projects[id]->tileC->amt),fi);
			decompressFromFile(projects[id]->tileC->truetDat.data(),projects[id]->tileC->tcSize*(projects[id]->tileC->amt),fi);
		}
	}
	if(projects[id]->useMask&pjHaveMap){
		if(projects[id]->share[2]<0){
			fread(&projects[id]->tileMapC->mapSizeW,1,sizeof(uint32_t),fi);
			fread(&projects[id]->tileMapC->mapSizeH,1,sizeof(uint32_t),fi);
			if(version>=2){
				uint8_t isBlockTemp;
				fread(&isBlockTemp,1,sizeof(uint8_t),fi);
				projects[id]->tileMapC->isBlock=isBlockTemp?true:false;
				if(isBlockTemp)
					fread(&projects[id]->tileMapC->amt,1,sizeof(uint32_t),fi);
				else
					projects[id]->tileMapC->amt=1;
				projects[id]->tileMapC->mapSizeHA=projects[id]->tileMapC->mapSizeH*projects[id]->tileMapC->amt;
			}else
				projects[id]->tileMapC->mapSizeHA=projects[id]->tileMapC->mapSizeH;
			if(version>=8)
				fread(&projects[id]->tileMapC->offset,1,sizeof(int32_t),fi);
			else
				projects[id]->tileMapC->offset=0;
			projects[id]->tileMapC->tileMapDat=(uint8_t*)realloc(projects[id]->tileMapC->tileMapDat,4*projects[id]->tileMapC->mapSizeW*projects[id]->tileMapC->mapSizeHA);
			decompressFromFile(projects[id]->tileMapC->tileMapDat,4*projects[id]->tileMapC->mapSizeW*projects[id]->tileMapC->mapSizeHA,fi);
		}
	}
	if(projects[id]->useMask&pjHaveChunks){
		if(projects[id]->share[3]<0){
			if(version>=3){
				uint8_t useBlockTemp;
				fread(&useBlockTemp,1,sizeof(uint8_t),fi);
				projects[id]->Chunk->useBlocks=useBlockTemp?true:false;
				fread(&projects[id]->Chunk->wi,1,sizeof(uint32_t),fi);
				fread(&projects[id]->Chunk->hi,1,sizeof(uint32_t),fi);
				fread(&projects[id]->Chunk->amt,1,sizeof(uint32_t),fi);
				projects[id]->Chunk->resizeAmt();
				decompressFromFile(projects[id]->Chunk->chunks.data(),projects[id]->Chunk->wi*projects[id]->Chunk->hi*sizeof(struct ChunkAttrs)*projects[id]->Chunk->amt,fi);
			}
		}
	}
	if(projects[id]->useMask&pjHaveSprites){
		if(projects[id]->share[4]<0){
			if(version>=5)
				projects[id]->spritesC->load(fi,version);
		}
	}
	return true;
}
bool loadProject(uint32_t id){
	if(!load_file_generic("Load project",false))
		return true;
	FILE * fi=fopen(the_file.c_str(),"rb");
	std::fill(projects[id]->share,&projects[id]->share[shareAmtPj],-1);//One file projects do not support sharing
	if(loadProjectFile(id,fi))
		fclose(fi);
	return true;
}
static bool saveProjectFile(uint32_t id,FILE * fo,bool saveShared,bool saveVersion=true){
	/*!
	File format
	char R
	char P
	Null terminated project description or just 0 if default string
	uint32_t version the reason this is stored is for backwards compability if I change the file format starts at version 0
	if (version >= 1) uint32_t have mask
	You can find the format in project.h
	if these bits are zero skip it 
	uint32_t game system
	if(version >= 4) uint32_t sub System requires special handeling for version==4
	if(version>=8) settings
	palette data (128 bytes if sega genesis or 16 bytes if NES)
	if((version>=7)&&(gameSystem==NES)) 16 bytes for sprite specific palette
	Free locked reserved data 64 bytes if sega genesis or 32 (16 if version<7) if NES
	uint32_t tile count
	uint32_t compressed size tiles
	tile data will decompress to either 32 bytes * tile count if sega genesis or 16 bytes * tile count if NES and is compressed with zlib
	uint32_t compressed size truecolor tiles
	true color tile data always decompresses to 256 bytes * tile count and is compressed with zlib
	uint32_t map size w
	uint32_t map size h
	if(version>=2){
		uint8_t isBlocks 0 if not blocks non-zero if so
		if(isBlocks)
			uint32_t blocks amount also treat w and h as width and height per block
	}
	if(version>=8) int32_t tilemap tile offset
	uint32_t compressed size map
	map data will decompress to map size w * map size h * 4 and is compressed with zlib
	if(version>=3){
		uint8_t use blocks for chunks non zero if so 0 if not
		uint32_t width per chunk
		uint32_t height per chunk
		uint32_t amount of chunks
		uint32_t compresssed Chunk map size
		Chunk data (zlib compressed)
	}
	if(version>=5) sprite data (see documentation in classSprites.cpp
	*/
	fputc('R',fo);
	fputc('P',fo);
	if(strcmp(projects[id]->Name.c_str(),defaultName)!=0)
		fputs(projects[id]->Name.c_str(),fo);
	fputc(0,fo);
	if(saveVersion){
		uint32_t version=currentProjectVersionNUM;
		fwrite(&version,sizeof(uint32_t),1,fo);
	}
	uint32_t haveTemp;
	if(saveShared){
		haveTemp=projects[id]->useMask;
		for(unsigned x=0;x<shareAmtPj;++x)
			haveTemp|=(projects[id]->share[x]>=0?1:0)<<x;
	}else
		haveTemp=projects[id]->useMask;
	fwrite(&haveTemp,sizeof(uint32_t),1,fo);
	fwrite(&projects[id]->gameSystem,sizeof(uint32_t),1,fo);
	fwrite(&projects[id]->subSystem,sizeof(uint32_t),1,fo);
	fwrite(&projects[id]->settings,sizeof(uint32_t),1,fo);
	int entries,eSize;
	switch(projects[id]->gameSystem){
		case sega_genesis:
			entries=64;
			eSize=2;
		break;
		case NES:
			entries=32;
			eSize=1;
		break;
	}
	if(haveTemp&pjHavePal){
		if(saveShared||(projects[id]->share[0]<0)){
			fwrite(projects[id]->palDat,eSize,entries,fo);
			fwrite(projects[id]->palType,1,entries,fo);
		}
	}
	if(haveTemp&pjHaveTiles){
		if(saveShared||(projects[id]->share[1]<0)){
			fwrite(&projects[id]->tileC->amt,1,sizeof(uint32_t),fo);
			compressToFile(projects[id]->tileC->tDat.data(),projects[id]->tileC->tileSize*(projects[id]->tileC->amt),fo);
			compressToFile(projects[id]->tileC->truetDat.data(),projects[id]->tileC->tcSize*(projects[id]->tileC->amt),fo);
		}
	}
	if(haveTemp&pjHaveMap){
		if(saveShared||(projects[id]->share[2]<0)){
			fwrite(&projects[id]->tileMapC->mapSizeW,1,sizeof(uint32_t),fo);
			fwrite(&projects[id]->tileMapC->mapSizeH,1,sizeof(uint32_t),fo);
			uint8_t isBlockTemp=projects[id]->tileMapC->isBlock?1:0;
			fwrite(&isBlockTemp,1,sizeof(uint8_t),fo);
			if(isBlockTemp)
				fwrite(&projects[id]->tileMapC->amt,1,sizeof(uint32_t),fo);
			fwrite(&projects[id]->tileMapC->offset,1,sizeof(int32_t),fo);
			compressToFile(projects[id]->tileMapC->tileMapDat,4*projects[id]->tileMapC->mapSizeW*projects[id]->tileMapC->mapSizeHA,fo);
		}
	}
	if(haveTemp&pjHaveChunks){
		if(saveShared||(projects[id]->share[3]<0)){
			uint8_t useBlockTemp=projects[id]->Chunk->useBlocks?1:0;
			fwrite(&useBlockTemp,1,sizeof(uint8_t),fo);
			fwrite(&projects[id]->Chunk->wi,1,sizeof(uint32_t),fo);
			fwrite(&projects[id]->Chunk->hi,1,sizeof(uint32_t),fo);
			fwrite(&projects[id]->Chunk->amt,1,sizeof(uint32_t),fo);
			compressToFile(projects[id]->Chunk->chunks.data(),projects[id]->Chunk->wi*projects[id]->Chunk->hi*sizeof(struct ChunkAttrs)*projects[id]->Chunk->amt,fo);
		}
	}
	if(haveTemp&pjHaveSprites){
		if(saveShared||(projects[id]->share[4]<0))
			projects[id]->spritesC->save(fo);
	}
	return true;
}
bool saveProject(uint32_t id){
	//Start by creating a save file dialog
	if(!load_file_generic("Save project as...",true))
		return true;
	FILE * fo=fopen(the_file.c_str(),"wb");
	saveProjectFile(id,fo,true);
	fclose(fo);
	return true;
}
bool saveAllProjects(void){
	/*!The format is the same except it begins with
	char R
	char G
	uint32_t amount of projects stored
	uint32_t version
	Before each project header is
	int32_t share[shareAmtPj] For version 4 the value of shareAmtPj is 4 in version 5 the value is 5
	(format described in saveProject is repeated n amount of times let n = amount of projects stored) The only difference is version is not stored
	*/
	if(!load_file_generic("Save projects group as...",true))
		return true;
	FILE * fo=fopen(the_file.c_str(),"wb");
	fputc('R',fo);
	fputc('G',fo);
	fwrite(&projects_count,1,sizeof(uint32_t),fo);
	uint32_t version=currentProjectVersionNUM;
	fwrite(&version,sizeof(uint32_t),1,fo);
	for(uint32_t s=0;s<projects_count;++s){
		fwrite(projects[s]->share,shareAmtPj,sizeof(uint32_t),fo);
		saveProjectFile(s,fo,false,false);
	}
	fclose(fo);
	return true;
}
static void invaildGroup(void){
	fl_alert("This is not a valid project group");
}
static void readShare(unsigned amt,FILE*fi,unsigned x){
	fread(projects[x]->share,amt,sizeof(uint32_t),fi);
	if(amt<shareAmtPj)
		std::fill(&projects[x]->share[amt-1],&projects[x]->share[shareAmtPj],-1);
}
bool loadAllProjects(bool Old){
	if(!load_file_generic("Load projects group"))
		return true;
	FILE * fi=fopen(the_file.c_str(),"rb");
	if(fgetc(fi)!='R'){
		invaildGroup();
		fclose(fi);
		return false;
	}
	if(fgetc(fi)!='G'){
		invaildGroup();
		fclose(fi);
		return false;
	}
	uint32_t PC;
	fread(&PC,1,sizeof(uint32_t),fi);
	while(PC>projects_count)
		appendProject();
	uint32_t version;
	if(!Old){
		fread(&version,1,sizeof(uint32_t),fi);
		printf("Group is version %d\n",version);
		if(version>currentProjectVersionNUM){
			fl_alert("The latest project version Retro Graphics Toolkit supports is %d but you are opening %d",currentProjectVersionNUM,version);
			fclose(fi);
			return false;
		}
	}
	for(unsigned x=0;x<projects_count;++x){
		if(Old)
			readShare(3,fi,x);
		else if(version<4)
			readShare(3,fi,x);
		else if(version==4)
			readShare(4,fi,x);
		else if(version<8)
			readShare(5,fi,x);
		else
			readShare(shareAmtPj,fi,x);
		if(Old){
			if(!(loadProjectFile(x,fi,true,3)))
				return false;
		}else{
			if(!(loadProjectFile(x,fi,false,version)))
				return false;
		}
	}
	for(unsigned x=0;x<projects_count;++x){
		if(projects[x]->share[0]>=0)
			shareProject(x,projects[x]->share[0],pjHavePal,true);
		if(projects[x]->share[1]>=0)
			shareProject(x,projects[x]->share[1],pjHaveTiles,true);
		if(projects[x]->share[2]>=0)
			shareProject(x,projects[x]->share[2],pjHaveMap,true);
		if(version>=3){
			if(projects[x]->share[3]>=0)
				shareProject(x,projects[x]->share[3],pjHaveChunks,true);
			if(version>=5){
				if(projects[x]->share[4]>=0)
					shareProject(x,projects[x]->share[4],pjHaveSprites,true);
			}
		}
	}
	fclose(fi);
	return true;
}
