CFLAGS = `pkg-config libwave --cflags` -g -Wall -Werror -pedantic

LIBS = `pkg-config libwave --libs` -lasound -lm -ljansson

LDFLAGS = -Wall -g

SOURCES = \
	src/main.c \
	src/config.c \
	src/process.c \
	src/filter.c \
	src/in_out.c \
	src/audit.c

OBJECTS = $(SOURCES:%.c=build/%.o)

DEPENDENCIES = $(OBJECTS:%.o=%d)

build/sound_meter: build_dir $(OBJECTS)
	gcc $(LDFLAGS) $(OBJECTS) $(LIBS) -o build/sound_meter

%.o: %.c

build/src/%.o: src/%.c
	gcc $(CFLAGS) -c $< -o $@

build_dir:
	mkdir -p build/src

clean:
	rm *.o sound_meter
