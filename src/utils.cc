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

#include <sstream>
#include <vector>

#include "debug.h"
#include "IR.h"
#include "utils.h"


namespace Boost {

using namespace Internal;

namespace Utils {

Expr add(const Expr &a, const Expr &b) {
  return Binary::make(a.type(), BinaryOpType::Add, a, b);
}


Expr operator+(const Expr &a, const Expr &b) {
  return add(a, b);
}


Expr sub(const Expr &a, const Expr &b) {
  return Binary::make(a.type(), BinaryOpType::Sub, a, b);
}


Expr operator-(const Expr &a, const Expr &b) {
  return sub(a, b);
}


Expr neg(const Expr &a) {
  return Unary::make(a.type(), UnaryOpType::Neg, a);
}


Expr operator-(const Expr &a) {
  return neg(a);
}


Expr mul(const Expr &a, const Expr &b) {
  return Binary::make(a.type(), BinaryOpType::Mul, a, b);
}


Expr operator*(const Expr &a, const Expr &b) {
  return mul(a, b);
}


Expr div(const Expr &a, const Expr &b) {
  return Binary::make(a.type(), BinaryOpType::Div, a, b);
}


Expr operator/(const Expr &a, const Expr &b) {
  return div(a, b);
}


Expr floordiv(const Expr &a, const Expr &b) {
  return Binary::make(a.type(), BinaryOpType::FloorDiv, a, b);
}


Expr mod(const Expr &a, const Expr &b) {
  return Binary::make(a.type(), BinaryOpType::Mod, a, b);
}


Expr floormod(const Expr &a, const Expr &b) {
  return Binary::make(a.type(), BinaryOpType::FloorMod, a, b);
}


std::string NameGenerator::unique_name(const std::string &name_hint) {
  std::ostringstream oss;
  oss << name_hint;
  if (name_map_.count(name_hint) != 0) {
    name_map_[name_hint]++; 
  } else {
    name_map_[name_hint] = 0;
  }
  oss << name_map_[name_hint];
  return oss.str();
}


std::string NameGenerator::operator()(const std::string &name_hint) {
  return unique_name(name_hint);
}


ExtRange ExtRange::floor_div(int factor) {
  ExtRange ret;
  if (!this->left_inf) {
    ret.left = Binary::make(this->left.type(), BinaryOpType::FloorDiv, this->left, factor);
    ret.left_inf = false;
  }
  if (!this->right_inf) {
    // ceil div
    ret.right = Binary::make(
      this->right.type(), BinaryOpType::FloorDiv,
      Binary::make(this->right.type(), BinaryOpType::Add, this->right, factor - 1), factor);
    ret.right_inf = false;
  }
  return ret;
}


ExtRange ExtRange::floor_mod(int factor) {
  return ExtRange(0, factor, false, false);
}


void RangeInference::visit(Ref<const Index> op) {
  range_map[op->name] = scope_.back();
}


void RangeInference::visit(Ref<const Binary> op) {
  if (op->op_type == BinaryOpType::Add) {
    Ref<const IntImm> a_as_int = op->a.as<IntImm>();
    Ref<const IntImm> b_as_int = op->b.as<IntImm>();
    ExtRange range = scope_.back();
    if (a_as_int.defined()) {
      int bias = (int)a_as_int->value();
      if (!range.left_inf) {
        range.left = range.left - bias;
      }
      if (!range.right_inf) {
        range.right = range.right - bias;
      }
      scope_.push_back(range);
      (op->b).visit_expr(this);
      scope_.pop_back();
    } else if (b_as_int.defined()) {
      int bias = (int)b_as_int->value();
      if (!range.left_inf) {
        range.left = range.left - bias;
      }
      if (!range.right_inf) {
        range.right = range.right - bias;
      }
      scope_.push_back(range);
      (op->a).visit_expr(this);
      scope_.pop_back();
    }
  } else if (op->op_type == BinaryOpType::Sub) {
    Ref<const IntImm> a_as_int = op->a.as<IntImm>();
    Ref<const IntImm> b_as_int = op->b.as<IntImm>();
    ExtRange range = scope_.back();
    if (a_as_int.defined()) {
      int bias = (int)a_as_int->value();
      ExtRangeType range_type = range.range_type();
      if (range_type == ExtRangeType::LCRC) {
        range.left = bias - scope_.back().right;
        range.right = bias - scope_.back().left;
      } else if (range_type == ExtRangeType::LCRO) {
        range.left_inf = true;
        range.right_inf = false;
        range.right = bias - range.left;
      } else if (range_type == ExtRangeType::LORC) {
        range.left_inf = false;
        range.right_inf = true;
        range.left = bias - range.right;
      } else {
        // nothing to do
      }
      scope_.push_back(range);
      (op->b).visit_expr(this);
      scope_.pop_back();
    } else if (b_as_int.defined()) {
      int bias = (int)b_as_int->value();
      if (!range.left_inf) {
        range.left = add(range.left, bias);
      }
      if (!range.right_inf) {
        range.right = add(range.right, bias);
      }
      scope_.push_back(range);
      (op->a).visit_expr(this);
      scope_.pop_back();
    }
  } else if (op->op_type == BinaryOpType::Mul) {
    Ref<const IntImm> a_as_int = op->a.as<IntImm>();
    Ref<const IntImm> b_as_int = op->b.as<IntImm>();
    ExtRange range = scope_.back();
    if (a_as_int.defined()) {
      int bias = (int)a_as_int->value();
      if (bias == 0) {
        range.left = 0;
        range.left_inf = false;
        range.right = 1;
        range.right_inf = false;
      } else if (bias > 0) {
        ExtRangeType range_type = range.range_type();
        if (range_type == ExtRangeType::LCRC) {
          range.left = floordiv(range.left, bias);
          range.right = floordiv(add(range.right, bias - 1), bias);
        } else if (range_type == ExtRangeType::LCRO) {
          range.left = floordiv(range.left, bias);
        } else if (range_type == ExtRangeType::LORC) {
          range.right = floordiv(add(range.right, bias - 1), bias);
        } else {
          // nothing to do
        }
      } else {
        ExtRangeType range_type = range.range_type();
        if (range_type == ExtRangeType::LCRC) {
          range.left = mul(sub(floordiv(add(scope_.back().right, -bias-1), -bias), 1), -1);
          range.right = mul(sub(floordiv(scope_.back().left, -bias), 1), -1);
        } else if (range_type == ExtRangeType::LCRO) {
          range.left_inf = true;
          range.right_inf = false;
          range.right = mul(sub(floordiv(range.left, -bias), 1), -1);
        } else if (range_type == ExtRangeType::LORC) {
          range.left_inf = false;
          range.right_inf = true;
          range.left = mul(sub(floordiv(add(range.right, -bias-1), -bias), 1), -1);
        } else {
          // nothing to do
        }
      }
      
      scope_.push_back(range);
      (op->b).visit_expr(this);
      scope_.pop_back();
    } else if (b_as_int.defined()) {
      int bias = (int)b_as_int->value();
      if (bias == 0) {
        range.left = 0;
        range.left_inf = false;
        range.right = 1;
        range.right_inf = false;
      } else if (bias > 0) {
        ExtRangeType range_type = range.range_type();
        if (range_type == ExtRangeType::LCRC) {
          range.left = floordiv(range.left, bias);
          range.right = floordiv(add(range.right, bias - 1), bias);
        } else if (range_type == ExtRangeType::LCRO) {
          range.left = floordiv(range.left, bias);
        } else if (range_type == ExtRangeType::LORC) {
          range.right = floordiv(add(range.right, bias - 1), bias);
        } else {
          // nothing to do
        }
      } else {
        ExtRangeType range_type = range.range_type();
        if (range_type == ExtRangeType::LCRC) {
          range.left = mul(sub(floordiv(add(scope_.back().right, -bias-1), -bias), 1), -1);
          range.right = mul(sub(floordiv(scope_.back().left, -bias), 1), -1);
        } else if (range_type == ExtRangeType::LCRO) {
          range.left_inf = true;
          range.right_inf = false;
          range.right = mul(sub(floordiv(range.left, -bias), 1), -1);
        } else if (range_type == ExtRangeType::LORC) {
          range.left_inf = false;
          range.right_inf = true;
          range.left = mul(sub(floordiv(add(range.right, -bias-1), -bias), 1), -1);
        } else {
          // nothing to do
        }
      }
      
      scope_.push_back(range);
      (op->a).visit_expr(this);
      scope_.pop_back();
    }
  }
}


void RangeInference::visit(Ref<const Unary> op) {
  if (op->op_type == UnaryOpType::Neg) {
    ExtRange range = scope_.back();
    ExtRangeType range_type = range.range_type();
    switch (range_type)
    {
    case ExtRangeType::LCRC:
      {Expr tmp = range.left;
      range.left = neg(range.right);
      range.right = neg(tmp);}
      break;
    case ExtRangeType::LCRO:
      {range.right = neg(range.left);
      range.left_inf = true;}
      break;
    case ExtRangeType::LORC:
      {range.left = neg(range.right);
      range.right_inf = true;}
      break;
    default:
      break;
    }
    scope_.push_back(range);
    (op->a).visit_expr(this);
    scope_.pop_back();
  }
}


std::vector<Expr> relax_matrix_array_product(Arith::Matrix<int> &m, std::vector<Expr> &v) {
  std::vector<Expr> res;
  int rows = m.height();
  int cols = m.width();
  ASSERT(cols <= (int)v.size()) << "Matrix-Array-Mult shape mismatch.\n";
  for (int i = 0; i < rows; ++i) {
    Expr tmp = 0;
    for (int j = 0; j < cols; ++j) {
      if (m[i][j] != 0) {
        tmp = Binary::make(
          tmp.type(),
          BinaryOpType::Add,
          tmp,
          Binary::make(tmp.type(), BinaryOpType::Mul, v[j], m[i][j])
        );
      }
    }
    res.push_back(tmp);
  }
  return res;
}

}  // namespace Utils

}  // namespace Boost