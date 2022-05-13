TARGET=mytar
CFLAGS=-std=gnu99 -Wno-deprecated-declarations

.PHONY: cleanall cleanobj

all: $(TARGET)

mytar: mytar.o inodemap.o

cleanobj:
	$(RM) mytar.o inodemap.o

cleanall: cleanobj
	$(RM) $(TARGET)
