CC = g++
CFLAGS = -g -Wall -Wextra -Wpedantic -O3 -march=native
IFLAGS = -Idependencies -Idependencies/glad/include -Idependencies/glfw-3.3/include -Idependencies/glm
CXXFLAGS = $(CFLAGS) $(IFLAGS) -Weffc++ -std=c++17
LINK = -ldl -lglfw

.PHONY: game.out clean

%.o: %.cpp
	$(CC) $(CXXFLAGS) -c $^ -o $@

%.o: %.c
	$(CC) $(CXXFLAGS) -c $^ -o $@

OBJECTS = $(patsubst %.cpp,%.o,$(wildcard *.cpp)) dependencies/glad/src/glad.o

game.out: $(OBJECTS)
	$(CC) $(CXXFLAGS) $^ $(LINK) -o game.out

clean:
	@rm -rfv $(OBJECTS)
	@rm -vf game.out
