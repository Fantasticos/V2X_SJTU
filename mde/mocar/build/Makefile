EXE:=mocar-sdk

TARGET_CC=arm-linux-gnueabihf-gcc
TARGET_LD=arm-linux-gnueabihf-ld
TARGET_AR=arm-linux-gnueabihf-ar

CC_FLAGS:=-Wall -Werror -fPIC
export CR_CC:=$(TARGET_CC) $(CC_FLAGS)

TOP_DIR="../"

BUILD_SUBDIRS:="$(TOP_DIR)/samples" 		  

BUILD_STRING=Building directories :

$(info $(BUILD_STRING) $(BUILD_SUBDIRS))

$(EXE):
	@ for dir in $(BUILD_SUBDIRS); do make -C $$dir; if [ "$$?" -ne "0" ]; then exit 3; fi; done

clean:
	@ rm ../bin/*
	@ for dir in $(BUILD_SUBDIRS); do make -C $$dir clean; if [ "$$?" -ne "0" ]; then exit 3; fi; done
	  
