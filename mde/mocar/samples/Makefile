EXE:=samples

SUBDIRS := $(wildcard */)

$(info $(SUBDIRS))


$(EXE):
	@ for dir in $(SUBDIRS); do make TPDIR=$(TOP_DIR)/$(EXE) -C $$dir; if [ "$$?" -ne "0" ]; then exit 3; fi; done
clean:
	@ for dir in $(SUBDIRS); do make TPDIR=$(TOP_DIR)/$(EXE) -C $$dir clean;  if [ "$$?" -ne "0" ]; then exit 3; fi; done

