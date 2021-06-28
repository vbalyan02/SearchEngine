COMPILER = g++
FLAGS = -c -g

all: exe

exe: Crawler.o
	$(COMPILER) Crawler.o -o exe -l curl -l gumbo -l mysqlcppconn

Crawler.o: Crawler/Crawler.cpp Crawler/Crawler.h PageLoader/PageLoader.cpp PageLoader/PageLoader.h LoadResult/LoadResult.h LoadResult/LoadResult.cpp ParsedData/ParsedData.h ParsedData/ParsedData.cpp DataBase/DataBase.h DataBase/DataBase.cpp
	$(COMPILER) $(FLAGS) Crawler/Crawler.cpp -o Crawler.o -l mysqlcppconn -l curl -l gumbo 

clean:
	rm *.o exe