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


#include <cstdio>
#include <stdint.h>
#include <stdexcept>

#include <FL/fl_ask.H>
#include "errorMsg.h"
#include "gui.h"
#include "filemisc.h"

void saveStrifNot(FILE*fp, const char*str, const char*cmp) {
	if (strcmp(cmp, str) != 0)
		fputs(str, fp);

	fputc(0, fp);
}
void fileToStr(FILE*fp, std::string&s, const char*defaultStr) {
	char d = fgetc(fp);

	if (d) {
		s.clear();

		do {
			s.push_back(d);
		} while ((d = fgetc(fp)));
	} else
		s.assign(defaultStr);
}
int clipboardAsk(void) {
	return fl_choice("File or clipboard?", "File", "Clipboard", "Cancel");
}
fileType_t askSaveType(bool save, fileType_t def) {
	return (fileType_t)MenuPopup(save ? "How would you like the file saved?" : "What type of file is this?", "Set if the file is saved as binary, C header, bex, or asm", 4, (unsigned)def, "Binary", "C header", "ASM", "BEX");
}
bool saveBinAsText(const void * ptr, size_t sizeBin, FILE * fp, fileType_t type, const char*comment, const char*label, int bits, boost::endian::order endian) {
	/*!
	This function saves binary data as plain text useful for C headers each byte is separated by a comma
	To use the clipboard specify file as NULL
	Returns True on success false on error
	Type can be:
	*/

	if (type == fileType_t::tBinary && !fp)
		throw std::invalid_argument("type cannot be tBinary when clipboard mode is enabled.");

	uint8_t * dat8 = (uint8_t *)ptr;
	uint16_t * dat16 = (uint16_t *)ptr;
	uint32_t * dat32 = (uint32_t *)ptr;
	char endc = ',';
	std::string temp;
	unsigned mask = (bits / 8) - 1;
	char tmp[16];

	if (mask) {
		if (sizeBin & mask) {
			fl_alert("Error file type unaligned to %d bits", bits);
			return false;
		}

		sizeBin /= mask + 1;
	}

	switch (bits) {
		case 8:
			mask = 31;
			break;

		case 16:
			mask = 15;
			break;

		case 32:
			mask = 7;
			break;
	}

	if (comment && type != fileType_t::tBinary) {
		switch (type) {
			case fileType_t::tCheader:
				temp.assign("// ");
				break;

			case fileType_t::tASM:
				temp.assign("; ");
				break;

			case fileType_t::tBEX:
				temp.assign("' ");
				break;

			default:
				show_default_error
		}

		temp.append(comment);
		temp.push_back('\n');
	}

	char hexStr[4];

	switch (type) {
		case fileType_t::tCheader:
			temp.append("#include <stdint.h>\n");
			temp.append("const uint");
			snprintf(tmp, 16, "%d", bits);
			temp.append(tmp);
			temp.append("_t ");
			temp.append(label);
			temp.append("[]={");
			strncpy(hexStr, "0x", sizeof(hexStr));
			break;

		case fileType_t::tASM:
		case fileType_t::tBEX:
			temp.append(label);
			temp.push_back(':');
			strncpy(hexStr, "$", sizeof(hexStr));
			break;
	}

	for (size_t x = 0; x < sizeBin; ++x) {
		if ((x & mask) == 0) {
			temp.push_back('\n');

			switch (type) {
				case fileType_t::tASM:
					switch (bits) {
						case 8:
							temp.append("\tdc.b ");
							break;

						case 16:
							temp.append("\tdc.w ");
							break;

						case 32:
							temp.append("\tdc.l ");
							break;
					}

					break;

				case fileType_t::tBEX:
					switch (bits) {
						case 8:
							temp.append("\tdata ");
							break;

						case 16:
							temp.append("\tdataint ");
							break;

						case 32:
							temp.append("\tdatalong ");
							break;
					}

					break;
			}
		}

		if (((x & mask) == mask) && (type != fileType_t::tCheader))
			endc = 0;
		else
			endc = ',';

		if (x == (sizeBin - 1))
			endc = '\n';

		switch (bits) {
			case 8:
			{
				uint8_t datTmp = *dat8++;

				if (type == fileType_t::tBinary)
					fwrite(&datTmp, sizeof(uint8_t), 1, fp);
				else
					snprintf(tmp, sizeof(tmp), "%s%X", hexStr, datTmp);
			}
			break;

			case 16:
			{
				uint16_t datTmp = *dat16++;

				if (type == fileType_t::tBinary) {
					if (endian == boost::endian::order::little)
						boost::endian::conditional_reverse_inplace<boost::endian::order::native, boost::endian::order::little>(datTmp);
					else if (endian == boost::endian::order::big)
						boost::endian::conditional_reverse_inplace<boost::endian::order::native, boost::endian::order::big>(datTmp);
					fwrite(&datTmp, sizeof(uint16_t), 1, fp);
				} else
					snprintf(tmp, sizeof(tmp), "%s%X", hexStr, datTmp);
			}
			break;

			case 32:
			{
				uint32_t datTmp = *dat32++;


				if (type == fileType_t::tBinary) {
					if (endian == boost::endian::order::little)
						boost::endian::conditional_reverse_inplace<boost::endian::order::native, boost::endian::order::little>(datTmp);
					else if (endian == boost::endian::order::big)
						boost::endian::conditional_reverse_inplace<boost::endian::order::native, boost::endian::order::big>(datTmp);
					fwrite(&datTmp, sizeof(uint32_t), 1, fp);
				} else
					snprintf(tmp, sizeof(tmp), "%s%X", hexStr, datTmp);
			}
			break;

			default:
				show_default_error
		}

		temp.append(tmp);

		if (endc)
			temp.push_back(endc);
	}

	if (type == fileType_t::tCheader)
		temp.append("};\n");

	if (type != fileType_t::tBinary) {
		if (fp)
			fputs(temp.c_str(), fp);
		else
			Fl::copy(temp.c_str(), temp.length(), 1);
	}

	return true;
}
