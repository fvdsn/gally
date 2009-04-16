CC = gcc
FLAGS = -Wall -g -pedantic `xml2-config --cflags`
LIBS = -lm -lpng -lpthread `xml2-config --libs`
RAY_BIN = ray.bin
RAS_BIN = raster.bin
MAIN_BIN = render.bin

all : Makefile main

main: ga_main.o
	${CC} ${FLAGS} ${LIBS} -o ${MAIN_BIN} ga_scene_load.o ga_scene.o ga_geom.o ga_math.o ga_list.o ga_img.o ga_raster.o ga_raytrace.o ga_main.o ga_kdt.o

rasteriser: ga_raster.o 
	${CC} ${FLAGS} ${LIBS} -o ${RAS_BIN} ga_scene_load.o ga_scene.o ga_geom.o ga_math.o ga_list.o ga_img.o ga_raster.o

raytracer: ga_raytrace.o 
	${CC} ${FLAGS} ${LIBS} -o ${RAY_BIN} ga_scene_load.o ga_scene.o ga_geom.o ga_math.o ga_list.o ga_img.o ga_raytrace.o

loader: ga_scene_load.o ga_scene.o
	${CC} ${FLAGS} ${LIBS} -o ${BIN} ga_scene_load.o ga_scene.o ga_geom.o ga_math.o ga_list.o ga_img.o

ga_main.o : src/ga_main.c ga_raster.o ga_raytrace.o ga_kdtree.o
	${CC} ${FLAGS} -c src/ga_main.c

ga_kdt.o : src/ga_kdt.c src/ga_kdt.h ga_geom.o ga_list.o
	${CC} ${FLAGS} -c src/ga_kdt.c

ga_raster.o: src/ga_raster.c src/ga_raster.h ga_scene.o ga_scene_load.o ga_math.o ga_geom.o
	${CC} ${FLAGS} -c src/ga_raster.c

ga_raytrace.o: src/ga_raytrace.c src/ga_raytrace.h ga_scene.o ga_scene_load.o ga_math.o ga_geom.o
	${CC} ${FLAGS} -c src/ga_raytrace.c

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
	-rm *.o *.bin

