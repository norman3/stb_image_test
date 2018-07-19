//------------------------------------------------------------------------------
// @file util/image.h
//------------------------------------------------------------------------------
#ifndef SAS_CATEGORY_UTIL_IMAGE_H_
#define SAS_CATEGORY_UTIL_IMAGE_H_
#include <memory>
#include <vector>
#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <sstream>
#include <string>

#include <iostream>

namespace sas {

//------------------------------------------------------------------------------
// @class Image
//------------------------------------------------------------------------------
// (x, y, z) 포지션은 (h, w, c)를 의미한다.
//------------------------------------------------------------------------------
struct Image {
 private:
  int h_;  // height
  int w_;  // width
  int c_;  // channel

  // 원래 std::vector<unsined char> 등으로 구성되는 것이 맞지만,
  // 여기서는 하위 stb api 에서 생성한 메모리를 바로 연결하기 위해
  // shared_ptr 포맷을 쓰도록 한다. 실제로는 uint8_t array 포인터이다.
  // pdata_를 array 포인터로 사용하기 위해 연결마다 deleter를 설정해준다.
  std::shared_ptr<uint8_t> pdata_;

 public:
  Image();
  Image(int h, int w, int c);
  Image(int h, int w, int c, uint8_t val);
  Image(const std::string& filename);

 private:
  Image(int h, int w, int c, const std::vector<uint8_t>& data);
  Image(int h, int w, int c, const uint8_t* data);
  Image(int h, int w, int c, std::shared_ptr<uint8_t> data);

 public:
  Image(const Image& image);
  Image(Image&& image);
  Image& operator=(const Image& image);
  Image& operator=(Image&& image);

 private:
  bool check(int x, int y, int z) const;

 public:
  size_t offset(int x, int y, int z) const;
  bool empty() const;
  size_t size() const;
  void clear();

 public:
  int h() const { return h_; }
  int w() const { return w_; }
  int c() const { return c_; }

  std::shared_ptr<uint8_t> data() const { return pdata_; }
  uint8_t* get() const { return data().get(); }

 public:
  uint8_t  pixel(int x, int y, int z) const;
  uint8_t& pixel(int x, int y, int z);
  uint8_t  pixelOrZero(int x, int y, int z) const;
  void addPixel(int x, int y, int z, uint8_t px);
  void subPixel(int x, int y, int z, uint8_t px);
  void setPixel(int x, int y, int z, uint8_t px);

  void setWhite();
  void setBlack();

 public:
  void swap(Image& image);
  Image copy() const;

 public:
  bool isSameSize(const Image& img) const;
  void resize(int new_h, int new_w);  // height, width 길이를 변경한다.
  void resizeHeight(int new_h);  // height 길이를 변경한다.
  void resizeWidth(int new_w);  // width 길이를 변경한다.

  bool resizeTo(Image* target) const;  // target 크기로 resize 하여 전

  // w/h 중 작은 쪽을 기준으로 new_size 크기로 변경한다.
  void resizeOnSmallerSide(int new_size);

  // w/h 중 큰 쪽을 기준으로 new_size 크기로 변경한다.
  void resizeOnLargerSide(int new_size);

 public:
  // Image 크기가 (w_, h_, 1)인 이미지 채널별 이미지 feature 를 반환.
  Image layer(int z) const;
  std::vector<Image> layers() const;

 public:
  void stamp(const Image& img, int x, int y);
  void addBorder(int h_border, int w_border, uint8_t pxl=0x00);
  void addBoxBorder(int border, uint8_t pxl=0x00);
  bool crop(int h, int w, int x, int y);
  bool centerCrop(int h, int w);
  bool centerCropWithRatio(float h_ratio, float w_ratio);
  bool centerSquaredCrop(float ratio= 1.0f);

  float aspectRatio() const;

 public:
  bool savePng(const std::string& filename) const;
  bool savePng(std::vector<uint8_t>* buffer) const;
  bool savePng(uint8_t* buffer, int size) const;
  bool saveJpg(const std::string& filename) const;
  bool saveJpg(std::vector<uint8_t>* buffer) const;
  bool saveJpg(uint8_t* buffer, int size) const;


  bool load(const std::string& filename, int num_channel=3);
  bool load(const std::vector<uint8_t>& raw,  int num_channel=3);
  bool load(const uint8_t* raw, size_t size, int num_channel=3);

 public:
  std::string str() const;

 public:
  bool operator==(const Image& image);
  bool operator!=(const Image& image);
};

}  // namespace sas
#endif  // SAS_CATEGORY_UTIL_IMAGE_H_
