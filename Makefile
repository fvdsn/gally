CC = gcc
FLAGS = -Wall -g -pedantic `xml2-config --cflags`
LIBS = -lm -lpng `xml2-config --libs`
BIN = a.bin

all : Makefile loader

loader: ga_scene_load.o ga_scene.o
	${CC} ${FLAGS} ${LIBS} -o ${BIN} ga_scene_load.o ga_scene.o ga_geom.o ga_math.o ga_list.o ga_img.o

ga_scene_load.o: src/ga_scene_load.c src/ga_scene.h ga_math.o ga_geom.o ga_list.o
	${CC} ${FLAGS} -c src/ga_scene_load.c

ga_scene.o: src/ga_scene.c src/ga_scene.h ga_math.o ga_geom.o ga_list.o ga_img.o
	${CC} ${FLAGS} -c src/ga_scene.c

ga_geom.o: src/ga_geom.c src/ga_geom.h ga_math.o
	${CC} ${FLAGS} -c src/ga_geom.c

ga_img.o: src/ga_img.c src/ga_img.h ga_math.o
	${CC} ${FLAGS} -c src/ga_img.c

ga_math.o: src/ga_math.c src/ga_math.h
	${CC} ${FLAGS} -c src/ga_math.c 

ga_list.o: src/ga_list.c src/ga_list.h
	${CC} ${FLAGS} -c src/ga_list.c

clean:
	-rm *.o 

