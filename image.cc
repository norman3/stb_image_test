//------------------------------------------------------------------------------
// @file image.cc
//------------------------------------------------------------------------------
#include "image.h"
#include <cassert>
#include <vector>
#include <sstream>
#include <algorithm>
#include <limits>

//------------------------------------------------------------------------------
// stb module. (don't use this in header but source)
//------------------------------------------------------------------------------
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#define STB_IMAGE_INLINE
#include "stb_image.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_STATIC
#define STB_IMAGE_RESIZE_INLINE
#include "stb_image_resize.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_STATIC
#define STB_IMAGE_WRITE_INLINE
#include "stb_image_write.h"

namespace sas {

//------------------------------------------------------------------------------
// invalid 한 base 이미지 객체를 생성한다.
//------------------------------------------------------------------------------
Image::Image() : h_{0}, w_{0}, c_{0}, pdata_(nullptr) {
}

//------------------------------------------------------------------------------
// (h, w, c) 크기의 이미지를 생성한다. 빈 이미지가 생성된다.
//------------------------------------------------------------------------------
Image::Image(int h, int w, int c)
    : h_{h}, w_{w}, c_{c}, pdata_(nullptr) {
  assert((h_>0) && (w_>0) && (c_>0));
  assert(size() > 0);
  pdata_.reset(new uint8_t[size()], std::default_delete<uint8_t[]>());
}

//------------------------------------------------------------------------------
// (h, w, c) 데이터에 초기값을 pxl 로 설정하여 생성.
//------------------------------------------------------------------------------
Image::Image(int h, int w, int c, uint8_t pxl)
    : h_{h}, w_{w}, c_{c}, pdata_(nullptr) {
  assert((h_>0) && (w_>0) && (c_>0));
  pdata_.reset(new uint8_t[size()], std::default_delete<uint8_t[]>());
  for (decltype(size()) i=0; i<size(); i++)
    pdata_.get()[i] = pxl;
}

//------------------------------------------------------------------------------
// 데이터를 복사하여 생성.
//------------------------------------------------------------------------------
Image::Image(int h, int w, int c, const std::vector<uint8_t>& data)
    : h_{h}, w_{w}, c_{c}, pdata_(nullptr) {
  assert((h_>0) && (w_>0) && (c_>0));
  assert(size() == data.size());
  pdata_.reset(new uint8_t[size()], std::default_delete<uint8_t[]>());
  // data copy
  for (decltype(size()) i=0; i<size(); i++)
    pdata_.get()[i] = data[i];
}

//------------------------------------------------------------------------------
// 데이터를 복사하여 생성.
//------------------------------------------------------------------------------
Image::Image(int h, int w, int c, const uint8_t* pdata)
    : h_{h}, w_{w}, c_{c}, pdata_(nullptr){
  assert((h_>0) && (w_>0) && (c_>0));
  assert(pdata);
  pdata_.reset(new uint8_t[size()], std::default_delete<uint8_t[]>());
  // data copy (주의깊게 사용할 필요 있음. overflow 문제)
  for (decltype(size()) i=0; i<size(); i++)
    pdata_.get()[i] = pdata[i];
}

//------------------------------------------------------------------------------
// 데이터를 share 하여 생성.
//------------------------------------------------------------------------------
Image::Image(int h, int w, int c, std::shared_ptr<uint8_t> pdata)
    : h_{h}, w_{w}, c_{c}, pdata_(pdata) {
  assert((h_>0) && (w_>0) && (c_>0));
  assert(pdata);
}

//------------------------------------------------------------------------------
// 파일에서 이미지 로드.
//------------------------------------------------------------------------------
Image::Image(const std::string& filename)
    : h_{0}, w_{0}, c_{0}, pdata_(nullptr) {
  load(filename);
}

//------------------------------------------------------------------------------
// 복사 생성자.
//------------------------------------------------------------------------------
Image::Image(const Image& image)
    : h_{image.h_}, w_{image.w_}, c_{image.c_}, pdata_(nullptr) {
  if (!image.empty()) {
    pdata_.reset(new uint8_t[size()], std::default_delete<uint8_t[]>());
    for (decltype(size()) i=0; i<size(); i++)
      pdata_.get()[i] = image.pdata_.get()[i];
  }
}

//------------------------------------------------------------------------------
// 이동 생성자.
//------------------------------------------------------------------------------
Image::Image(Image&& image) : h_{0}, w_{0}, c_{0}, pdata_(nullptr) {
  swap(image);
  image.clear();
}

//------------------------------------------------------------------------------
// 대입 연산자.
//------------------------------------------------------------------------------
Image& Image::operator=(const Image& image) {
  if (this == &image)
    return *this;

  if (image.empty()) {
    clear();
  }
  else {
    h_ = image.h_;
    w_ = image.w_;
    c_ = image.c_;
    if (!image.empty()) {
      pdata_.reset(new uint8_t[size()], std::default_delete<uint8_t[]>());
      for (decltype(size()) i=0; i<size(); i++)
        pdata_.get()[i] = image.pdata_.get()[i];
    }
  }
  return *this;
}

//------------------------------------------------------------------------------
// 이동 연산자.
//------------------------------------------------------------------------------
Image& Image::operator=(Image&& image) {
  if (this == &image)
    return *this;
  swap(image);
  image.clear();
  return *this;
}

//------------------------------------------------------------------------------
// 입력된 (x, y, z) 포지션이 valid 위치인지를 체크.
//------------------------------------------------------------------------------
bool Image::check(int x, int y, int z) const {
  if (x < 0) return false;
  if (y < 0) return false;
  if (z < 0) return false;
  if (x >= h_) return false;
  if (y >= w_) return false;
  if (z >= c_) return false;
  return true;
}

//------------------------------------------------------------------------------
// 입력된 (x, y, z) 포지션을 1차원 index 값으로 변경하여 반환.
//------------------------------------------------------------------------------
size_t Image::offset(int x, int y, int z) const {
  assert(check(x, y, z));
  assert(!empty());
  // use [h, w, c] format.
  auto pos = z + (y * c_) + (x * c_ * w_);
  assert(pos < size());
  return static_cast<size_t>(pos);
}

//------------------------------------------------------------------------------
// 이미지가 비어있는지 여부
//------------------------------------------------------------------------------
bool Image::empty() const {
  if (size() == 0) {
    assert(!pdata_);
    return true;
  }
  return false;
}

//------------------------------------------------------------------------------
// 이미지 크기를 반환.
//------------------------------------------------------------------------------
size_t Image::size() const {
  return static_cast<size_t>(h_) * w_ * c_;
}

//------------------------------------------------------------------------------
// 이미지 초기화
//------------------------------------------------------------------------------
void Image::clear() {
  w_ = h_ = c_ = 0;
  pdata_.reset();
}

//------------------------------------------------------------------------------
// (x, y, z) 위치의 pixel 값 반환
//------------------------------------------------------------------------------
uint8_t Image::pixel(int x, int y, int z) const {
  auto pos = offset(x, y, z);
  return pdata_.get()[pos];
}

//------------------------------------------------------------------------------
// (x, y, z) 위치의 pixel 참조 반환
//------------------------------------------------------------------------------
uint8_t& Image::pixel(int x, int y, int z) {
  auto pos = offset(x, y, z);
  return pdata_.get()[pos];
}

//------------------------------------------------------------------------------
// (x, y, z) 위치가 정상이면 pixel 반환. 아닌 경우 0 반환.
//------------------------------------------------------------------------------
uint8_t Image::pixelOrZero(int x, int y, int z) const {
  if (check(x, y, z))
    return pixel(x, y, z);
  return 0;
}

//------------------------------------------------------------------------------
// (x, y, z) 위치에 pixel 값을 누적. 255 보다 큰 경우 255 설정.
//------------------------------------------------------------------------------
void Image::addPixel(int x, int y, int z, uint8_t pxl) {
  // prohibit overflow
  int v = std::numeric_limits<uint8_t>::max() - pixel(x, y, z);
  v = std::min(v, static_cast<int>(pxl));
  pixel(x, y, z) += static_cast<uint8_t>(v);
}

//------------------------------------------------------------------------------
// (x, y, z) 위치에 pixel 값을 뺀다. 0보다 작은 경우 0으로 설정.
//------------------------------------------------------------------------------
void Image::subPixel(int x, int y, int z, uint8_t pxl) {
  if (pixel(x, y, z) < pxl)
    setPixel(x, y, z, 0);
  else
    pixel(x, y, z) -= pxl;
}

//------------------------------------------------------------------------------
// (x, y, z) 위치에 pixel 설정.
//------------------------------------------------------------------------------
void Image::setPixel(int x, int y, int z, uint8_t pxl) {
  pixel(x, y, z) = pxl;
}

//------------------------------------------------------------------------------
// 이미지를 white 이미지로 변경
//------------------------------------------------------------------------------
void Image::setWhite() {
  for (decltype(size()) i=0; i<size(); i++)
    pdata_.get()[i] = 0xFF;
}

//------------------------------------------------------------------------------
// 이미지를 black 이미지로 변경
//------------------------------------------------------------------------------
void Image::setBlack() {
  for (decltype(size()) i=0; i<size(); i++)
    pdata_.get()[i] = 0x0;
}

//------------------------------------------------------------------------------
// 두 이미지를 swap
//------------------------------------------------------------------------------
void Image::swap(Image& image) {
  if (this == &image)
    return;
  std::swap(h_, image.h_);
  std::swap(w_, image.w_);
  std::swap(c_, image.c_);
  pdata_.swap(image.pdata_);
}

//------------------------------------------------------------------------------
// 현재 이미지를 복사하여 반환
//------------------------------------------------------------------------------
Image Image::copy() const {
  return Image(*this);
}

//------------------------------------------------------------------------------
// 두 이미지가 동일한 크기인지를 확인.
//------------------------------------------------------------------------------
bool Image::isSameSize(const Image& image) const {
  if ((h_ == image.h()) && (w_ == image.w()) && (c_ == image.c())) {
    return true;
  }
  return false;
}

//------------------------------------------------------------------------------
// 새로운 크기로 이미지를 변환한다.
//------------------------------------------------------------------------------
void Image::resize(int new_h, int new_w) {
  assert(new_h > 0);
  assert(new_w > 0);
  if ((new_h == h_) && (new_w == w_))
    return;
  Image image(new_h, new_w, c_);

  ::stbir_resize_uint8(pdata_.get(), w_, h_, w_*c_,
                       image.pdata_.get(), new_w, new_h, new_w*c_, c_);
  swap(image);
}

//------------------------------------------------------------------------------
// 현재 이미지를 target 이미지에 resize 하여 전송. target 크기는 고정.
//------------------------------------------------------------------------------
bool Image::resizeTo(Image* target) const {
  if ((!target) || target->empty()) {
    return false;
  }
  auto h = target->h();
  auto w = target->w();
  auto c = target->c();

  if (c != c_)
    return false;
  assert(target->pdata_);
  ::stbir_resize_uint8(pdata_.get(), w_, h_, w_*c_,
                       target->pdata_.get(), w, h, w*c, c);
  return true;
}

//------------------------------------------------------------------------------
// height 크기를 변경. (resize)
//------------------------------------------------------------------------------
void Image::resizeHeight(int new_h) {
  assert(new_h > 0);
  if (new_h == h_)
    return;
  if (empty())
    return;
  Image image(new_h, w_, c_);
  stbir_resize_uint8(pdata_.get(), w_, h_, w_*c_,
                     image.pdata_.get(), w_, new_h, w_*c_, c_);
  swap(image);
}

//------------------------------------------------------------------------------
// width 크기를 변경. (resize)
//------------------------------------------------------------------------------
void Image::resizeWidth(int new_w) {
  assert(new_w > 0);
  if (new_w == w_)
    return;
  if (empty())
    return;
  Image image(h_, new_w, c_);
  stbir_resize_uint8(pdata_.get(), w_, h_, w_*c_,
                     image.pdata_.get(), new_w, h_, new_w*c_, c_);
  swap(image);
}

//------------------------------------------------------------------------------
// h와 hw중 작은 쪽을 기준으로 new_size 크기로 변경한다. (비율 고정)
//------------------------------------------------------------------------------
void Image::resizeOnSmallerSide(int new_size) {
  auto w = w_;
  auto h = h_;
  if (w < h) {
    h = (h * new_size) / w;
    w = new_size;
  }
  else {
    w = (w * new_size) / h;
    h = new_size;
  }
  if (h == h_ && w == w_)
    return;
  resize(h, w);
}

//------------------------------------------------------------------------------
// w와 h 중 큰 쪽을 기준으로 new_size 크기로 변경한다. (비율 고정)
//------------------------------------------------------------------------------
void Image::resizeOnLargerSide(int new_size) {
  auto h = h_;
  auto w = w_;
  if (w < h) {
    w = (w * new_size) / h;
    h = new_size;
  }
  else {
    h = (h * new_size) / w;
    w = new_size;
  }
  if (h == h_ && w == w_)
    return;
  resize(h, w);
}

//------------------------------------------------------------------------------
// 특정 channel 의 layer 만을 추출하여 (h, w, 1) 크기의 이미지를 만들어 반환.
//------------------------------------------------------------------------------
Image Image::layer(int z) const {
  if (z < 0 || z >= c_)
    return Image();
  Image image(h_, w_, 1);
  for (int x=0; x<h_; x++) {
    for (int y=0; y<w_; y++) {
      image.pixel(x, y, 0) = pixel(x, y, z);
    }
  }
  return image;
}

//------------------------------------------------------------------------------
// channel layer 별로 추출하여 (h, w, 1) 크기의 이미지 벡터를 만들어 반환.
//------------------------------------------------------------------------------
std::vector<Image> Image::layers() const {
  std::vector<Image> images;
  for (int z=0; z<c_; z++)
    images.push_back(layer(z));
  return images;
}

//------------------------------------------------------------------------------
// 입력된 image를 현재 이미지 (x, y) 지점을 시작으로 하여 데이터 복사.
// 현재 이미지에다가 image 를 도장 찍는다고 생각하면 쉽다.
// 현재 이미지로부터 범위가 넘어가거나 모자란 것은 대응하지 않는다.
// channel 크기가 다를 경우 현재 이미지를 기준으로 한다.
//------------------------------------------------------------------------------
void Image::stamp(const Image& img, int x, int y) {
  for(int h=x; h<std::min(h_, img.h()+x); h++) {
    for(int w=y; w<std::min(w_, img.w()+y); w++) {
      for (int z=0; z<c(); z++) {
        if (z >= img.c())
          break;
        pixel(h, w, z) = img.pixel(h-x, w-y, z);
      }
    }
  }
}

//------------------------------------------------------------------------------
// 이미지 border 를 추가. 색상은 pixel 값으로 추가한다. (모든 채널에 적용)
//------------------------------------------------------------------------------
void Image::addBorder(int h_border, int w_border, uint8_t pxl) {
  if (h_border < 0 || w_border < 0)
    return;
  // 현재 이미지보다 boader 크기만큼 크고, pxl 로 채워진 임시 이미지 생성
  auto h2 = h_border * 2;
  auto w2 = w_border * 2;
  Image image(h_ + h2, w_ + w2, c_, pxl);
  image.stamp(*this, h_border, w_border);
  swap(image);
}

//------------------------------------------------------------------------------
// 이미지 border 를 추가. 색상은 pixel 값으로 추가한다. (모든 채널에 적용)
//------------------------------------------------------------------------------
void Image::addBoxBorder(int border, uint8_t pxl) {
  return addBorder(border, border, pxl);
}

//------------------------------------------------------------------------------
// 이미지 crop. (h, w)는 crop 크기. (x, y)는 중심 위치.
//------------------------------------------------------------------------------
//      w
// -----------
// |         |
// |    @    | h  (@ 위치가 (x, y) 좌표 위치)
// |         |
// -----------
//------------------------------------------------------------------------------
bool Image::crop(int h, int w, int x, int y) {
  if (empty()) return false;
  if (x < 0 || x >= h_) return false;
  if (y < 0 || y >= w_) return false;

  auto sx = x - h / 2;
  auto sy = y - w / 2;
  auto ex = sx + h;
  auto ey = sy + w;
  if (sx < 0) sx = 0;
  if (sy < 0) sy = 0;
  if (ex > h_) ex = h_;
  if (ey > w_) ey = w_;
  if ((sx >= ex) || (sy >= ey))
    return false;

  Image image(h, w, c_);
  for(int h=sx; h<ex; h++) {
    for(int w=sy; w<ey; w++) {
      for(int c=0; c<c_; c++) {
        image.pixel(h-sx, w-sy, c) = pixel(h, w, c);
      }
    }
  }
  swap(image);
  return true;
}

//------------------------------------------------------------------------------
// 이미지 center 를 중심으로 crop.
//------------------------------------------------------------------------------
bool Image::centerCrop(int h, int w) {
  auto x = h_ / 2;
  auto y = w_ / 2;
  return crop(h, w, x, y);
}

//------------------------------------------------------------------------------
// 이미지 center 를 중심으로 crop을 하되 절대값이 아닌 ratio 로 조절.
//------------------------------------------------------------------------------
bool Image::centerCropWithRatio(float h_ratio, float w_ratio) {
  if (h_ratio <= 0.0f) return false;
  if (w_ratio <= 0.0f) return false;
  if (h_ratio > 1.0f) h_ratio = 1.0f;
  if (w_ratio > 1.0f) w_ratio = 1.0f;

  int h = static_cast<int>(h_ratio * h_);
  int w = static_cast<int>(w_ratio * w_);
  if (h == h_ && w == w_)
    return true;
  return centerCrop(h, w);
}

//------------------------------------------------------------------------------
// 이미지 center 를 중심으로 정사각 crop을 하되 ratio 로 조절.
//------------------------------------------------------------------------------
bool Image::centerSquaredCrop(float ratio) {
  int h, w;
  if (w_ < h_) {
    w = static_cast<int>(ratio * w_);
    h = w;
  }
  else {
    h = static_cast<int>(ratio * h_);
    w = h;
  }
  return centerCrop(h, w);
}

//------------------------------------------------------------------------------
// 가로/세로 대비 비율을 반환함. 정상 이미지인 경우 1.0f 보다 항상 크거나 같다.
//------------------------------------------------------------------------------
float Image::aspectRatio() const {
  if (w_ < h_ && w_ > 0)
    return static_cast<float>(h_) / w_;
  else if (h_ < w_ && h_ > 0)
    return static_cast<float>(w_) / h_;
  return 0.0f;
}

//------------------------------------------------------------------------------
// 이미지를 binary 로 변경하는 함수로 내부에서만 사용함.
//------------------------------------------------------------------------------
static void write_func(void* context, void* data, int size) {
  assert(context);
  assert(data);
  auto tgt = reinterpret_cast<uint8_t*>(context);
  auto src = reinterpret_cast<uint8_t*>(data);
  for (auto i=0; i<size; i++) {
    tgt[i] = src[i];
  }
}

//------------------------------------------------------------------------------
// png 포맷으로 이미지 저장. (특정 파일에 저장한다.)
//------------------------------------------------------------------------------
bool Image::savePng(const std::string& filename) const {
  auto f = filename.c_str();
  return ::stbi_write_png(f, w_, h_, c_, pdata_.get(), w_*c_);
}

//------------------------------------------------------------------------------
// png 포맷으로 이미지 저장. (buffer에 저장)
//------------------------------------------------------------------------------
bool Image::savePng(std::vector<uint8_t>* buffer) const {
  assert(buffer);
  if ((!buffer) || empty())
    return false;
  buffer->resize(size());
  return ::stbi_write_png_to_func(write_func, buffer->data(),
                                  w_, h_, c_, pdata_.get(), w_*c_);
}

//------------------------------------------------------------------------------
// png 포맷으로 이미지 저장. (raw buffer에 저장)
//------------------------------------------------------------------------------
bool Image::savePng(uint8_t* buffer, int buf_size) const {
  assert(buffer);
  if (buf_size < static_cast<int>(size()))
    return false;
  return ::stbi_write_png_to_func(write_func, buffer,
                                  w_, h_, c_, pdata_.get(), w_*c_);
}

bool Image::saveJpg(const std::string& filename) const {
  auto f = filename.c_str();
  static const int quality = 100;
  return ::stbi_write_jpg(f, w_, h_, c_, pdata_.get(), quality);
}
bool Image::saveJpg(std::vector<uint8_t>* buffer) const {
  static const int quality = 100;
  assert(buffer);
  if ((!buffer) || empty())
    return false;
  buffer->resize(size());
  return ::stbi_write_jpg_to_func(write_func, buffer->data(),
                                  w_, h_, c_, pdata_.get(), quality);
}
bool Image::saveJpg(uint8_t* buffer, int buf_size) const {
  static const int quality = 100;
  if (buf_size < static_cast<int>(size()))
    return false;
  return ::stbi_write_jpg_to_func(write_func, buffer,
                                  w_, h_, c_, pdata_.get(), quality);
}


//------------------------------------------------------------------------------
// 이미지를 파일에서 load
//------------------------------------------------------------------------------
bool Image::load(const std::string& filename, int num_channel) {
  auto f = filename.c_str();
  int h, w, c;
  uint8_t* mem = ::stbi_load(f, &w, &h, &c, num_channel);
  if (!mem)   return false;
  if (h <= 0) return false;
  if (w <= 0) return false;
  if (c <= 0) return false;

  if (num_channel != c) {
    c = num_channel;
  }
  std::shared_ptr<uint8_t> data(mem, std::default_delete<uint8_t[]>());
  Image image(h, w, c, data);
  swap(image);
  return true;
}

//------------------------------------------------------------------------------
// 이미지를 vector 데이터에서 로드. data는 binary 포멧임.
//------------------------------------------------------------------------------
bool Image::load(const std::vector<uint8_t>& raw, int num_channel) {
  assert(!(raw.empty()));
  int h, w, c;
  uint8_t* mem = ::stbi_load_from_memory(raw.data(), raw.size(),
                                         &w, &h, &c, num_channel);
  if (!mem) return false;
  if (h <= 0) return false;
  if (w <= 0) return false;
  if (c <= 0) return false;
  if (num_channel != c)
    c = num_channel;
  std::shared_ptr<uint8_t> p(mem, std::default_delete<uint8_t[]>());
  Image image(h, w, c, p);
  swap(image);
  return true;
}

//------------------------------------------------------------------------------
// 이미지를 raw pointer 데이터에서 로드. data는 binary 포멧임.
//------------------------------------------------------------------------------
bool Image::load(const uint8_t* raw, size_t size, int num_channel) {
  assert(raw);
  int h, w, c;
  int sz = static_cast<int>(size);
  auto mem = ::stbi_load_from_memory(raw, sz,  &w, &h, &c, num_channel);
  if (!mem) return false;
  if (h <= 0) return false;
  if (w <= 0) return false;
  if (c <= 0) return false;
  if (num_channel != c)
    c = num_channel;
  std::shared_ptr<uint8_t> p(mem, std::default_delete<uint8_t[]>());
  Image image(h, w, c, p);
  swap(image);
  return true;
}

//------------------------------------------------------------------------------
// 이미지 정보를 문자열로 출력.
//------------------------------------------------------------------------------
std::string Image::str() const {
  std::stringstream ss;
  ss << "HWC[" << h() << ", " << w() << ", " << c();
  ss << "] (size:" << size() << ")";
  return ss.str();
}

//------------------------------------------------------------------------------
// 이미지가 동일한지 여부를 판단.
//------------------------------------------------------------------------------
bool Image::operator==(const Image& image) {
  if (h_ != image.h_) return false;
  if (w_ != image.w_) return false;
  if (c_ != image.c_) return false;
  if (pdata_ != image.pdata_) return false;
  return true;
}

//------------------------------------------------------------------------------
// 이미지가 동일하지 않은지 여부를 판단.
//------------------------------------------------------------------------------
bool Image::operator!=(const Image& image) {
  return !operator==(image);
}

}  // namespace sas
