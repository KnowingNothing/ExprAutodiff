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

#ifndef BOOST_AUTODIFF_H
#define BOOST_AUTODIFF_H

#include <unordered_set>
#include <string>

#include "IR.h"
#include "utils.h"
#include "arith.h"
#include "IRPrinter.h"
#include "IRMutator.h"

namespace Boost {

using namespace Internal;

namespace Autodiff {

#define UNEXPECTED { LOG(ERROR) << "Unexpected visit of " << Expr(op) << "."; throw; }


class SubstituteContext {
 public:
  SubstituteContext() { bound_begin = -1; }
  std::vector<std::string> index_names;
  std::unordered_map<std::string, Ref<const Index>> index_map;
  int bound_begin;
  std::unordered_map<std::string, Arith::ExtRange> range_map;
  std::unordered_map<std::string, Expr> var2expr;
  // not found availabe map for Expr by structural equal
  // so use vector
  // TODO: use optimized container
  std::vector<std::pair<Expr, std::string>> expr2var;
  // std::unordered_map<Expr, std::string, StructuralHash, CheckExprEqual> expr2var;

  int find_bound(Expr &expr);

  std::string get_bound_name(Expr &expr);

  void add(std::string &name, Ref<const Index> index, Expr expr, Arith::ExtRange range);

  SubstituteContext copy() {
    SubstituteContext ret;
    for (auto name: index_names) {
      ret.index_names.push_back(name);
    }
    for (auto kv : index_map) {
      ret.index_map[kv.first] = kv.second;
    }
    ret.bound_begin = bound_begin;
    for (auto kv : range_map) {
      ret.range_map[kv.first] = kv.second;
    }
    for (auto kv : var2expr) {
      ret.var2expr[kv.first] = kv.second;
    }
    for (auto kv : expr2var) {
      ret.expr2var.push_back(std::make_pair(kv.first, kv.second));
    }
    return ret;
  }

  friend std::ostream &operator<<(std::ostream &out, SubstituteContext &context) {
    out << "Show Substitute Context";
    out << "\nindices:\n";
    for (auto name : context.index_names) {
      out << name << " ";
    }
    out << "\nvariable map:\n";
    for (auto kv : context.index_map) {
      out << kv.first << " = " << kv.second << "\n";
    }
    out << "\nrange map:\n";
    for (auto kv : context.range_map) {
      out << kv.first << " [" << kv.second.left << ", " << kv.second.right << "]\n";
    }
    out << "\nbindings:\n";
    for (auto kv : context.var2expr) {
      out << kv.first << " = " << kv.second << "\n";
    }
    out << "substitutions:\n";
    for (auto kv : context.expr2var) {
      out << kv.first << " -> " << kv.second << "\n";
    }
    return out;
  }
};


class EliminateIndexFloorDivAndMod : public ExprFunctor<Expr(const Expr&)> {
 public:
  Utils::NameGenerator &name_generator_;
  std::string &substitute_name_hint_;
  SubstituteContext &context_;
  EliminateIndexFloorDivAndMod(
    Utils::NameGenerator &name_generator, std::string &subname_hint, SubstituteContext &context) :
    name_generator_(name_generator), substitute_name_hint_(subname_hint), context_(context) {}

  Expr eliminate(const Expr &expr) {
    return visit_expr(expr);
  }

 protected:
  // list of functions to override.

  Expr visit(Ref<const IntImm> op) override {
    return op;
  }

  Expr visit(Ref<const UIntImm> op) override {
    return op;
  }

  Expr visit(Ref<const FloatImm> op) override {
    return op;
  }

  Expr visit(Ref<const StringImm> op) override UNEXPECTED

  Expr visit(Ref<const Unary> op) override {
    return Unary::make(op->type(), op->op_type, visit_expr(op->a));
  }

  Expr visit(Ref<const Binary> op) override;

  Expr visit(Ref<const Select> op) override {
    return Select::make(op->type(), visit_expr(op->cond),
              visit_expr(op->true_value), visit_expr(op->false_value));
  }

  Expr visit(Ref<const Compare> op) override {
    return Compare::make(op->type(), op->op_type, visit_expr(op->a), visit_expr(op->b));
  }

