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

#ifndef BOOST_UTILS_H
#define BOOST_UTILS_H

#include <unordered_map>
#include <string>
#include <vector>

#include "IR.h"
#include "IRVisitor.h"

namespace Boost {

using namespace Internal;

namespace Utils {

class NameGenerator {
 private:
  std::unordered_map<std::string, int> name_map_;
 public:
  NameGenerator() = default;

  std::string unique_name(const std::string &name_hint);

  std::string operator()(const std::string &name_hint);
};


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


std::vector<Expr> relax_matrix_array_product(Arith::Matrix<int> &m, std::vector<Expr> &v);

}  // namespace Utils

}  // namespace Boost


#endif  // BOOST_UTILS_H