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

function updateProjectGUI(sys)
	if sys==project.segaGenesis then
		palTabSel:show()
		spriteSizeSel:hide()
		spriteSizeTextToValueTable = nil
	else
		palTabSel:hide()
		spriteSizeSel:show()
		spriteSizeSel:clear()
		
		local spriteSizeOptions = {'8x8 sprites'}
		
		if sys == project.NES then
			spriteSizeOptions[2] = '8x16 sprites'
		else
			spriteSizeOptions[2] = '16x16 sprites'
		end

		spriteSizeTextToValueTable = generateLutFromList(spriteSizeOptions)
		addItemsToChoice(spriteSizeSel, spriteSizeOptions)

		local p = projects.current
		local spriteSizeID = p:getSpriteSizeID()

		spriteSizeSel:value(spriteSizeOptions[spriteSizeID + 1])
	end
end

function switchSystemBefore(old,new)
end

function switchSystemAfter(old,new)
	if is_headless == 0 then
		updateProjectGUI(new)
	end
end
