TARGET=mysh
CC=gcc
CFLAG=-g

.PHONY: cleanall cleanobj

all: $(TARGET)

mysh: tokens.o tokens.o mysh.o

cleanobj:
	$(RM) tokens.o tokens.o mysh.o

cleanall: cleanobj
	$(RM) $(TARGET)
