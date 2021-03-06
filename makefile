VERSION = 1
UNAME := $(shell uname -s)
CC = clang

OBJDIRDEV = bin/obj/dev
OBJDIRREL = bin/obj/rel
OBJDIRTEST = bin/obj/test

SOURCES = \
	$(wildcard src/*.c) \
	$(wildcard src/modules/zen_core/*.c) \
	$(wildcard src/modules/zen_math/*.c) \
	$(wildcard src/modules/zen_text/*.c) \
	$(wildcard src/modules/json/*.c) \
	$(wildcard src/modules/storage/*.c) \
	$(wildcard src/overview/config/*.c) \
	$(wildcard src/overview/tree/*.c)

SOURCES_MAIN = $(SOURCES) \
	src/overview/overview.c

SOURCES_TEST = $(SOURCES) \
	src/overview/overview_test.c

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
	-lm \
	-lX11 \
	-lXi \
	-lfreetype

OBJECTSDEV := $(addprefix $(OBJDIRDEV)/,$(SOURCES_MAIN:.c=.o))
OBJECTSREL := $(addprefix $(OBJDIRREL)/,$(SOURCES_MAIN:.c=.o))
OBJECTSTEST := $(addprefix $(OBJDIRTEST)/,$(SOURCES_TEST:.c=.o))

dev: $(OBJECTSDEV)
	$(CC) $^ -o bin/i3-overviewdev $(LDFLAGS) -fsanitize=address

rel: $(OBJECTSREL)
	$(CC) $^ -o bin/i3-overview $(LDFLAGS)

test: $(OBJECTSTEST)
	$(CC) $^ -o bin/i3-overview-test $(LDFLAGS) -fsanitize=address
	bin/i3-overview-test -c tst/test_config_1 -w tst/tst_ws1.json -t tst/tst_tree1.json -o tst/curr_tst1.bmp
	bin/i3-overview-test -c tst/test_config_2 -w tst/tst_ws1.json -t tst/tst_tree1.json -o tst/curr_tst2.bmp
	bin/i3-overview-test -c tst/test_config_1 -w tst/tst_ws2.json -t tst/tst_tree2.json -o tst/curr_tst3.bmp
	bin/i3-overview-test -c tst/test_config_2 -w tst/tst_ws2.json -t tst/tst_tree2.json -o tst/curr_tst4.bmp
	bin/i3-overview-test -c tst/test_config_1 -w tst/tst_ws3.json -t tst/tst_tree3.json -o tst/curr_tst5.bmp
	bin/i3-overview-test -c tst/test_config_2 -w tst/tst_ws3.json -t tst/tst_tree3.json -o tst/curr_tst6.bmp
	diff tst/tst1.bmp tst/curr_tst1.bmp
	diff tst/tst2.bmp tst/curr_tst2.bmp
	diff tst/tst3.bmp tst/curr_tst3.bmp
	diff tst/tst4.bmp tst/curr_tst4.bmp
	diff tst/tst5.bmp tst/curr_tst5.bmp
	diff tst/tst6.bmp tst/curr_tst6.bmp

$(OBJECTSDEV): $(OBJDIRDEV)/%.o: %.c
	mkdir -p $(@D)
	$(CC) -c $< -o $@ $(CFLAGS) -g -DDEBUG -DVERSION=0 -DBUILD=0 -fsanitize=address

$(OBJECTSREL): $(OBJDIRREL)/%.o: %.c
	mkdir -p $(@D)
	$(CC) -c $< -o $@ $(CFLAGS) -O3 -DVERSION=$(VERSION) -DBUILD=$(shell cat version.num)

$(OBJECTSTEST): $(OBJDIRTEST)/%.o: %.c
	mkdir -p $(@D)
	$(CC) -c $< -o $@ $(CFLAGS) -g -DDEBUG -DVERSION=0 -DBUILD=0 -fsanitize=address

clean:
	rm -f $(OBJECTSDEV) i3-overview
	rm -f $(OBJECTSREL) i3-overview
	rm -f $(OBJECTSTEST) i3-overview

vjump: 
	$(shell ./version.sh "$$(cat version.num)" > version.num)

install: rel
	/usr/bin/install -c -s -m 755 bin/i3-overview /usr/bin
	/usr/bin/install -d -m 755 /usr/share/i3-overview
	cp config /usr/share/i3-overview

remove:
	rm /usr/bin/i3-overview
	rm -r /usr/share/i3-overview
