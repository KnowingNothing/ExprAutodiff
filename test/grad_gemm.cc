#include <string>
#include <iostream>

#include "IR.h"
#include "IRMutator.h"
#include "IRVisitor.h"
#include "IRPrinter.h"
#include "type.h"
#include "autodiff.h"
#include "codegen_C.h"

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

    // index c
    Expr dom_c = Dom::make(index_type, 0, K);
    Expr c = Index::make(index_type, "c", dom_c, IndexType::Reduce);

    // A
    Expr expr_A = Var::make(data_type, "A", {i, k}, {M, K});

    // A
    Expr expr_A_prime = Var::make(data_type, "A", {i, c}, {M, K});

    // B
    Expr expr_B = Var::make(data_type, "B", {k, j}, {K, N});

    // C
    Expr expr_C = Var::make(data_type, "C", {i, j}, {M, N});

    // D
    Expr expr_D = Var::make(data_type, "D", {c, j}, {K, N});

    // RHS
    Expr rhs = Binary::make(data_type, BinaryOpType::Add,
            Binary::make(data_type, BinaryOpType::Mul, expr_A, expr_B),
            Binary::make(data_type, BinaryOpType::Mul, expr_A_prime, expr_D));

    // main stmt
    Stmt main_stmt = Move::make(
        expr_C,
        rhs,
        MoveType::MemToMem
    );

    // loop nest
    Stmt loop_nest = LoopNest::make({i, j, k}, {main_stmt});

    // kernel
    Group kernel = Kernel::make("simple_gemm", {expr_A, expr_B}, {expr_C}, {loop_nest}, KernelType::CPU);

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


    Expr expr_dC = Var::make(data_type, "dC", {i, j}, {M, N});

    // auto diff
    Stmt new_stmt = Boost::Autodiff::grad_stmt(rhs, {i, j, k, c}, {0, 1}, expr_A.as<Var>(), expr_dC.as<Var>());
    
    std::cout << printer.print(new_stmt) << "\n";

    Boost::codegen::CodeGen_C gen;
    std::cout << "original: " << gen.print(rhs) << "\n";
    std::cout << gen.print(new_stmt) << "\n";

    std::cout << "Success!\n";
    return 0;
}