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

#include <vector>

#include "debug.h"
#include "type.h"
#include "arith.h"
#include "utils.h"
#include "autodiff.h"
#include "IRMutator.h"


namespace Boost {

using namespace Internal;

namespace Autodiff {


int SubstituteContext::find_bound(Expr &expr) {
  Utils::ExprEqualByValue eev;
  int i = 0;
  for (auto kv : expr2var) {
    if (eev.visit_expr(expr, kv.first)) {
      return i;
    }
    ++i;
  }
  return -1;
}


std::string SubstituteContext::get_bound_name(Expr &expr) {
  int id = find_bound(expr);
  if (id < 0) {
    return "";
  } else {
    return expr2var[id].second;
  }
}


void SubstituteContext::add(std::string &name, Ref<const Index> index, Expr expr, Arith::ExtRange range) {
  ASSERT(var2expr.count(name) == 0) << "Internal error: variable of the same name: " << name << "\n";
  if (bound_begin < 0) {
    bound_begin = (int)index_names.size();
  }
  index_names.push_back(name);
  index_map[name] = index;
  range_map[name] = range;
  var2expr[name] = expr;
  expr2var.push_back(std::make_pair(expr, name));
}


Expr EliminateIndexFloorDivAndMod::visit(Ref<const Binary> op) {
  Expr new_a = visit_expr(op->a);
  Expr new_b = visit_expr(op->b);
  if (op->op_type == BinaryOpType::FloorDiv) {
    Ref<const Index> a_as_index = new_a.as<Index>();
    Ref<const IntImm> b_as_int = new_b.as<IntImm>();
    ASSERT(b_as_int.defined()) << "Only support floor_div on type int, but find " << new_b->type() << "\n";

    Expr new_div;
    std::string new_name;
    if (!a_as_index.defined()) {
      // bound left expr to unique var
      new_name = context_.get_bound_name(new_a);
      Ref<const Index> new_var = Index::make(
        new_a->type(), new_name, Dom::make(new_a->type(), Expr(), Expr()), IndexType::Unknown).as<Index>();
      if (new_name == "") {
        // not found
        new_name = name_generator_.unique_name(substitute_name_hint_);
        new_var = Index::make(
          new_a->type(), new_name, Dom::make(new_a->type(), Expr(), Expr()), IndexType::Unknown).as<Index>();
        // we don't infer the range of expression
        // TODO: infer range for expression
        context_.add(new_name, new_var, new_a, Arith::ExtRange());
      }
      new_div = Arith::floordiv(new_var, new_b);
    } else {
      // left expr is already a var
      new_name = a_as_index->name;
      new_div = Arith::floordiv(new_a, new_b);
    }
    
    // check if this new div expr is bounded
    // check by value equal
    std::string bound_this = context_.get_bound_name(new_div);
    if (bound_this == "") {
      // not bound
      std::string new_div_name = name_generator_.unique_name(substitute_name_hint_);
      // we should know the left var
      Ref<const Index> new_var = Index::make(
          new_div->type(), new_div_name, Dom::make(new_div->type(), Expr(), Expr()), IndexType::Unknown).as<Index>();
      context_.add(new_div_name, new_var,
          new_div, context_.range_map[new_name].floor_div((int)b_as_int->value()));
      return std::move(new_var);
    } else {
      // bound
      return context_.index_map[bound_this];
    }
  } else if (op->op_type == BinaryOpType::FloorMod) {
    Ref<const Index> a_as_index = new_a.as<Index>();
    Ref<const IntImm> b_as_int = new_b.as<IntImm>();
    ASSERT(b_as_int.defined()) << "Only support floor_mod on type int, but find " << new_b->type() << "\n";

    Expr new_mod;
    std::string new_name;
    if (!a_as_index.defined()) {
      // bound left expr to unique var
      new_name = context_.get_bound_name(new_a);
      Ref<const Index> new_var = Index::make(
        new_a->type(), new_name, Dom::make(new_a->type(), Expr(), Expr()), IndexType::Unknown).as<Index>();
      if (new_name == "") {
        // not found
        new_name = name_generator_.unique_name(substitute_name_hint_);
        new_var = Index::make(
          new_a->type(), new_name, Dom::make(new_a->type(), Expr(), Expr()), IndexType::Unknown).as<Index>();
        // we don't infer the range of expression
        // TODO: infer range for expression
        context_.add(new_name, new_var, new_a, Arith::ExtRange());
      }
      new_mod = Arith::floormod(new_var, new_b);
    } else {
      // left expr is already a var
      new_name = a_as_index->name;
      new_mod = Arith::floormod(new_a, new_b);
    }
    
    // check if this new div expr is bounded
    // check by value equal
    std::string bound_this = context_.get_bound_name(new_mod);
    if (bound_this == "") {
      // not bound
      std::string new_mod_name = name_generator_.unique_name(substitute_name_hint_);
      // we should know the left var
      Ref<const Index> new_var = Index::make(
          new_mod->type(), new_mod_name, Dom::make(new_mod->type(), Expr(), Expr()), IndexType::Unknown).as<Index>();
      context_.add(new_mod_name, new_var,
          new_mod, context_.range_map[new_name].floor_div((int)b_as_int->value()));
      return std::move(new_var);
    } else {
      // bound
      return context_.index_map[bound_this];
    }
  }
  return Binary::make(op->type(), op->op_type, new_a, new_b);
}


void ExtractIndexCoefficients::visit(Ref<const Index> op) {
  (*(scope_.back()))[op->name] = (VType)1;
}


void ExtractIndexCoefficients::visit(Ref<const Unary> op) {
  std::unordered_map<std::string, VType> tmp;
  scope_.push_back(&tmp);
  visit_expr(op->a);
  scope_.pop_back();
  if (op->op_type == UnaryOpType::Neg) {
    for (auto kv : tmp) {
      (*(scope_.back()))[kv.first] = -kv.second;
    }
  } else UNEXPECTED
}


void ExtractIndexCoefficients::visit(Ref<const Binary> op) {
  std::unordered_map<std::string, VType> a;
  scope_.push_back(&a);
  visit_expr(op->a);
  scope_.pop_back();
  std::unordered_map<std::string, VType> b;
  scope_.push_back(&b);
  visit_expr(op->b);
  scope_.pop_back();

  switch (op->op_type)
  {
  case BinaryOpType::Add:
    {
      for (auto kv : a) {
        (*(scope_.back()))[kv.first] = kv.second;
      }
      for (auto kv : b) {
        if ((*(scope_.back())).count(kv.first) != 0) {
          (*(scope_.back()))[kv.first] += kv.second;
        } else {
          (*(scope_.back()))[kv.first] = kv.second;
        }
      }
    }
    break;
  case BinaryOpType::Sub:
    {
      for (auto kv : a) {
        (*(scope_.back()))[kv.first] = kv.second;
      }
      for (auto kv : b) {
        if ((*(scope_.back())).count(kv.first) != 0) {
          (*(scope_.back()))[kv.first] -= kv.second;
        } else {
          (*(scope_.back()))[kv.first] = kv.second;
        }
      }
    }
    break;
  case BinaryOpType::Mul:
    {
      // merge results from two branches
      for (auto kv1 : a) {
        for (auto kv2 : b) {
          if (kv1.first == const_tag_) {
            if ((*(scope_.back())).count(kv2.first)) {
              (*(scope_.back()))[kv2.first] += kv1.second * kv2.second;
            } else {
              (*(scope_.back()))[kv2.first] = kv1.second * kv2.second;
            }
          } else if (kv2.first == const_tag_) {
            if ((*(scope_.back())).count(kv1.first)) {
              (*(scope_.back()))[kv1.first] += kv1.second * kv2.second;
            } else {
              (*(scope_.back()))[kv1.first] = kv1.second * kv2.second;
            }
          } else {
            LOG(ERROR) << "Find index multiply: " << Expr(op); throw;
          }
        }
      }
    }
    break;
  default: UNEXPECTED
    break;
  }
}


FloorDivModEntry FloorDivModEntry::merge(const FloorDivModEntry &other) const {
  ASSERT((*this) == other) << "Can't handle different entry.\n";
  std::string new_first;
  std::string new_second;
  if (first != "" && other.first == "") {
    new_first = first;
  } else if (first == "" && other.first != "") {
    new_first = other.first;
  } else {
    LOG(ERROR) << "Unexpected conflict.\n";
  }

  if (second != "" && other.second == "") {
    new_second = second;
  } else if (second == "" && other.second != "") {
    new_second = other.second;
  } else {
    LOG(ERROR) << "Unexpected conflict.\n";
  }

  return FloorDivModEntry(factor, var_name, new_first, new_second);
}


void solve_floor_div_mod(const SubstituteContext &context,
  std::unordered_set<FloorDivModEntry, FloorDivModEntryHash> &s) {
  for (auto kv : context.var2expr) {
    FloorDivModEntry entry;
    Ref<const Binary> as_div_mod = kv.second.as<Binary>();
    ASSERT(as_div_mod.defined() && (as_div_mod->op_type == BinaryOpType::FloorDiv
            || as_div_mod->op_type == BinaryOpType::FloorDiv)) << "Only can solve floor_div or floor_mod now.\n";
    if (as_div_mod->op_type == BinaryOpType::FloorDiv) {
      Ref<const IntImm> as_int = as_div_mod->b.as<IntImm>();
      Ref<const Index> as_var = as_div_mod->a.as<Index>();
      ASSERT(as_var.defined()) << "Div must be on variable.\n";
      ASSERT(as_int.defined()) << "Div factor must be int type.\n";
      entry.factor = (int)as_int->value();
      entry.first = kv.first;
      entry.var_name = as_var->name;
    } else {
      Ref<const IntImm> as_int = as_div_mod->b.as<IntImm>();
      Ref<const Index> as_var = as_div_mod->a.as<Index>();
      ASSERT(as_var.defined()) << "Mod must be on variable.\n";
      ASSERT(as_int.defined()) << "Mod factor must be int type.\n";
      entry.factor = (int)as_int->value();
      entry.second = kv.first;
      entry.var_name = as_var->name;
    }
    auto it = s.find(entry);
    if (it != s.end()) {
      FloorDivModEntry new_entry = entry.merge(*it);
      s.erase(it);
      s.insert(new_entry);
    } else {
      s.insert(entry);
    }
  }
}


Expr solve_multi_bindings(SubstituteContext &context, std::vector<Expr> &bindings,
    std::unordered_set<std::string> &unused, std::vector<Expr> &conditions) {
  int num_bindings = (int)bindings.size();
  ASSERT(num_bindings > 0) << "Internal error: empty bindings.\n";
  int res_pos = -1;
  Expr res;
  for (int i = 0; i < num_bindings; ++i) {
    Ref<const Index> as_var = bindings[i].as<Index>();
    if (as_var.defined()) {
      ASSERT(context.range_map.count(as_var->name) != 0) << "Internal error: unknown var: "
                                                             << as_var->name << ".\n";
      Arith::ExtRange range = context.range_map[as_var->name];
      Arith::ExtRangeType range_type = range.range_type();
      if (range_type == Arith::ExtRangeType::LCRC) {
        res = bindings[i];
        res_pos = i;
        break;
      }
    } else {
      res = bindings[i];
      res_pos = i;
    }
  }

  // all the bindings are unbounded
  if (res_pos < 0) {
    res = bindings[0];
    res_pos = 0;
  }

  // the second pass
  // merge bindings
  for (int i = 0; i < num_bindings; ++i) {
    // skip res itself
    if (i == res_pos) {
      continue;
    }
    Ref<const Index> as_var = bindings[i].as<Index>();
    if (as_var.defined()) {
      ASSERT(context.range_map.count(as_var->name) != 0) << "Internal error: unknown var: "
                                                             << as_var->name << ".\n";
      Arith::ExtRange range = context.range_map[as_var->name];
      Arith::ExtRangeType range_type = range.range_type();
      if (range_type == Arith::ExtRangeType::LORO) {
        // skip (-inf, +inf)
        unused.insert(as_var->name);
      } else if (range_type == Arith::ExtRangeType::LORC) {
        // (-inf, val)
        // this shouldn't be index
        // conditions.push_back(LTNode::make(res, range.right));
        LOG(ERROR) << "Unexpected range : (-inf, " << range.right << ").\n";
        throw;
      } else if (range_type == Arith::ExtRangeType::LCRO) {
        // [val, +inf)
        // this shouldn't be index
        // conditions.push_back(GENode::make(res, range.left));
        LOG(ERROR) << "Unexpected range : [" << range.right << ", +inf).\n";
        throw;
      } else {
        // [val1, val2)
        // this should be index
        conditions.push_back(Arith::eq(res, bindings[i]));
      }
    }
  }

  return res;
}


void solve_substitutions(SubstituteContext &context,
  std::unordered_map<std::string, std::vector<Expr>> &bindings,
  std::unordered_set<std::string> &unused,
  std::vector<Expr> &conditions, std::unordered_map<std::string, Expr> &result) {
  int num_index = (int)context.index_names.size();
  int end = context.bound_begin < 0 ? num_index : context.bound_begin;
  for (int i = num_index - 1; i >= end; --i) {
    std::string sub_var_name = context.index_names[i];
    ASSERT(bindings.count(sub_var_name) != 0) << "Internal error: unknown substitution var: "
                                             << sub_var_name << ".\n";
    Expr unique_binding = solve_multi_bindings(context, bindings[sub_var_name], unused, conditions);
    std::unordered_map<std::shared_ptr<const Index>, Expr> vmap;
    vmap[context.index_map[sub_var_name].real_ptr()] = unique_binding;
    std::cout << "check solve sub var: " << sub_var_name << "\n";
    for (int j = i - 1; j >= 0; --j) {
      std::vector<Expr> new_bindings;
      for (auto expr : bindings[context.index_names[j]]) {
        std::cout << "target expr: " << expr << "\n";
        new_bindings.push_back(Utils::substitute_index(expr, vmap));
      }
      // replace bindings
      bindings[context.index_names[j]] = new_bindings;
    }
  }
  // solve indice that are not substitution vars
  int beg = context.bound_begin < 0 ? num_index - 1 : context.bound_begin;
  for (int i = beg; i >= 0; --i) {
    std::string var_name = context.index_names[i];
    ASSERT(bindings.count(var_name) != 0) << "Internal error: unknown var: "
                                             << var_name << ".\n";
    Expr unique_binding = solve_multi_bindings(context, bindings[var_name], unused, conditions);
    result[var_name] = unique_binding;
  }
}


class GradOp : public IRMutator {
 private:

