#include"db.hpp"

//单元测试，每个接口单独测试
void TestImageTable()
{
  MYSQL *mysql = image_system::MySQLInit();
  image_system::ImageTable image_table(mysql);
  
  bool ret = false;

  //1、插入数据
#if 0 
  Json::Value image;
  image["image_name"] = "db_test.png";
  image["size"] = 1024;
  image["upload_time"] = "2019/08/27";
  image["md5"] = "abcdef";
  image["type"] = "png";
  image["path"] = "data/test.png";
  ret = image_table.Insert(image);
  printf("ret = %d\n", ret);
#endif

  //2、查找所有图片信息
#if 0
  Json::Value images;
  //Json::FastWriter writer;//Writer的子类FastWriter，不直接用Writer类。
  Json::StyledWriter writer;//StyledWriter类能够让序列化后的Json具有一定的格式，能够更方便看。
  ret = image_table.SelectAll(&images);
  printf("ret = %d\n", ret);
  printf("%s\n", writer.write(images).c_str());
  //批量注释代码
  //1、ctrl+v进入可视列模式
  //2、shift+i进入insert模式
  //3、//
  //4、esc
#endif
  
  //3、查找指定图片信息
#if 0
  Json::Value image;
  Json::StyledWriter writer;
  ret = image_table.SelectOne(1, &image);
  printf("ret = %d\n", ret);
  printf("%s\n", writer.write(image).c_str());
#endif
  
  //4、删除指定图片
  ret = image_table.Delete(3);
  printf("ret = %d\n", ret);

  image_system::MySQLRelease(mysql);
}

int main() {
  TestImageTable();
  return 0;
}
