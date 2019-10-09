#插入模式下ma，再Ctrl+j
.PHONY:all
all:MySQL_insert MySQL_select

MySQL_insert:MySQL_insert.cc
	gcc $^ -o $@ -L /usr/lib64/mysql -lmysqlclient
#usr = unix shared resource
#库默认是在/usr/lib64中查找，但mysqlclient是在/usr/lib64/mysql下

MySQL_select:MySQL_select.cc
	gcc $^ -o $@ -L /usr/lib64/mysql -lmysqlclient

.PHONY:clean
clean:
	rm MySQL_insert MySQ_select
