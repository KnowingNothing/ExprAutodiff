#include <string>
#include <iostream>

#include "IR.h"
#include "IRMutator.h"
#include "IRFunctor.h"
#include "IRPrinter.h"
#include "type.h"

using namespace Boost::Internal;


class ExprVisitor : public ExprFunctor<int(const Expr&)> {
 public:
   int visit(Ref<const IntImm> op) override { return 1; }
   int visit(Ref<const UIntImm> op) override { return 1; }
   int visit(Ref<const FloatImm> op) override { return 1; }
   int visit(Ref<const StringImm> op) override { return 1; }
   int visit(Ref<const Unary> op) override { return visit_expr(op->a) + 1; }
   int visit(Ref<const Binary> op) override { return visit_expr(op->a) + visit_expr(op->b) + 1; }
   int visit(Ref<const Select> op) override { return visit_expr(op->cond) + visit_expr(op->true_value) + visit_expr(op->false_value) + 1; }
   int visit(Ref<const Compare> op) override { return visit_expr(op->a) + visit_expr(op->b) + 1; }
   int visit(Ref<const Call> op) override {
       int tmp = 0;
       for (auto arg : op->args) {
           tmp += visit_expr(arg);
       }
       return tmp;
   }
   int visit(Ref<const Var> op) override {
       int tmp = 0;
       for (auto arg : op->args) {
           tmp += visit_expr(arg);
       }
       return tmp;
   }
   int visit(Ref<const Cast> op) override { return visit_expr(op->val) + 1; }
   int visit(Ref<const Ramp> op) override {
       return visit_expr(op->base) + 1;
   }
   int visit(Ref<const Index> op) override {
       return visit_expr(op->dom) + 1;
   }
   int visit(Ref<const Dom> op) override {
       return visit_expr(op->begin) + visit_expr(op->extent) + 1;
   }
};

int main() {
    const int M = 1024;
    const int N = 512;
    const int K = 256;
    Type index_type = Type::int_scalar(32);
    Type data_type = Type::float_scalar(32);

    // index i
    Expr dom_i = Dom::make(index_type, 0, M);
    Expr i = Index::make(index_type, "i", dom_i, IndexType::Spatial);

    // index j
    Expr dom_j = Dom::make(index_type, 0, N);
    Expr j = Index::make(index_type, "j", dom_j, IndexType::Spatial);

    // index k
    Expr dom_k = Dom::make(index_type, 0, K);
    Expr k = Index::make(index_type, "k", dom_k, IndexType::Reduce);

    // A
    Expr expr_A = Var::make(data_type, "A", {i, k}, {M, K});

    // B
    Expr expr_B = Var::make(data_type, "B", {k, j}, {K, N});

    // C
    Expr expr_C = Var::make(data_type, "C", {i, j}, {M, N});

    Expr src = Binary::make(data_type, BinaryOpType::Add, expr_C,
            Binary::make(data_type, BinaryOpType::Mul, expr_A, expr_B));

    // main stmt
    Stmt main_stmt = Move::make(
        expr_C,
        src,
        MoveType::MemToMem
    );

    // loop nest
    Stmt loop_nest = LoopNest::make({i, j, k}, {main_stmt});

    // kernel
    Group kernel = Kernel::make("simple_gemm", {expr_A, expr_B}, {expr_C}, {loop_nest}, KernelType::CPU);

    // functor
    ExprVisitor visitor;
    std::cout << visitor.visit_expr(src) << "\n";

    std::cout << "Success!\n";
    return 0;
}