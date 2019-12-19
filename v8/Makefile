MODULE=v8
V8_PATH=/home/mesibo/v8/v8
V8_LIBPATH=$(V8_PATH)/out.gn/x64.release.sample/obj/
EXTRA_CCFLAGS=-I$(V8_PATH) -I$(V8_PATH)/include 
EXTRA_LDFLAGS=-Wl,-whole-archive -lv8_monolith -L$(V8_LIBPATH) -Wl,--no-whole-archive -lpthread
-include ../make.inc/make.inc

