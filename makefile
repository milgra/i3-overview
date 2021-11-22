UNAME := $(shell uname -s)
CC = clang
OBJDIRDEV = bin/obj/dev
OBJDIRREL = bin/obj/rel
VERSION = 1

SOURCES = \
	$(wildcard src/*.c) \
	$(wildcard src/modules/zen_core/*.c) \
	$(wildcard src/modules/zen_math/*.c) \
	$(wildcard src/modules/zen_text/*.c) \
	$(wildcard src/modules/json/*.c) \
	$(wildcard src/modules/storage/*.c) \
	$(wildcard src/overview/*.c)


CFLAGS = \
	-Isrc/ \
	-I/usr/include \
	-I/usr/include/X11 \
	-I/usr/include/gtk-3.0 \
	-I/usr/include/pango-1.0 \
	-I/usr/include/glib-2.0 \
	-I/usr/lib/glib-2.0/include \
	-I/usr/include/harfbuzz \
	-I/usr/include/freetype2 \
	-I/usr/include/libpng16 \
	-I/usr/include/libmount \
	-I/usr/include/blkid \
	-I/usr/include/fribidi \
	-I/usr/include/cairo \
	-I/usr/include/lzo \
	-I/usr/include/pixman-1 \
	-I/usr/include/gdk-pixbuf-2.0 \
	-I/usr/include/gio-unix-2.0 \
	-I/usr/include/cloudproviders \
	-I/usr/include/atk-1.0 \
	-I/usr/include/at-spi2-atk/2.0 \
	-I/usr/include/dbus-1.0 \
	-I/usr/lib/dbus-1.0/include \
	-I/usr/include/at-spi-2.0 \
	-pthread \
	-Isrc/modules/zen_core \
	-Isrc/modules/zen_math \
	-Isrc/modules/zen_text \
	-Isrc/modules/json \
	-Isrc/modules/storage

LDFLAGS = \
	-lm \
	-lpthread \
	-lX11 \
	-lXi \
	-lgtk-3 \
	-lgdk-3 \
	-lz \
	-lpangocairo-1.0 \
	-lpango-1.0 \
	-lharfbuzz \
	-latk-1.0 \
	-lcairo-gobject \
	-lcairo \
	-lgdk_pixbuf-2.0 \
	-lgio-2.0 \
	-lgobject-2.0 \
	-lglib-2.0 \
	-lfreetype



OBJECTSDEV := $(addprefix $(OBJDIRDEV)/,$(SOURCES:.c=.o))
OBJECTSREL := $(addprefix $(OBJDIRREL)/,$(SOURCES:.c=.o))

dev: $(OBJECTSDEV)
	$(CC) $^ -o bin/i3-overviewdev $(LDFLAGS) -fsanitize=address

rel: $(OBJECTSREL)
	$(CC) $^ -o bin/i3-overview $(LDFLAGS)

$(OBJECTSDEV): $(OBJDIRDEV)/%.o: %.c
	mkdir -p $(@D)
	$(CC) -c $< -o $@ $(CFLAGS) -g -DDEBUG -DVERSION=0 -DBUILD=0 -fsanitize=address

$(OBJECTSREL): $(OBJDIRREL)/%.o: %.c
	mkdir -p $(@D)
	$(CC) -c $< -o $@ $(CFLAGS) -O3 -DVERSION=$(VERSION) -DBUILD=$(shell cat version.num)

clean:
	rm -f $(OBJECTSDEV) i3-overview
	rm -f $(OBJECTSREL) i3-overview

vjump: 
	$(shell ./version.sh "$$(cat version.num)" > version.num)

install: rel
	/usr/bin/install -c -s -m 755 bin/i3-overview /usr/bin
	/usr/bin/install -d -m 755 /usr/share/i3-overview
	cp config /usr/share/i3-overview

remove:
	rm /usr/bin/i3-overview
	rm -r /usr/share/i3-overview
