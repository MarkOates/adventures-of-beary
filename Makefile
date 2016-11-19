

TMX_DIR=/Users/markoates/Repos/tmx/src
ALLEGRO_DIR=/Users/markoates/Repos/allegro5


TMX_BUILD=-ltmx -lxml2 -lz -LE:/tmx-deps/lib -LE:/tmx-build/lib -LE:/libxml2-2.9.2-win32-x86/lib -LE:/iconv-1.14-win32-x86/lib


all: motion main
	g++ -o bin/bearys_bash obj/main.o obj/motion.o -LE:/allegro-5.1.11-mingw-edgar/lib -lallegro_monolith-debug.dll $(TMX_BUILD)


main:
	g++ -std=gnu++11 -c -o obj/main.o main.cpp -I$(ALLEGRO_DIR)/include -I$(TMX_DIR)


motion:
	g++ -std=gnu++11 -c -o obj/motion.o motion.cpp