  std::string const_tag_;
  std::string sub_hint_;
  std::string dummy_tag_;
  Utils::NameGenerator &generator_;
  SubstituteContext &context_;
  EliminateIndexFloorDivAndMod *eliminator_;
  Ref<const Var> grad_to_;
  Ref<const Var> doutput_;
  std::vector<Expr> &call_args_;
  std::vector<Expr> compute_args_;
  std::vector<std::unordered_map<std::shared_ptr<const Index>, Expr>> vmap_scope_;
  
 public:
  GradOp(Utils::NameGenerator &generator, SubstituteContext &context, Ref<const Var> &grad_to,
    Ref<const Var> &doutput, std::vector<Expr> &call_args, std::vector<Expr> compute_args) :
    generator_(generator), context_(context), grad_to_(grad_to), doutput_(doutput),
    call_args_(call_args), compute_args_(compute_args) {
      const_tag_ = generator_.unique_name("_const");
      sub_hint_ = generator_.unique_name("_s");
      dummy_tag_ = generator_.unique_name("_r");
      eliminator_ = new EliminateIndexFloorDivAndMod(generator_, sub_hint_, context_);
    }

  Expr grad(Expr expr) {
    return mutate(expr);
  }

  // virtual Expr visit(Ref<const IntImm>);
  // virtual Expr visit(Ref<const UIntImm>);
  // virtual Expr visit(Ref<const FloatImm>);
  // virtual Expr visit(Ref<const StringImm>);
  // virtual Expr visit(Ref<const Unary>);
  // virtual Expr visit(Ref<const Binary>);
  // virtual Expr visit(Ref<const Select>);
  // virtual Expr visit(Ref<const Compare>);
  // virtual Expr visit(Ref<const Call>);

