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
	$(wildcard src/overview/config/*.c) \
	$(wildcard src/overview/tree/*.c) \
	src/overview/overview.c

CFLAGS = \
	-Isrc/ \
	-I/usr/include \
	-I/usr/include/X11 \
	-I/usr/include/freetype2 \
	-Isrc/modules/zen_core \
	-Isrc/modules/zen_math \
	-Isrc/modules/zen_text \
	-Isrc/modules/json \
	-Isrc/modules/storage \
	-Isrc/overview/config \
	-Isrc/overview/tree

LDFLAGS = \
	-lX11 \
	-lXi \
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
