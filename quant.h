/*	quantizer.h

	See quantizer.c for more information
	From http://www.gnu-darwin.org/www001/ports-1.5a-CURRENT/graphics/mtpaint/work/mtpaint-3.11/src/quantizer.c
*/
#pragma once


int dl1quant(unsigned char *inbuf, int width, int height,
			int quant_to, unsigned char userpal[3][256]);

int dl3quant(unsigned char *inbuf, int width, int height,
			int quant_to, unsigned char userpal[3][256],bool showProgress);

int dl3floste(unsigned char *inbuf, unsigned char *outbuf, int width, int height,
			int quant_to, int dither, unsigned char userpal[3][256]);
