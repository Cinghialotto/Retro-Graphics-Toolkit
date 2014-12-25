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
#include "CIE.h"
static inline int sq(int x){
	return x*x;
}
static inline double sqd(double x){
	return x*x;
}
unsigned nearestColIndex(int red,int green,int blue,uint8_t*pal,unsigned amt,bool checkType,unsigned off){
	unsigned bestcolor=0;
	switch(currentProject->nearestAlg){
		case aCiede2000:
			{
				double minerrord=1e99;
				for(int i=(amt-1)*3;i>=0;i-=3){
					double distance=ciede2000rgb(red,green,blue,pal[i],pal[i+1],pal[i+2]);
					if(!checkType||(currentProject->pal->palType[i/3+off]!=2)){
						if (distance < minerrord){
							minerrord = distance;
							bestcolor = i;
						}
					}
				}
			}
		break;
		case aWeighted:
			{
				uint32_t minerrori=0xFFFFFFFF;
				for(int i=(amt-1)*3;i>=0;i-=3){
					uint32_t distance=ColourDistance(red,green,blue,pal[i],pal[i+1],pal[i+2]);
					if(!checkType||(currentProject->pal->palType[i/3+off]!=2)){
						if (distance < minerrori){
							minerrori = distance;
							bestcolor = i;
						}
					}
				}
			}
		break;
		case aCIE76:
			{
				double minerrord=1e99;
				for(int i=(amt-1)*3;i>=0;i-=3){
					double L1,L2,a1,a2,b1,b2;
					Rgb2Lab255(&L1,&a1,&b1,red,green,blue);
					Rgb2Lab255(&L2,&a2,&b2,pal[i],pal[i+1],pal[i+2]);
					double distance=sqd(L1-L2)+sqd(a1-a2)+sqd(b1-b2);
					if(!checkType||(currentProject->pal->palType[i/3+off]!=2)){
						if (distance < minerrord){
							minerrord = distance;
							bestcolor = i;
						}
					}
				}
			}
		break;
		default:
			{
				int minerrori=(255*255)+(255*255)+(255*255)+1;
				for(int i=(amt-1)*3;i>=0;i-=3){
					int distance=sq((int)pal[i]-red)+sq((int)pal[i+1]-green)+sq((int)pal[i+2]-blue);
					if(!checkType||(currentProject->pal->palType[i/3+off]!=2)){
						if (distance < minerrori){
							minerrori = distance;
							bestcolor = i;
						}
					}
				}

			}

	}
	return bestcolor/3;
}
