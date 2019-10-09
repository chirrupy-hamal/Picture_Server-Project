图片 服务器
图床：图片服务器别名

一个网页上的图片是如何展示的？
1、有一个url来表示图片的位置
2、有一个image标签，里面引用这个位置。

项目的核心需求，就是实现一个http服务器，然后用这个服务器来存储图片，针对每个图片提供唯一的url，
有了这个url之后，就可以借助它把图片展示到网页上。
1、上传图片(上传图片就会得到一个url)
客户端(网页，包含一段特殊的html代码，html生成一个按钮，点击之后弹出文件选择框，
发送按钮，给服务器发送一个特殊的http请求，http服务器处理这个特殊的请求。)
2、根据图片的url访问图片，获取图片内容(获取图片内容就相当于下载到浏览器上)。
3、获取某图片的属性：大小、格式是png还是jpg、上传时间…等。
4、能上传就得能删除

模块划分
1、图片属性存储模块，使用数据库。
数据库设计：数据库中有几张表，每张表都是啥样的结构(表头信息是啥?)。
而此处只需要一张表就能搞定了。
create table image_table
(
  image_id int,
  image_name varchar(128),
  size int,#大小
  upload_time varchar(64),#上传时间
  type varchar(64),#类型
  path varchar(256),#图片文件在服务器上的路径
  md5 varchar(128),#校验和，用来进行校验图片内容的正确性。
                   #图片上传之后，服务器就可以计算一个该图片的md5值，
                   #后续用户下载图片的时候也能获取到该图片的md5，用户可以把自己计算的md5和服务器计算的md5对比，
                   #就知道自己的图片下载是否正确。
#针对图片的内容，直接存在磁盘文件上。
)
md5是一种字符串hash算法：
a)不管啥样的字符串，最终得到的md5值都是固定长度的；
b)如果一个字符串的内容稍有变化，得到的md5值差异很大；
c)通过原字符串计算md5很容易，但是拿到md5还原原字符串理论上不可能。
Linux下有个命令，md5sum +文件名：计算一个文件内容的md5
其中，d5有两个版本，64位版本得到8字节16进制的数字，128位版本得到16字节的16进制数字。

如何实现一个数据库的客户端程序？
MySQL已经提供了一系列的API实现客户端
如何安装MySQL的API？
yum list | grep mysql 
找devel(软件开发)字样的直接下载就OK了

2、服务器模块，给前端提供一些接口
设计服务器API
http服务器需要接收http请求，返回http响应，此处需要约定不同的请求来表示不同的操作方式。
例如，有些请求表示上传图片，有些请求表示查看图片，有些请求表示删除图片……
http协议中，哪些东西可以自定制来携带用户需要的一些信息呢？
http://ip:port/image(路径)?op=get&image_id=123 查看id为123的图片
http://ip:port/image(路径)?op=delete&image_id=123 删除id为123的图片
以上是一种比较传统的方法，借助url中的查询字符串来完成具体的操作。

此处使用Restual风格的设计
1、用http的方法(GET/POST/等)来表示操作的动词
   用GET表示查，用POST表示增，PUT表示改，DELETE表示删。
2、用http的url中的路径表示要操作的对象
3、补充信息一般使用body来传递，通常情况下body中使用json格式的数据来组织。
   json是一种数据组织格式，最主要的用途之一就是序列化。
   json源于javacript，json在javacript是用来表示一个"对象"的。
   以王者荣耀为例：
   {//json格式的数据都是用{}组织的
    "hero_name":"曹操",//键值对
    "skill1":"三段跳",
    "skill2":"剑气",
    "skill3":"加攻击+吸血",
    "skill4":"释放技能加攻速"//被动技能
   }
   json优势：方便看，换句话说就是方便调试(用眼看)。
   json劣势：组织格式的效率比较低(因为skill重复，但去掉的话，json直观看起来就不好理解了)，更占用存储空间和带宽(网络传输)。
   其实，效率对今天来说已经没那么关键了，所以json会经常用到。
4、响应数据通常也是用json格式组织，放到body中。
API的具体设计
1、上传图片
请求
POST /image HTTP/1.1
Content-Type:application/x-www-from-urlencoded
图片内容放在body中

响应
HTTP/1.1 200 OK
body中
{
  "ok":"true"
}
2、查看所有图片信息
请求
GET /image

响应
HTTP1.1 200 OK
//json也能表示数组
[
  {
    "image_id":"1",
    "image_name":"test.png",
    "type":"image/png",
    "md5":"test_data",
    "upload_time":"2019/08/26",
    "path":"data/test.png",
    "size":1024
  },
  {
    ……
  }
]
3、查看指定图片信息
请求
GET /image/image_id

响应
HTTP1.1 200 OK
{
    "image_id":1,
    "image_name":"test.png",
    "type":"image/png",
    "md5":"test_data",
    "upload_time":"2019/08/26",
    "path":"data/test.png",
    "size":1024;
}
4、查看指定图片内容
请求
GET /show/image_id

响应
HTTP/1.1 200 OK
content-type:image/png

[body 图片内容数据]
5、删除图片
请求
DELETE /image/image_id

响应
HTTP/1.1 200 OK
{
  "ok":"true"
}
