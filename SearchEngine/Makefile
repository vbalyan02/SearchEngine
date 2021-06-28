COMPILER = g++
FLAGS = -c -g
LINKING = -l mysqlcppconn -l nuspell -l icuuc -l icudata -I /usr/include/boost/ -pthread

all: exe

exe: SearchEngine.cpp
	$(COMPILER) SearchEngine.cpp -o exe $(LINKING) -std=c++17 

clean:
	rm *.o exe