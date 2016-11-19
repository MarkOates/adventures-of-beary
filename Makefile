

TMX_DIR=/Users/markoates/Repos/tmx/src
ALLEGRO_DIR=/Users/markoates/Repos/allegro5


ALLEGRO_LIBS=-lallegro_color -lallegro_font -lallegro_ttf -lallegro_dialog -lallegro_audio -lallegro_acodec -lallegro_primitives -lallegro_image -lallegro_main -lallegro
TMX_LIBS=-ltmx -lxml2 -lz


all: obj/motion.o obj/main.o
	g++ -o bin/adventures_of_beary obj/main.o obj/motion.o $(ALLEGRO_LIBS) $(TMX_LIBS)


obj/main.o:
	g++ -Wall -std=gnu++11 -c -o obj/main.o src/main.cpp -I$(ALLEGRO_DIR)/include -I$(TMX_DIR)


obj/motion.o:
	g++ -Wall -std=gnu++11 -c -o obj/motion.o src/motion.cpp


.PHONY: clean
clean:
	-rm obj/*.o
	-rm bin/adventures_of_beary