  Expr visit(Ref<const Var> op) override {
    if (op == grad_to_) {
      std::vector<std::unordered_map<std::string, int>> coeffs;
      // handle args
      for (const Expr &arg : op->args) {
        // eliminate possible mod & div
        Expr new_arg = eliminator_->eliminate(arg);
        // extract coefficients
        ExtractIndexCoefficients extractor(const_tag_);
        std::unordered_map<std::string, int> tmp;
        extractor.get_coefficients(new_arg, tmp);
        coeffs.push_back(tmp);
      }

      int cols = (int)context_.index_names.size();
      int rows = (int)coeffs.size();
      Arith::Matrix<int> trans(rows, cols);
      for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
          if (coeffs[i].count(context_.index_names[j]) != 0) {
            // has the coefficent for this index
            trans[i][j] = coeffs[i][context_.index_names[j]];
          } else {
            trans[i][j] = 0;
          }
        }
        // has constants
        if (coeffs[i].count(const_tag_)) {
          compute_args_[i] = Arith::sub(compute_args_[i], coeffs[i][const_tag_]);
        }
      }
    } else {

    }
    return op;
  }

  // virtual Expr visit(Ref<const Cast>);
  // virtual Expr visit(Ref<const Ramp>);
  // virtual Expr visit(Ref<const Index>);
  // virtual Expr visit(Ref<const Dom>);
  // virtual Stmt visit(Ref<const LoopNest>);
  // virtual Stmt visit(Ref<const IfThenElse>);
  // virtual Stmt visit(Ref<const Move>);
  // virtual Group visit(Ref<const Kernel>);
  // virtual Operation visit(Ref<const PlaceholderOp>);
  // virtual Operation visit(Ref<const ComputeOp>);
};


Stmt grad_stmt(Expr expr, Ref<const Var> grad_to, Ref<const Var> doutput) {
  Utils::NameGenerator gen;
  SubstituteContext context;


  std::vector<Expr> new_args;
  Type index_type = Type::int_scalar(32);
  for (uint64_t s : grad_to->shape) {
    new_args.push_back(
      Index::make(
        index_type, gen("z"), Dom::make(index_type, Expr(0), Expr(s)), IndexType::Spatial)
      );
  }
  Expr new_dst = Var::make(grad_to->type(), gen("d" + grad_to->name), new_args, grad_to->shape);

  std::vector<Expr> free_args;

  GradOp grader(gen, context, grad_to, doutput, free_args, new_args);

  grader.grad(expr);

  Expr new_src = expr;

  Stmt stmt = Move::make(new_dst, new_src);
  return stmt;
};


}  // namespace Autodiff

}  // namespace Boost