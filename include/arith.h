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
#include <unordered_map>

#include "IR.h"
#include "IRVisitor.h"

namespace Boost {

using namespace Internal;

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


std::vector<Expr> relax_matrix_array_product(Matrix<int> &m, std::vector<Expr> &v);


Expr add(const Expr &a, const Expr &b);


Expr operator+(const Expr &a, const Expr &b);


Expr sub(const Expr &a, const Expr &b);


Expr operator-(const Expr &a, const Expr &b);


Expr neg(const Expr &a);


Expr operator-(const Expr &a);


Expr mul(const Expr &a, const Expr &b);


Expr operator*(const Expr &a, const Expr &b);


Expr div(const Expr &a, const Expr &b);


Expr operator/(const Expr &a, const Expr &b);


Expr floordiv(const Expr &a, const Expr &b);


Expr mod(const Expr &a, const Expr &b);


Expr floormod(const Expr &a, const Expr &b);


Expr eq(const Expr &a, const Expr &b);


Expr ne(const Expr &a, const Expr &b);


Expr gt(const Expr &a, const Expr &b);


Expr ge(const Expr &a, const Expr &b);


Expr lt(const Expr &a, const Expr &b);


Expr le(const Expr &a, const Expr &b);


enum class ExtRangeType : uint8_t {
  LORC,  // left open right close
  LORO,  // left open right open
  LCRO,  // left close right open
  LCRC   // left close right close
};


class ExtRange {
 public:
  Expr left;
  Expr right;
  bool left_inf;
  bool right_inf;

  ExtRange() { left_inf = true; right_inf = true; }

  ExtRange(ExtRange &range) : left(range.left), right(range.right),
    left_inf(range.left_inf), right_inf(range.right_inf) {}

  ExtRange(ExtRange &&range) : left(std::move(range.left)), right(std::move(range.right)),
    left_inf(std::move(range.left_inf)), right_inf(std::move(range.right_inf)) {}

  ExtRange(const ExtRange &range) : left(range.left), right(range.right),
    left_inf(range.left_inf), right_inf(range.right_inf) {}

  ExtRange(const ExtRange &&range) : left(std::move(range.left)), right(std::move(range.right)),
    left_inf(std::move(range.left_inf)), right_inf(std::move(range.right_inf)) {}

  ExtRange(Expr l, Expr r, bool li, bool ri) : left(l), right(r), left_inf(li), right_inf(ri) {}

  ExtRange &operator=(ExtRange &range) {
    left = range.left;
    right = range.right;
    left_inf = range.left_inf;
    right_inf = range.right_inf;
    return *this;
  }

  ExtRange &operator=(ExtRange &&range) {
    left = std::move(range.left);
    right = std::move(range.right);
    left_inf = std::move(range.left_inf);
    right_inf = std::move(range.right_inf);
    return *this;
  }

  ExtRange &operator=(const ExtRange &range) {
    left = range.left;
    right = range.right;
    left_inf = range.left_inf;
    right_inf = range.right_inf;
    return *this;
  }

  ExtRange &operator=(const ExtRange &&range) {
    left = std::move(range.left);
    right = std::move(range.right);
    left_inf = std::move(range.left_inf);
    right_inf = std::move(range.right_inf);
    return *this;
  }

  ExtRange floor_div(int factor);

  ExtRange floor_mod(int factor);

  ExtRangeType range_type() {
    if (left_inf && right_inf) {
      return ExtRangeType::LORO;
    } else if (left_inf && !right_inf) {
      return ExtRangeType::LORC;
    } else if (!left_inf && !right_inf) {
      return ExtRangeType::LCRC;
    } else {
      return ExtRangeType::LCRO;
    }
  }
};


class RangeInference : public IRVisitor {
 private:
  std::vector<ExtRange> scope_;
 public:
  std::unordered_map<std::string, ExtRange> range_map;
  RangeInference(ExtRange init) { scope_.push_back(init); }

  void do_infer(const Expr &expr) {
    expr.visit_expr(this);
  }

 protected:
  // list of functions to override.
  void visit(Ref<const Index> op) override;

  // only for Neg
  void visit(Ref<const Unary> op) override;

  // only for Add/Sub/Mul
  void visit(Ref<const Binary> op) override;
};


}  // namespace Arith

}  // namespace Boost


#endif  // BOOST_ARITH_H