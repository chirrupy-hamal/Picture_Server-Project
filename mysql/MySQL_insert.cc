#include<cstdio>
#include<cstdlib>
#include<mysql/mysql.h>

int main()
{
  //使用MySQL API来操作数据库
  //1、先创建一个MySQL的句柄
  MYSQL *mysql = mysql_init(NULL);//初始化一个句柄
  
  //2、拿着句柄和数据库建立连接
  if(NULL == mysql_real_connect(mysql, "127.0.0.1", "root", "", "image_system", 3306, NULL, 0))
  {
    //数据库连接失败
    printf("连接失败！%s\n", mysql_error(mysql));
    return 1;
  }
  
  //3、设置客户端的编码格式，看你的数据库服务器是什么格式，两者必须得一致。
  mysql_set_character_set(mysql, "utf8");
  
  //4、拼接SQL语句
  char sql[4096] = {0};
  sprintf(sql, "insert into image_table values(null, 'insert_test.png', 1024, '2019/08/26', 'abcdef', 'png', 'data/insert_test.png')");
  
  //5、执行SQL语句，就是客户端把SQL语句通过socket发给服务器，服务器把执行结果再通过socket返回来。
  int ret = mysql_query(mysql, sql);
  if(0 != ret)
  {
    printf("执行sql失败！%s\n", mysql_error(mysql));
    return 1;
  }
  
  //6、关闭句柄
  mysql_close(mysql);
  
  return 0;
}
