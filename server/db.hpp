#pragma once 
#include<cstdio>
#include<cstdlib>
#include<mysql/mysql.h>
#include<jsoncpp/json/json.h>

namespace image_system
{
  static MYSQL *MySQLInit()
  {  
    //使用MySQL API来操作数据库
    //1、先创建一个MySQL的句柄
    MYSQL *mysql = mysql_init(NULL);//初始化一个句柄
    
    //2、拿着句柄和数据库建立连接
    if(NULL == mysql_real_connect(mysql, "127.0.0.1", "root", "", "image_system", 3306, NULL, 0))
    {
      //数据库连接失败
      printf("连接失败！%s\n", mysql_error(mysql));
      return NULL;
    }
    
    //3、设置客户端的编码格式，看你的数据库服务器是什么格式，两者必须得一致。
    mysql_set_character_set(mysql, "utf8");
    return mysql;
  }

  static void MySQLRelease(MYSQL *mysql)
  {
    mysql_close(mysql);
  }
  
  //操作数据库中image_table这个表，不涉及图片内容。
  class ImageTable
  {
    public:
      ImageTable(MYSQL *mysql)
        : _mysql(mysql)
      {}
    
      //Insert函数依赖的输入信息比较多，
      //为了防止参数太多，使用JSON来封装参数。
      //需要借助jsoncpp第三方库，使用C++操作和解析json。
      //jsoncpp库主要包含一个核心类(Json::Value，非常类似于std::map) + 两个重要方法
      //Reader::parse方法，把一个json格式的字符串转成一个Json::Value对象。这是一个反序列化动作。
      //Writer::write方法，把一个Json::Value对象转成一个json格式的字符串。这是一个序列化动作。
      bool Insert(const Json::Value& image)
      {
        //image对象就形如以下形式：
        //{
        //  "image_name":"test.png",
        //  "size":1024,//key必须用双引号括起来，value如果不是字符串则不必用双引号括起来。
        //  "upload_time":"2019/08/28",
        //  "md5":"abcd",
        //  "type":"png",
        //  "path":"data/test.png"
        //}
        //使用json的原因：
        //1、扩展更方便(用class类代替json也可以，但是一旦修改类中成员变量，那类中成员函数也需要做出相应修改)；
        //2、方便和服务器接收到的数据打通。

        //拼装SQL语句
        char sql[4*1024] = {0};
        sprintf(sql, "insert into image_table values(null, '%s', %d, '%s', '%s', '%s', '%s')",
            image["image_name"].asCString(),//image["image_name"]根据key获取value，这是一个JSON对象。
            image["size"].asInt(),
            image["upload_time"].asCString(),
            image["md5"].asCString(),
            image["type"].asCString(),
            image["path"].asCString());
        //JS操作json对象key、value 
        //var jsonObj = {
        //  "创维电视":50,
        //  "家电":40
        //}
        //根据key获取value
        //var value = jsonObj["家电"];//value = 40
        //添加key
        //jsonObj["西门子"] = 100;
        printf("[Insert sql] %s\n", sql);
        
        //执行SQL语句，就是客户端把SQL语句通过socket发给服务器，服务器把执行结果再通过socket返回来。
        int ret = mysql_query(_mysql, sql);
        if(0 != ret)
        {
          printf("Insert 执行 sql失败！%s\n", mysql_error(_mysql));
          return false;
        }
        
        return true;
        //直接拼装SQL的方式与一个严重的缺陷，容易受到SQL注入攻击。
      }

      bool SelectAll(Json::Value* images)//输出型参数
      {
        char sql[1024*4] = {0};
        sprintf(sql, "select * from image_table");
        int ret = mysql_query(_mysql, sql);
        if(0 != ret)
        {
          printf("SelectAll 执行 sql 失败！%s\n", mysql_error(_mysql));
          return false;
        }
        
        //遍历结果集合，并把结果写到images参数中。
        MYSQL_RES *result = mysql_store_result(_mysql);
        int rows = mysql_num_rows(result);
        for(int i = 0; i < rows; ++i)
        {
          MYSQL_ROW row = mysql_fetch_row(result);
          //数据库查出的每条记录都相当于是一个图片的信息，
          //需要把这个信息转成JSON格式。
          Json::Value image;
          image["image_id"] = atoi(row[0]);//添加key
          image["image_name"] = row[1];
          image["size"] = atoi(row[2]);
          image["upload_time"] = row[3];
          image["md5"] = row[4];
          image["type"] = row[5];
          image["path"] = row[6];

          images->append(image);
        }
        
        //释放结果集合，否则内存泄漏
        mysql_free_result(result);
        
        return true;
      }

      bool SelectOne(int image_id, Json::Value *image_ptr)
      {
        char sql[1024*4] = {0};
        sprintf(sql, "select * from image_table where image_id = %d", image_id);
        int ret = mysql_query(_mysql, sql);
        if(0 != ret)
        {
          printf("SelectOne 执行 sql 失败！%s\n", mysql_error(_mysql));
          return false;
        }
        
        //遍历结果集合
        MYSQL_RES *result = mysql_store_result(_mysql);
        int rows = mysql_num_rows(result);
        if(1 != rows)
        {
          printf("SelectOne 查询结果不是1条记录！实际查到%d条！\n", rows);
          return false;
        }
        MYSQL_ROW row = mysql_fetch_row(result);
        Json::Value image;
        image["image_id"] = atoi(row[0]);
        image["image_name"] = row[1];
        image["size"] = atoi(row[2]);
        image["upload_time"] = row[3];
        image["md5"] = row[4];
        image["type"] = row[5];
        image["path"] = row[6];
        *image_ptr = image;
        
        //释放结果集合
        mysql_free_result(result);
        
        return true;
      }

      bool Delete(int image_id)
      {
        char sql[1024*4] = {0};
        sprintf(sql, "delete from image_table where image_id = %d", image_id);
        int ret = mysql_query(_mysql, sql);
        if(0 != ret)
        {
          printf("Delete 执行 sql 失败！%s\n", mysql_error(_mysql));
          return false;
        }
        
        return true;
      }
    private:
      MYSQL *_mysql;
  };//end class ImageTable
}//end namespace image_system 
