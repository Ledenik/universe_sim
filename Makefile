CC := g++
CFLAGS := -Wall

INC := include
LIB := lib
LIBFLG := -lglfw3 -lglfw3dll -lopengl32 -lgdi32

SRC := 	src/glad.c \
		src/main.cpp 

all:
	$(CC) $(CFLAGS) $(SRC) -I$(INC) -L$(LIB) $(LIBFLG) -o sim.exe
	./sim.exe