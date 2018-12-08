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
	Copyright Sega16 (or whatever you wish to call me) (2012-2017)
*/
#ifndef LUA_CHUNK_ENTRY_HPP
#define LUA_CHUNK_ENTRY_HPP
#include "lualib.h"
#include "lauxlib.h"

int luaopen_ChunkEntry(lua_State *L, size_t projectIDX, size_t chunkIDX, size_t columnIDX, size_t entryIDX);
#endif


