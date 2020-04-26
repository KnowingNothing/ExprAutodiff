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
#include "utils.h"
#include "autodiff.h"
#include "IRMutator.h"


namespace Boost {

using namespace Internal;

namespace Autodiff {


class GradOp : public IRMutator {
 private:
  Ref<const Var> grad_to_;
  Ref<const Var> output;
  
 public:
  GradOp(Ref<const Var> &grad_to) : grad_to_(grad_to) {}

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

  std::vector<Expr> new_args;
  Type index_type = Type::int_scalar(32);
  for (uint64_t s : grad_to->shape) {
    new_args.push_back(
      Index::make(
        index_type, gen("z"), Dom::make(index_type, Expr(0), Expr(s)), IndexType::Spatial)
      );
  }
  Expr new_dst = Var::make(grad_to->type(), gen("d" + grad_to->name), new_args, grad_to->shape);

  GradOp grader(grad_to);

  grader.grad(expr);

  Expr new_src = expr;

  Stmt stmt = Move::make(new_dst, new_src);
  return stmt;
};


}  // namespace Autodiff

}  // namespace Boost