  Expr visit(Ref<const Call> op) override {
    std::vector<Expr> new_args;
    for (auto arg : op->args) {
      new_args.push_back(visit_expr(arg));
    }
    return Call::make(op->type(), new_args, op->func_name, op->call_type);
  }

  Expr visit(Ref<const Var> op) override {
    std::vector<Expr> new_args;
    for (auto arg : op->args) {
      new_args.push_back(visit_expr(arg));
    }
    return Var::make(op->type(), op->name, new_args, op->shape);
  }

  Expr visit(Ref<const Cast> op) override {
    return Cast::make(op->type(), op->new_type, visit_expr(op->val));
  }

  Expr visit(Ref<const Ramp> op) override {
    return Ramp::make(op->type(), visit_expr(op->base), op->stride, op->lanes);
  }

  Expr visit(Ref<const Index> op) override {
    return Index::make(op->type(), op->name, visit_expr(op->dom), op->index_type);
  }

  Expr visit(Ref<const Dom> op) override {
    return Dom::make(op->type(), visit_expr(op->begin), visit_expr(op->extent));
  }
};


// TODO: how do we handle complex type casting?
class ExtractIndexCoefficients : public ExprFunctor<void(const Expr&)> {
  using VType = int;
 public:
  ExtractIndexCoefficients(
    const std::string &const_tag) : const_tag_(const_tag) {
  }

  void get_coefficients(const Expr& expr, std::unordered_map<std::string, VType> &result) {
    result[const_tag_] = (VType)0;
    scope_.push_back(&result);
  }

 protected:
  void visit(Ref<const IntImm> op) override {
    (*(scope_.back()))[const_tag_] = (VType)op->value();
  }

  void visit(Ref<const UIntImm> op) override {
    (*(scope_.back()))[const_tag_] = (VType)op->value();
  }

  void visit(Ref<const FloatImm> op) override UNEXPECTED
  void visit(Ref<const StringImm> op) override UNEXPECTED
  void visit(Ref<const Unary> op) override;
  void visit(Ref<const Binary> op) override;
  void visit(Ref<const Select> op) override UNEXPECTED
  void visit(Ref<const Compare> op) override UNEXPECTED
  void visit(Ref<const Call> op) override UNEXPECTED
  void visit(Ref<const Var> op) override UNEXPECTED
  void visit(Ref<const Cast> op) override UNEXPECTED
  void visit(Ref<const Ramp> op) override UNEXPECTED
  void visit(Ref<const Index> op) override;
  void visit(Ref<const Dom> op) override UNEXPECTED
 private:
  std::vector<std::unordered_map<std::string, VType>*> scope_;
  std::string const_tag_;
};


class FloorDivModEntry {
 public:
  // var_name = first * factor + second
  int factor;
  std::string var_name;
  std::string first;
  std::string second;

  FloorDivModEntry() { first = ""; second = ""; }

  FloorDivModEntry(int f, std::string var, std::string fi, std::string se) :
    factor(f), var_name(var), first(fi), second(se) {}

  bool operator==(const FloorDivModEntry &other) const {
    return (other.factor == factor) && (other.var_name == var_name);
  }

  bool operator!=(const FloorDivModEntry &other) const {
    return !((*this) == other);
  }

  FloorDivModEntry merge(const FloorDivModEntry &other) const;
};


class FloorDivModEntryHash {
 public:
  size_t operator()(const FloorDivModEntry &entry) const {
    return std::hash<int>{}(entry.factor) + std::hash<std::string>{}(entry.var_name);
  }
};


void solve_floor_div_mod(const SubstituteContext &context,
  std::unordered_set<FloorDivModEntry, FloorDivModEntryHash> &s);


Expr solve_multi_bindings(SubstituteContext &context, std::vector<Expr> &bindings,
      std::unordered_set<std::string> &unused, std::vector<Expr> &conditions);


void solve_substitutions(SubstituteContext &context,
  std::unordered_map<std::string, std::vector<Expr>> &bindings,
  std::unordered_set<std::string> &unused,
  std::vector<Expr> &conditions, std::unordered_map<std::string, Expr> &result);



Stmt grad_stmt(Expr expr, Ref<const Var> grad_to, Ref<const Var> doutput);

}  // namespace Autodiff

}  // namespace Boost


#endif  // BOOST_AUTODIFF_H