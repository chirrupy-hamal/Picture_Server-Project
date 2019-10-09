.PHONY:all
all:image_server #db_test

FLAG=-L/usr/lib64/mysql -lmysqlclient -ljsoncpp

image_server:image_server.cc
	g++ $^ -o $@ -g -std=c++11 -pthread $(FLAG) -lcrypto

#db_test:db_test.cc db.hpp
#	g++ db_test.cc -o $@ -g $(FLAG)
#json库就在/usr/lib64下

.PHONY:clean
clean:
	rm image_server #db_test
