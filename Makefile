

TMX_DIR=/Users/markoates/Repos/tmx/src
ALLEGRO_DIR=/Users/markoates/Repos/allegro5


TMX_BUILD=-ltmx -lxml2 -lz -LE:/tmx-deps/lib -LE:/tmx-build/lib -LE:/libxml2-2.9.2-win32-x86/lib -LE:/iconv-1.14-win32-x86/lib
ALLEGRO_LIBS=-lallegro_color -lallegro_font -lallegro_ttf -lallegro_dialog -lallegro_audio -lallegro_acodec -lallegro_primitives -lallegro_image -lallegro_main -lallegro


all: obj/motion.o obj/main.o
	g++ -o bin/adventures_of_beary obj/main.o obj/motion.o $(ALLEGRO_LIBS)


obj/main.o:
	g++ -std=gnu++11 -c -o obj/main.o main.cpp -I$(ALLEGRO_DIR)/include -I$(TMX_DIR)


obj/motion.o:
	g++ -std=gnu++11 -c -o obj/motion.o motion.cpp


.PHONY: clean
clean:
	-rm obj/*.o
	-rm bin/adventures_of_beary


