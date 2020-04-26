/*
 * MIT License
 * 
 * Copyright (c) 2020 Size Zheng

 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

#ifndef BOOST_ARITH_H
#define BOOST_ARITH_H

#include <vector>

namespace Boost {

namespace Arith {

class Bounds {
 private:
   std::vector<std::pair<size_t, size_t>> bounds;
 public:
   Bounds() {}

   Bounds(const Bounds &other) : bounds(other.bounds) {}

   Bounds(const Bounds &&other) : bounds(other.bounds) {}
};


template<typename T>
class Matrix {
 public:
  Matrix(int height, int width) : width_(width), height_(height) {
    ptr = new T[width * height];
  }
  ~Matrix() {
    if (ptr != nullptr) {
      delete []ptr;
    }
  }

  int height() const {
    return height_;
  }

  int width() const {
    return width_;
  }

  T *operator[](int id) {
    ASSERT(id < height_) << "index out of height range: " << id << " vs. " << height_ << "\n";
    return (ptr + id * width_);
  }

  void swap_row(int i, int j);

  void swap_col(int i, int j);

  void scale_row(int i, T factor);

  void scale_col(int j, T factor);

  void add_row(int i, int j, T factor);

  void add_col(int i, int j, T factor);

  void row_transform(int i, int j, T s, T t, T f, T g);

  void col_transform(int i, int j, T s, T t, T f, T g);

 private:
  T *ptr;
  int width_, height_;
};


bool divisible(int a, int b);


int ext_euclidean(int a, int b, int &x, int &y);


int smith_normalize(Matrix<int> &trans, Matrix<int> &U, Matrix<int> &V);


}  // namespace Arith

}  // namespace Boost


#endif  // BOOST_ARITH_H