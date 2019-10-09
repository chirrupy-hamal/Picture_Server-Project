create database if not exists image_system;
use image_system;

drop table if exists image_table;
create table image_table(
  image_id int not null primary key auto_increment,
  image_name varchar(128),
  size int,
  upload_time varchar(64),
  md5 varchar(128),
  type varchar(64),
  path varchar(256)
);
insert into image_table values(null, 'test.png', 1024, '2019/08/26', 'test_data', 'png', 'data/test.png');
