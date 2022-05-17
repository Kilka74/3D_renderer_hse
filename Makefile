CC=clang++
CFLAGS=-c -Wall -Werror -O3
LDFLAGS=

SOURCES := $(patsubst %.cpp,%.o,$(wildcard *.cpp))
SOURCES += $(patsubst %.cpp,%.o,$(wildcard */*.cpp))
SOURCES += $(patsubst %.cpp,%.o,$(wildcard */*/*.cpp))
SOURCES += $(patsubst %.cpp,%.o,$(wildcard */*/*/*.cpp))
SFM_OBJECTS=-lsfml-graphics -lsfml-window -lsfml-system
OBJECTS=$(SOURCES:.cpp=.o) $(SFM_OBJECTS)
EXECUTABLE=./bin/sfml_app.a

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f *.o
	rm -f */*.o
	rm -f */*/*.o
	rm -f */*/*/*.o
