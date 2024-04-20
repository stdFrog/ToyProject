CC = g++

OBJS = Source\Maze.o Source\Tile.o
RCRS = $(OBJS : .o = .cpp)
MAIN = main

TARGET = $(MAIN)

all : $(TARGET)

CFLAGS = -W -Wall
OPT = -mwindows

$(TARGET): Source\$(MAIN).cpp $(OBJS)
	$(CC) $(CFLAGS) -o Release\$@ $^ $(OPT)

clean:
	del Release\*.exe
	del Source\*.o
