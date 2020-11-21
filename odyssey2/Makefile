CC = g++
CFLAGS = -Wall -Wextra -Wpedantic -Idependencies -Idependencies/glad/include -O3 -march=native
CXXFLAGS = $(CFLAGS) -Weffc++ -std=c++17
LINK = -ldl -lglfw

.PHONY: game.out clean

%.o: %.cpp
	$(CC) $(CXXFLAGS) -c $^ -o $@

OBJECTS = $(patsubst %.cpp,%.o,$(wildcard *.cpp)) dependencies/glad/src/glad.o

game.out: $(OBJECTS)
	$(CC) $(CXXFLAGS) $^ $(LINK) -o game.out

clean:
	@rm -rfv $(OBJECTS)
	@rm -vf game.out
