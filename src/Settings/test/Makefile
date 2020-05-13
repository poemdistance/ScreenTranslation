CC = gcc

TEST_PROGRAM = test
OBJ = main.o
LIBS = -L../../lib -lshared -lui
INCLUDE = -I../include
COMMON_HEADER = -I../../CommonHeader

all : $(TEST_PROGRAM)

$(TEST_PROGRAM) : $(OBJ)
	$(CC) $(INCLUDE) $(COMMON_HEADER) $(LIBS) $(OBJ) -o test

$(OBJ) : %.o : %.c
	$(CC) $(INCLUDE) $(COMMON_HEADER) $(LIBS) -c $< -o $@

clean:
	rm -rf $(TEST_PROGRAM)
	rm -rf $(OBJ)
