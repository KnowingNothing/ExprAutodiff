#include <string>
#include <iostream>

#include "IR.h"
#include "IRMutator.h"
#include "IRVisitor.h"
#include "IRPrinter.h"
#include "type.h"

using namespace Boost::Internal;

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

    // placeholder_A
    Operation placeholder_A = PlaceholderOp::make(data_type, "A", {i, k}, {M, K});

    // placeholder_B
    Operation placeholder_B = PlaceholderOp::make(data_type, "B", {k, j}, {K, N});

    // C
    Expr expr_C = Var::make(data_type, "C", {i, j}, {M, N});

    // main stmt
    Stmt main_stmt = Move::make(
        expr_C,
        Binary::make(data_type, BinaryOpType::Add, expr_C,
            Binary::make(data_type, BinaryOpType::Mul, placeholder_A.output_expr()[0], 
                placeholder_B.output_expr()[0])), MoveType::MemToMem
    );

    // compute
    Operation compute = ComputeOp::make({i, j, k}, {main_stmt});

    // kernel
    Group kernel = Kernel::make("simple_gemm", {"A", "B"}, {"C"}, {compute.operation_stmt()}, KernelType::CPU);

    // visitor
    IRVisitor visitor;
    kernel.visit_group(&visitor);

    // mutator
    IRMutator mutator;
    kernel = mutator.mutate(kernel);

    // printer
    IRPrinter printer;
    std::string code = printer.print(kernel);

    std::cout << code;

    std::cout << "Success!\n";
    return 0;
}