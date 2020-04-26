#include <string>
#include <iostream>

#include "IR.h"
#include "IRMutator.h"
#include "IRFunctor.h"
#include "IRPrinter.h"
#include "type.h"

using namespace Boost::Internal;


class MyVisitor : public IRFunctor<void(int)> {
 public:
   void visit(Ref<const IntImm>, int a) override {}
   void visit(Ref<const UIntImm>, int a) override {}
   void visit(Ref<const FloatImm>, int a) override {}
   void visit(Ref<const StringImm>, int a) override {}
   void visit(Ref<const Unary>, int a) override {}
   void visit(Ref<const Binary>, int a) override {}
   void visit(Ref<const Select>, int a) override {}
   void visit(Ref<const Compare>, int a) override {}
   void visit(Ref<const Call>, int a) override {}
   void visit(Ref<const Var>, int a) override {}
   void visit(Ref<const Cast>, int a) override {}
   void visit(Ref<const Ramp>, int a) override {}
   void visit(Ref<const Index>, int a) override {}
   void visit(Ref<const Dom>, int a) override {}
   void visit(Ref<const LoopNest>, int a) override {}
   void visit(Ref<const IfThenElse>, int a) override {}
   void visit(Ref<const Move>, int a) override {}
   void visit(Ref<const Kernel>, int a) override {}
   void visit(Ref<const PlaceholderOp>, int a) override {}
   void visit(Ref<const ComputeOp>, int a) override {}
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

    // main stmt
    Stmt main_stmt = Move::make(
        expr_C,
        Binary::make(data_type, BinaryOpType::Add, expr_C,
            Binary::make(data_type, BinaryOpType::Mul, expr_A, expr_B)),
        MoveType::MemToMem
    );

    // loop nest
    Stmt loop_nest = LoopNest::make({i, j, k}, {main_stmt});

    // kernel
    Group kernel = Kernel::make("simple_gemm", {expr_A, expr_B}, {expr_C}, {loop_nest}, KernelType::CPU);

    // functor
    // IRFunctor<void(int)> visitor;
    // kernel.visit_(&visitor);
    MyVisitor myvisitor;
    kernel.visit_(myvisitor);

    std::cout << "Success!\n";
    return 0;
}