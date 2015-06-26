CPPFLAGS=-DLUA_COMPAT_5_2
CC=gcc
CXX=g++
#gentoo fix needs include directory set to /usr/include/fltk-1 if you are using a different distro then this may not apply to you
CFLAGS=-march=native -pg -ggdb -flto -fuse-linker-plugin -I/usr/include/fltk/ -Ikens/ -c -Wall -Wextra -Wdouble-promotion -O3 -pipe -march=native
CXXFLAGS= $(CFLAGS) -std=gnu++11
LDFLAGS=-flto -fuse-linker-plugin -O3 -pg -L/usr/lib/fltk/ -lfltk_images -lfltk -lpng -ljpeg -lXft -lXext -lXinerama -lX11 -lz
OBJECTS=project.o main.o callbacks_palette.o callback_tiles.o class_global.o quant.o tilemap.o color_convert.o \
	errorMsg.o classpalettebar.o dither.o class_tiles.o kens/nemesis.o \
	kens/enigma.o kens/kosinski.o spatial_color_quant.o NEUQUANT.o \
	classtilemap.o palette.o zlibwrapper.o color_compare.o windowinit.o tiles_io.o savepng.o \
	callback_project.o callback_tilemap.o callback_gui.o classChunks.o compressionWrapper.o callback_chunk.o gui.o \
	wu.o system.o filemisc.o classSprite.o classSprites.o callbacksprites.o undo.o image.o \
	classlevel.o runlua.o nearestColor.o CIE.o classpalette.o classtilemaps.o callbacktilemaps.o \
	nespal.o callbacklevel.o filereader.o
EXECUTABLE=RetroGraphicsToolkit.profile

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $@
.c.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@
.cc.o:
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -o $@
.cpp.o:
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -o $@
clean:
	rm $(OBJECTS) $(EXECUTABLE)
