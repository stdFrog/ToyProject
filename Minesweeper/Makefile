CC = g++
CPPFLAGS = -W -Wall -I$(HDRDIR)

OPT = -mwindows
LDLIBS = -lcomctl32

HDRDIR	= Headers
SRCDIR	= Sources
RSRCDIR	= Resources
OBJDIR	= ObjectFiles
EXEDIR	= bin

HDRS = $(notdir $(wildcard $(HDRDIR)/*.h))
SRCS = $(notdir $(wildcard $(SRCDIR)/*.cpp))
RSRCS = $(notdir $(wildcard $(RSRCDIR)/*.rc))
OBJS = $(patsubst %.cpp,%.o, $(SRCS))

OUT_OBJS = $(addprefix $(OBJDIR)/, $(OBJS))
OUT_RSRCS = $(addprefix $(RSRCDIR)/, $(RSRCS))
OUT_RES = $(join $(OBJDIR)/, $(MAIN).res)

MAIN = Minesweeper
TARGET = $(EXEDIR)/$(MAIN).exe
all: $(TARGET)

$(TARGET): $(OUT_OBJS) $(OUT_RES)
	$(CC) $(CPPFLAGS) -o $@ $^ $(OPT) $(LDLIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) -c $^ -o $@

$(OUT_RES): $(OUT_RSRCS)
	windres $(OUT_RSRCS) -O coff -o $@ -I$(HDRDIR)

clean:
	del $(EXEDIR)\*.exe
	del $(OBJDIR)\*.o
	del $(OBJDIR)\*.res
