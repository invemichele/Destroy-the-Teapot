################################################################################
## Makefile to compile the game "Destroy the Teapot!" and the related objects ##
################################################################################

#see main.cpp for required libraries
LBITS := $(shell getconf LONG_BIT) #to avoid problems compiling with 32-bit and 64-bit architecture
ifeq ($(LBITS),64 )
   EXECUTABLES = destroy_the_teapot_64
   ALL_OBJECTS = main_64.o physic_system_64.o physic_object_64.o my3Dvector_64.o player_64.o
else
   EXECUTABLES = destroy_the_teapot_32
   ALL_OBJECTS = main.o physic_system.o physic_object.o my3Dvector.o player.o
endif

HEADERS = my3Dvector.h physic_object.h physic_system.h player.h game_handler.hpp graphics.hpp zpr.hpp

FLAGS=-Wall -I. -lGL -lGLU -lglut -lboost_system -pthread
DEBUG_FLAGS = -O0 -g

all: $(EXECUTABLES)


my3Dvector.o: my3Dvector.cpp my3Dvector.h
	c++ $(FLAGS) -c my3Dvector.cpp -o my3Dvector.o  
	
physic_object.o: physic_object.cpp my3Dvector.h physic_object.h
	c++ $(FLAGS) -c physic_object.cpp -o physic_object.o 

physic_system.o: physic_system.cpp my3Dvector.h physic_object.h physic_system.h
	c++ $(FLAGS) -c physic_system.cpp -o physic_system.o 

player.o: player.cpp player.h
	c++ $(FLAGS) -c player.cpp -o player.o 
	
main.o: main.cpp $(HEADERS)
	c++ $(FLAGS) -c main.cpp -o main.o

destroy_the_teapot_32: $(ALL_OBJECTS)
	c++ $(ALL_OBJECTS) -o ../destroy_the_teapot_32 $(FLAGS)

####################
## 64-bit version ##
####################
my3Dvector_64.o: my3Dvector.cpp my3Dvector.h
	c++ $(FLAGS) -c my3Dvector.cpp -o my3Dvector_64.o 
	
physic_object_64.o: physic_object.cpp my3Dvector.h physic_object.h
	c++ $(FLAGS) -c physic_object.cpp -o physic_object_64.o 

physic_system_64.o: physic_system.cpp my3Dvector.h physic_object.h physic_system.h
	c++ $(FLAGS) -c physic_system.cpp -o physic_system_64.o

player_64.o: player.cpp player.h
	c++ $(FLAGS) -c player.cpp -o player_64.o 

main_64.o: main.cpp $(HEADERS)
	c++ $(FLAGS) -c main.cpp -o main_64.o

destroy_the_teapot_64: $(ALL_OBJECTS)
	c++ $(ALL_OBJECTS) -o ../destroy_the_teapot_64 $(FLAGS)


clean: ; rm *~ *.o
clean_all: clean ; rm ../$(EXECUTABLES)
