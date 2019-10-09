#include<fstream>
#include<signal.h>
#include<sys/stat.h>
#include<time.h>
#include<openssl/md5.h>
#include"db.hpp"
#include"httplib.h"

class FileUtil
{
  public:
    static bool Write(const std::string& file_name,
        const std::string& content)
    {
      std::ofstream file(file_name.c_str());
      if(!file.is_open())
        return false;
      file.write(content.c_str(), content.length());
      file.close();
      return true;
    }

    static bool Read(const std::string& file_name,
        std::string* content)
    {
      std::ifstream file(file_name.c_str());
      if(!file.is_open())
        return false;
      struct stat st;
      stat(file_name.c_str(), &st);
      content->resize(st.st_size);//设置成和文件一样长
      //一口气把整个文件都读完
      file.read((char*)content->c_str(), content->size());//缓冲区+读取多长
      file.close();
      return true;
    }
};

//回调函数
/*
   void Hello(const httplib::Request& req,
   httplib::Response& resp)
   {
   resp.set_content("hello", "text/html");
   }
   */

class TimeAndMd5
{
  public:
    static std::string GetNowTime()
    {
      time_t setTime;
      time(&setTime);
      tm *ptm = localtime(&setTime);
      std::string time = std::to_string(ptm->tm_year + 1900)
        + "/"
        +std::to_string(ptm->tm_mon + 1)
        + "/"
        +std::to_string(ptm->tm_mday)
        + " "
        +std::to_string(ptm->tm_hour)
        + ":"
        +std::to_string(ptm->tm_min)
        + ":"
        +std::to_string(ptm->tm_sec);
      return time;
    }

#define MD5_SECRET_LEN_16   16
#define MD5_BYTE_STRING_LEN 4
    static std::string commonMd5Secrrt32(const std::string& src)
    {
      MD5_CTX ctx;

      std::string md5String;
      unsigned char md[MD5_SECRET_LEN_16] = {0};
      char tmp[MD5_BYTE_STRING_LEN] = {0};

      MD5_Init(&ctx);
      MD5_Update(&ctx, src.c_str(), src.size());
      MD5_Final(md, &ctx);

      for(int i = 0; i < 16; ++i)
      {
        memset(tmp, 0x00, sizeof(tmp));
        snprintf(tmp, sizeof(tmp), "%02X", md[i]);
        md5String += tmp;
      }

      return md5String;
    }
};

MYSQL *mysql = NULL;//为了关闭mysql句柄

