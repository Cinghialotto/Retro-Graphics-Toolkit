--[[
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
	Copyright Sega16 (or whatever you wish to call me) (2012-2016)
--]]
function setPalTabCB(unused)
	project.setPalTab(palTabSel:value())
	palette.toRgbAll()
	rgt.damage()
end
function tabConfig(tab)
	if tab==rgt.paletteTab then
		palTabSel=Fl_Choice.new(336, 464, 128, 24,"Palette table selection")
		palTabSel:align(FL.ALIGN_TOP)
		palTabSel:callback('setPalTabCB')
		palTabSel:add("HardwareMan's measured values")
		palTabSel:add('round(255*v\\/7)')
		palTabSel:add('Steps of 36')
		palTabSel:add('Steps of 32')
		palTabSel:labelsize(12)
		palTabSel:value(0)
	elseif tab==rgt.levelTab then
		initLevelEditor()
	end
end
