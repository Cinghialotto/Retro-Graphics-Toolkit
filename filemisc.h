/*
	This file is part of Retro Graphics Toolkit

	Retro Graphics Toolkit is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or any later version.

	Retro Graphics Toolkit is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Retro Graphics Toolkit. If not, see <http://www.gnu.org/licenses/>.
	Copyright Sega16 (or whatever you wish to call me) (2012-2015)
*/
#pragma once
enum fileType_t{
	tCancle=-1,
	tBinary,
	tCheader,
	tASM,
	tBEX,
};
void saveStrifNot(FILE*fp,const char*str,const char*cmp);
void fileToStr(FILE*fp,std::string&s,const char*defaultStr);
int clipboardAsk(void);
fileType_t askSaveType(bool save=true,fileType_t def=tBinary);
bool saveBinAsText(void * ptr,size_t sizeBin,FILE * fp,fileType_t type,const char*comment,const char*label,int bits);
