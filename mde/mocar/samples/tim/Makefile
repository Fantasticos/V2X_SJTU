EXE:=tim

SRCS:= tim_main.c 

subdirs := $(wildcard */)

SRCS+=$(wildcard $(addsuffix *.c,$(subdirs)))

$(info $(SRCS))
objects += $(patsubst %.c,%.o,$(SRCS))

$(info $(objects))

LOCAL_CFLAGS:= -I../../libs/includes
LOCAL_LFLAGS:= -L../../libs -lmocarv2x -L ../../libs/base -lasnhl -lLLC -losstoed -lsmartway -lstack -lrt

%.o: %.c
	$(CR_CC) -c -g $< -o $@ $(LOCAL_CFLAGS)

$(EXE): $(objects)
	@ $(CR_CC) -o $@ $(objects) $(LOCAL_LFLAGS)
	@cp $(EXE) ../../bin
	@rm $(EXE)

clean:
	@ rm -rf *.o $(EXE)