#include "image.h"
#include <string>
#include <iostream>

using namespace sas;

int main() {
  std::string org_path = "./image.jpeg";


  // 생성
  Image img;
  img.load(org_path);

  // 저장
  img.savePng("images/org_copy_image.png");
  img.saveJpg("images/org_copy_image.jpg");

  // resize
  std::cout << "resize 200x300" << std::endl;
  img.resize(200, 300);
  img.saveJpg("images/resize_200_300.jpg");

  std::cout << "resize 200x200" << std::endl;
  img.resize(200, 200);
  img.saveJpg("images/resize_200_200.jpg");

  // crop
  std::cout << "crop 100x50" << std::endl;  
  img.centerCrop(100, 50);
  img.saveJpg("images/crop_200_to_100x50.jpg");

  Image h_image(100, 100, 3, 0xFF);
  for (int i=50; i<100; i++) {
    for (int j=0; j<100; j++) {
      h_image.pixel(i, j, 0) = 0x00;
      h_image.pixel(i, j, 1) = 0x00;
      h_image.pixel(i, j, 2) = 0x00;      
    }
  }
  h_image.saveJpg("images/h_color.jpg");

  Image w_image(100, 100, 3, 0xFF);
  for (int i=0; i<100; i++) {
    for (int j=50; j<100; j++) {
      w_image.pixel(i, j, 0) = 0x00;
      w_image.pixel(i, j, 1) = 0x00;
      w_image.pixel(i, j, 2) = 0x00;      
    }
  }
  w_image.saveJpg("images/w_color.jpg");  

  Image x_image(100, 100, 3, 0xFF);
  for (int i=0; i<100; i++) {
      x_image.pixel(i, i, 0) = 0x00;
      x_image.pixel(i, i, 1) = 0x00;
      x_image.pixel(i, i, 2) = 0x00;

      x_image.pixel(100-i-1, i, 0) = 0x00;
      x_image.pixel(100-i-1, i, 1) = 0x00;
      x_image.pixel(100-i-1, i, 2) = 0x00;
  }
  x_image.saveJpg("images/x_color.jpg");

  Image target;
  target.load(org_path);
  target.resize(50, 50);

  target.saveJpg("images/target.jpg");
  
  Image stamp_b_image(100, 100, 3, uint8_t(0xFF));
  stamp_b_image.stamp(target, 10, 10);
  stamp_b_image.stamp(target, 20, 20);
  stamp_b_image.stamp(target, 30, 30);    
  stamp_b_image.stamp(target, 40, 40);
  stamp_b_image.stamp(target, 50, 50);
  stamp_b_image.stamp(target, 60, 60);
  stamp_b_image.stamp(target, 70, 70);
  
  stamp_b_image.saveJpg("images/stamp_b_image_x3.jpg");


  target.clear();
  target.load(org_path);
  target.addBorder(10, 20);
  target.savePng("images/add_border.png");

  return 0;
}
