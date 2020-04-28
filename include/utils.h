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
#include <iostream>
#include <functional>

#include "IR.h"
#include "arith.h"
#include "IRVisitor.h"
#include "IRMutator.h"
#include "IRPrinter.h"
#include "IRFunctor.h"

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


class ExprEqualByValue : public ExprFunctor<bool(const Expr&, const Expr&)> {
 public:
  #define CHECK_TYPE(T)                       \
    Ref<const T> other_op = other.as<T>();    \
    if (!other_op.defined()) {                \
      return false;                           \
    }                                         \
    if (op->type() != other_op->type()) {     \
      return false;                           \
    }
  bool visit(Ref<const IntImm> op, const Expr& other) override {
    CHECK_TYPE(IntImm)
    return op->value() == other_op->value();
  }

  bool visit(Ref<const UIntImm> op, const Expr& other) override {
    CHECK_TYPE(UIntImm)
    return op->value() == other_op->value();
  }

  bool visit(Ref<const FloatImm> op, const Expr& other) override {
    CHECK_TYPE(FloatImm)
    return op->value() == other_op->value();
  }

  bool visit(Ref<const StringImm> op, const Expr& other) override {
    CHECK_TYPE(StringImm)
    return op->value() == other_op->value();
  }

  bool visit(Ref<const Unary> op, const Expr& other) override {
    CHECK_TYPE(Unary)
    return (op->op_type == other_op->op_type) && visit_expr(op->a, other_op->a);
  }

  bool visit(Ref<const Binary> op, const Expr& other) override {
    CHECK_TYPE(Binary)
    return ((op->op_type == other_op->op_type)
      && visit_expr(op->a, other_op->a) && visit_expr(op->b, other_op->b));
  }

  bool visit(Ref<const Select> op, const Expr& other) override {
    CHECK_TYPE(Select)
    return (visit_expr(op->cond, other_op->cond)
      && visit_expr(op->true_value, other_op->true_value)
      && visit_expr(op->false_value, other_op->false_value));
  }

  bool visit(Ref<const Compare> op, const Expr& other) override {
    CHECK_TYPE(Compare)
    return ((op->op_type == other_op->op_type)
      && visit_expr(op->a, other_op->a) && visit_expr(op->b, other_op->b));
  }

  bool visit(Ref<const Call> op, const Expr& other) override {
    CHECK_TYPE(Call)
    bool ret = true;
    int num_args = (int)op->args.size();
    ret = ret && (num_args == (int)other_op->args.size());
    for (int i = 0; i < num_args; ++i) {
      ret = ret && visit_expr(op->args[i], other_op->args[i]);
    }
    ret = (ret && (op->func_name == other_op->func_name)
          && (op->call_type == other_op->call_type));
    return ret;
  }

  bool visit(Ref<const Var> op, const Expr& other) override {
    CHECK_TYPE(Var)
    bool ret = op->name == other_op->name;
    int num_args = (int)op->args.size();
    ret = ret && (num_args == (int)other_op->args.size());
    for (int i = 0; i < num_args; ++i) {
      ret = ret && visit_expr(op->args[i], other_op->args[i]);
    }
    int dim = (int)op->shape.size();
    ret = ret && (dim == (int)other_op->shape.size());
    for (int i = 0; i < dim; ++i) {
      ret = ret && (op->shape[i] == other_op->shape[i]);
    }
    return ret;
  }

  bool visit(Ref<const Cast> op, const Expr& other) override {
    CHECK_TYPE(Cast)
    return ((op->new_type == other_op->new_type)
        && visit_expr(op->val, other_op->val));
  }

  bool visit(Ref<const Ramp> op, const Expr& other) override {
    CHECK_TYPE(Ramp)
    return (visit_expr(op->base, other_op->base)
      && (op->stride == other_op->stride) && (op->lanes == other_op->lanes));
  }

  bool visit(Ref<const Index> op, const Expr& other) override {
    CHECK_TYPE(Index)
    return ((op->name == other_op->name)
      && visit_expr(op->dom, other_op->dom)
      && (op->index_type == other_op->index_type));
  }

  bool visit(Ref<const Dom> op, const Expr& other) override {
    CHECK_TYPE(Dom)
    return (visit_expr(op->begin, other_op->begin)
          && visit_expr(op->extent, other_op->extent));
  }
};


class SubstituteIndexByName : public IRMutator {
 private:
  std::unordered_map<std::shared_ptr<const Index>, Expr> &vmap_;
 public:
  SubstituteIndexByName(
    std::unordered_map<std::shared_ptr<const Index>, Expr> &vmap) : vmap_(vmap) {}
  
  Expr substitute(const Expr &expr) {
    return mutate(expr);
  }

  Expr visit(Ref<const Index> op) override {
    for (auto kv : vmap_) {
      if (op->name == kv.first->name) {
        return kv.second;
      }
    }
    return op;
  }
};


class SubstituteIndex : public IRMutator {
 private:
  std::unordered_map<std::shared_ptr<const Index>, Expr> &vmap_;
 public:
  SubstituteIndex(
    std::unordered_map<std::shared_ptr<const Index>, Expr> &vmap) : vmap_(vmap) {}

  Expr substitute(const Expr &expr) {
    return mutate(expr);
  }

  Expr visit(Ref<const Index> op) override {
    for (auto kv : vmap_) {
      if (op == kv.first) {
        return kv.second;
      }
    }
    return op;
  }
};


Expr substitute_index(const Expr &expr,
  std::unordered_map<std::shared_ptr<const Index>, Expr> &vmap);


Expr substitute_index_by_name(const Expr &expr,
  std::unordered_map<std::shared_ptr<const Index>, Expr> &vmap);


class IndexCollector : public IRVisitor {
 public:
  IndexCollector(std::function<bool(Ref<const Index>)> func) : func_(func) {}

  void collect(const Expr& expr, std::vector<Ref<const Index>> &results) {
    results_ = &results;
    expr.visit_expr(this);
  }

  void collect(const Stmt& stmt, std::vector<Ref<const Index>> &results) {
    results_ = &results;
    stmt.visit_stmt(this);
  }

  void collect(const Group& group, std::vector<Ref<const Index>> &results) {
    results_ = &results;
    group.visit_group(this);
  }

  void visit(Ref<const Index> op) override {
    if (func_(op)) {
      (*results_).push_back(op);
    }
  }
 private:
  std::function<bool(Ref<const Index>)> func_;
  std::vector<Ref<const Index>> *results_ = nullptr;
};


}  // namespace Utils

}  // namespace Boost


#endif  // BOOST_UTILS_H