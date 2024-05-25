CC = g++

MAIN = Minesweeper
OBJS = resource.o Button.o Color.o
RCRS = $(OBJS : .o=.cpp)

TARGET = $(MAIN)
all : $(TARGET)

OPT = -mwindows
CFLAGS = -W -Wall
RES = $(MAIN).res

$(RES) : resource.h $(MAIN).rc
	windres $(MAIN).rc -O Coff -o $(RES)

$(TARGET) : $(MAIN).cpp $(OBJS) $(RES)
	$(CC) $(CFLAGS) -o $@ $^ $(OPT) -lcomctl32

clean:
	del *.res
	del *.o
	del *.exe
