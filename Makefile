
LDFLAGS:=`pkg-config gtk+-3.0 --libs` -rdynamic -lexpat -lm
CFLAGS:=`pkg-config gtk+-3.0 --cflags`
CFLAGS+=-DGDK_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED

#LDFLAGS:=`pkg-config gtk4 --libs` -rdynamic -lexpat -lm
#CFLAGS:=`pkg-config gtk4 --cflags`

#CFLAGS+= -Wfatal-errors -pedantic -Wall -Wextra -Werror
CFLAGS+= -Wfatal-errors -Wall -Wextra -Werror
CFLAGS+= -std=c99 -Iinclude

# because glib-compile-resources don't play nice...
CFLAGS+=  -Wno-overlength-strings


SRC:=$(wildcard src/*.c)
OBJ:=$(SRC:src/%.c=obj/%.o)
INC:=$(wildcard include/*.h)

# GCR should be on the path...
GCR=glib-compile-resources
CC=gcc

cLogicFun: $(OBJ)
	$(CC) $(OBJ) -o cLogicFun $(LDFLAGS)

$(OBJ): obj/%.o : src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# this one file needs a special case as it has many specific dependencies
src/resources.c: res/resource.xml res/ui.glade res/*.png
	$(GCR) res/resource.xml --generate-source --target=src/resources.c --generate


.PHONY: debug release
debug: CFLAGS+= -g
release: CFLAGS+= -O3

debug release: clean cLogicFun

.PHONY:	clean 
clean:
	rm obj/* -f
	rm cLogicFun -f

style: $(SRC) $(INC)
	astyle -A10 -s4 -S -p -xg -j -z2 -n src/* include/*
