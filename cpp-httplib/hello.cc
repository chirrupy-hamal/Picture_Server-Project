//
//  hello.cc
//
//  Copyright (c) 2019 Yuji Hirose. All rights reserved.
//  MIT License
//

#include <httplib.h>
#include <iostream>
using namespace httplib;

int main(void) {
  Server svr;

  svr.Get("/hi", [](const Request & /*req*/, Response &res) {
      std::cout << "hello" << std::endl;
    res.set_content("Hello World!", "text/plain");
  });

  svr.listen("localhost", 9000);
}
