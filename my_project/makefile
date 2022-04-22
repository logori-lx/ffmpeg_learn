all: execute

execute: compile
	./car_client 

compile:
	gcc -o car_client car_control.c client.c dealdata.c -lwiringPi -lpthread

debug: debug_compile
	gdb car_client

debug_compile:
	gcc -g -o car_client car_control.c client.c dealdata.c -lwiringPi -lpthread