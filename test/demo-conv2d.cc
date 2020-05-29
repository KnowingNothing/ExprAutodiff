#include <string>
#include <iostream>

#include "IR.h"
#include "IRMutator.h"
#include "IRVisitor.h"
#include "IRPrinter.h"
#include "type.h"
#include "autodiff.h"
#include "codegen_C.h"
#include "genIR.h"
#include "typechecker.h"


using namespace Boost::Internal;

int main() {
  Stmt res = outinit("../../test/demo-inputs/demo-conv2d.in");
  Ref<const LoopNest> loop_nest = res.as<LoopNest>();

  IRPrinter p;
  p.enable_print_arg();
  p.enable_print_index();
  std::cout << "Original input expression:\n";
  std::cout << p.print(loop_nest->body_list[0]) << "\n";

  Expr b = loop_nest->index_list[0];
  Expr k = loop_nest->index_list[1];
  Expr pp = loop_nest->index_list[2];
  Expr q = loop_nest->index_list[3];
  Expr c = loop_nest->index_list[4];
  Expr r = loop_nest->index_list[5];
  Expr s = loop_nest->index_list[6];

  Ref<const Move> move = loop_nest->body_list[0].as<Move>();
  Expr A = move->src.as<Binary>()->b.as<Binary>()->a;
  Ref<const Var> C = move->dst.as<Var>();
  Expr dC = Var::make(C->type(), "d"+C->name, C->args, C->shape);

  Stmt new_stmt = Boost::Autodiff::grad_stmt(move->src, {b, k, pp, q, c, r, s}, {0, 1, 2, 3}, A.as<Var>(), dC.as<Var>());

  std::cout << "After autograd, the expression is:\n";
  std::cout << p.print(new_stmt) << "\n";

  Boost::Typechecker::Typechecker checker;
  std::cout << "typing: " << checker.check(new_stmt.as<Move>()->src) << "\n";
  return 0;
}