int main()
{
  using namespace httplib;

  mysql = image_system::MySQLInit();
  image_system::ImageTable image_table(mysql);
  signal(SIGINT, [](int){
      image_system::MySQLRelease(mysql);
      exit(0);
      });

  Server server;

  //server.Get("/hello", Hello);//路径+函数指针
  server.Get("/hello", [](const httplib::Request& req,
        httplib::Response& resp){
      (void)req;
      resp.set_content("<h1>hello</h1>", "text/html");
      });//lamda表达式
  //当有客户端请求一个path为"/hello"这样的请求时，
  //就会执行Hello函数。
  //指定不同的路径对应到不同的函数上，这个过程称为“设置路由”。
  server.set_base_dir("./wwwroot");//设置一个静态资源的目录
  //ip:9000/dog.jpg

  server.Post("/image", [&image_table](const Request& req, Response& resp){
      //1、对参数进行校验
      printf("上传图片\n");
      Json::Value resp_json;
      Json::FastWriter writer;
      auto ret = req.has_file("upload");//upload.html
      if(!ret)
      {
      printf("文件上传出错！\n");
      resp.status = 404;
      //使用json格式组织返回结果
      resp_json["ok"] = false;
      resp_json["reason"] = "上传文件出错，没有需要的upload字段";
      resp.set_content(writer.write(resp_json), "application/json");
      return;
      }

      //2、根据文件名获取到文件数据，文件数据就在file对象中。
      const auto& file = req.get_file_value("upload");
      auto body = req.body.substr(file.offset, file.length);//图片内容

      //3、把图片的属性信息插入到数据库中
      Json::Value image;
      image["image_name"] = file.filename;
      image["size"] = (int)file.length;
      image["upload_time"] = TimeAndMd5::GetNowTime().c_str();
      image["md5"] = TimeAndMd5::commonMd5Secrrt32(TimeAndMd5::GetNowTime().c_str());
      image["type"] = file.content_type;
      image["path"] = "./data/" + file.filename;
      ret = image_table.Insert(image);
      if(!ret)
      {
        printf("image_table Insert failed!\n");
        resp_json["ok"] = false;
        resp_json["reason"] = "数据库插入失败！";
        resp.status = 500;
        resp.set_content(writer.write(resp_json), "application/json");
        return;
      }

      //4、把图片保存到指定的磁盘目录中
      body = req.body.substr(file.offset, file.length);
      FileUtil::Write(image["path"].asString(), body);

      //5、构造一个响应数据通知客户端上串成功
      resp_json["ok"] = true;
      resp_json["status"] = 200;
      resp.set_content(writer.write(resp_json), "application/json");
  });
  //尝试插入gif，OK！

  //浏览器发送的是GET请求，如果测试POST请求、PUT请求、DELETE请求？浏览器显然不是那么方便。
  //Postman：这是一个测试工具，是一个http客户端，可以很方便的构造http各种请求并进行测试。
  server.Get("/image", [&image_table](const Request& req, Response& resp){
      (void)req;
      printf("获取所有图片信息\n");
      Json::Value resp_json;
      Json::StyledWriter writer; 
      //1、调用数据库接口来获取数据
      bool ret = image_table.SelectAll(&resp_json);
      if(!ret)
      {
      printf("查询数据库失败！\n");
      resp_json["ok"] = false;
      resp_json["reason"] = "查询数据库失败！";
      resp.status = 500;
      resp.set_content(writer.write(resp_json), "application/json");
      return;
      }

      //2、构造响应结果返回给客户端
      resp.status = 200;
      resp.set_content(writer.write(resp_json), "application/json");
      });

  //server.Get("/image/(\\d+)", [](const Request& req, Response& resp) 
  server.Get(R"(/image/(\d+))", [&image_table](const Request& req, Response& resp){
      printf("获取指定图片信息\n");
      Json::Value resp_json;
      Json::FastWriter writer;
      //1、先获取到图片id
      int image_id = std::stoi(req.matches[1]);//C++11
      printf("获取id为%d的图片信息！\n", image_id);

      //2、再根据图片id查询数据库
      bool ret = image_table.SelectOne(image_id, &resp_json);
      if(!ret)
      {
        printf("数据库查询出错！\n");
        resp_json["ok"] = false;
        resp_json["reason"] = "数据库查询出错！";
        resp.status = 404;//用户输入一个数据库没有的image_id，所以还是让用户背锅吧。
        resp.set_content(writer.write(resp_json), "application/json");
        return;
      }

      //3、把查询结果返回给客户端
      resp.status = 200;
      resp.set_content(writer.write(resp_json), "application/json");
  });//正则表达式：一个带有特殊符号的字符串，描述了一个字符串的特征。
  //字符串应该包含什么信息，以某个信息开头，以某个信息结尾，某个信息出现多少次……
  // "/numbers/(\d+)"，其/numbers/表示子串，而\d表示匹配一个数字(0~9字符)，+表示这个数字出现一次或多次。
  // /numbers/111就会被匹配到
  // /numbes/abc就不能被匹配到
  // "/image/(\\d+)"中第一个\用来转义
  //原始字符串(raw string, C++11)：原始字符串中没有转义字符

  server.Get(R"(/show/(\d+))", [&image_table](const Request& req, Response& resp){
      Json::Value resp_json;
      Json::FastWriter writer;
      //1、根据图片id去数据库中查到对应的目录
      int image_id = std::stoi(req.matches[1]);
      printf("获取id为%d的图片内容！\n", image_id);
      Json::Value image;
      bool ret = image_table.SelectOne(image_id, &image);
      if(!ret)
      {
        printf("读取数据库失败！\n");
        resp_json["ok"] = false;
        resp_json["reason"] = "数据库查询出错！";
        resp.status = 404;
        resp.set_content(writer.write(resp_json), "application/json");
        return;
      }
      
      //2、根据目找到文件(图片是以文件的形式存在磁盘上)内容，读取文件内容。
      std::string image_body;
      printf("%s\n", image["path"].asCString());
      ret = FileUtil::Read(image["path"].asString(), &image_body);
      if(!ret)
      {
        printf("读取图片文件失败！\n");
        resp_json["ok"] = false;
        resp_json["reason"] = "读取图片文件失败！";
        resp.status = 500;
        resp.set_content(writer.write(resp_json), "application/json");
        return;
      }
      
      //3、把文件内容构成一个响应
      resp.status = 200;
      //不同的图片，设置的content type是不一样的：
      //png设为image/png
      //jpg设为image/jpg
      //前端页面上传的图片，数据库中表中type字段就已经被设置为对应的，所以把type字段取出来就行了。
      resp.set_content(image_body, image["type"].asCString());
  });

  //Postman测试
  server.Delete(R"(/image/(\d+))", [&image_table](const Request& req, Response& resp){
      Json::Value resp_json;
      Json::FastWriter writer;
      //1、根据图片id去数据库中查找对应的目录
      int image_id = std::stoi(req.matches[1]);
      printf("删除id为%d的图片！\n", image_id);

      //2、查找对应的文件路径
      Json::Value image;
      bool ret = image_table.SelectOne(image_id, &image);
      if(!ret)
      {
        printf("数据库查找失败！\n");
        resp_json["ok"] = false;
        resp_json["reason"] = "数据库查找失败！";
        resp.status = 404;
        resp.set_content(writer.write(resp_json), "application/json");
        return;
      }

      //3、调用数据库操作进行删除
      ret = image_table.Delete(image_id);
      if(!ret)
      {
        printf("数据库删除失败！\n");
        resp_json["ok"] = false;
        resp_json["reason"] = "数据库删除失败！";
        resp.status = 404;
        resp.set_content(writer.write(resp_json), "application/json");
        return;
      }

      //4、删除磁盘上的图片文件
      //C++标准库中没有删除文件的方法
      //使用操作系统提供的函数
      //#include<unistd.h>
      //int unlink(const char *path);让硬链接的引用计数-1
      unlink(image["path"].asCString());

      //5、构造响应
      resp.status = 200;
      resp.set_content(writer.write(resp_json), "application/json");
  });

  server.listen("0.0.0.0", 9000);//启动服务器

  return 0;
}
