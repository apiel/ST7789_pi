
ifneq ($(shell uname -m),x86_64)
GPIO := -lpigpio -lrt -DPIGPIO=1
endif

all: build run

build:
	g++ -o st7789 main.cpp $(GPIO)

run:
	sudo ./st7789
