CC = g++
AR = ar
RM = rm

CDEBUG = -g
LDEBUG = -g

OBJ_DIR = ../obj
BIN_DIR = ../bin

all: $(BIN_DIR)/CDataList

SRC = \
CDataList.cpp \

OBJS = $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRC))

LIBS = -lCStrUtil

CPPFLAGS = \
-std=c++11 \
-I. \
-I../../CStrUtil/include \

LFLAGS = \
-L../../CStrUtil/lib \

clean:
	$(RM) -f $(OBJ_DIR)/*.o
	$(RM) -f $(BIN_DIR)/CDataList

$(OBJS): $(OBJ_DIR)/%.o: %.cpp
	$(CC) $(CDEBUG) -c $< -o $(OBJ_DIR)/$*.o $(CPPFLAGS)

.SUFFIXES: .cpp

$(BIN_DIR)/CDataList: $(OBJ_DIR)/CDataList.o
	$(CC) $(LDEBUG) -o $(BIN_DIR)/CDataList $(OBJ_DIR)/CDataList.o $(LFLAGS) $(LIBS)
