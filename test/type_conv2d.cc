#include <string>
#include <iostream>

#include "IR.h"
#include "IRMutator.h"
#include "IRVisitor.h"
#include "IRPrinter.h"
#include "type.h"
#include "autodiff.h"
#include "codegen_C.h"
#include "typechecker.h"

using namespace Boost::Internal;

int main() {
    Boost::Typechecker::Typechecker checker;
    const int N = 256;
    const int C = 1024;
    const int P = 7;
    const int Q = 7;
    const int H = 9;
    const int W = 9;
    const int K = 1024;
    const int R = 3;
    const int S = 3;
    Type index_type = Type::int_scalar(32);
    Type data_type = Type::float_scalar(32);

    // index n
    Expr dom_n = Dom::make(index_type, 0, N);
    Expr n = Index::make(index_type, "n", dom_n, IndexType::Spatial);
    // std::cout << "n: " << checker.check(n) << "\n";

    // index k
    Expr dom_k = Dom::make(index_type, 0, K);
    Expr k = Index::make(index_type, "k", dom_k, IndexType::Spatial);
    // std::cout << "k: " << checker.check(k) << "\n";

    // index p
    Expr dom_p = Dom::make(index_type, 0, P);
    Expr p = Index::make(index_type, "p", dom_p, IndexType::Spatial);
    // std::cout << "p: " << checker.check(p) << "\n";

    // index q
    Expr dom_q = Dom::make(index_type, 0, Q);
    Expr q = Index::make(index_type, "q", dom_q, IndexType::Spatial);
    // std::cout << "q: " << checker.check(q) << "\n";

    // index c
    Expr dom_c = Dom::make(index_type, 0, C);
    Expr c = Index::make(index_type, "c", dom_c, IndexType::Reduce);
    // std::cout << "c: " << checker.check(c) << "\n";

    // index h
    Expr dom_h = Dom::make(index_type, 0, H);
    Expr h = Index::make(index_type, "h", dom_h, IndexType::Spatial);
    // std::cout << "h: " << checker.check(h) << "\n";

    // index w
    Expr dom_w = Dom::make(index_type, 0, W);
    Expr w = Index::make(index_type, "w", dom_w, IndexType::Spatial);
    // std::cout << "w: " << checker.check(w) << "\n";

    // index r
    Expr dom_r = Dom::make(index_type, 0, R);
    Expr r = Index::make(index_type, "r", dom_r, IndexType::Reduce);
    // std::cout << "r: " << checker.check(r) << "\n";

    // index s
    Expr dom_s = Dom::make(index_type, 0, S);
    Expr s = Index::make(index_type, "s", dom_s, IndexType::Reduce);
    // std::cout << "s: " << checker.check(s) << "\n";

    // I
    Expr expr_I = Var::make(data_type, "I",
        {n, c, Binary::make(index_type, BinaryOpType::Add, p, r),
               Binary::make(index_type, BinaryOpType::Add, q, s)},
        {N, C, H, W});
    std::cout << "expr_I: " << checker.check(expr_I) << "\n";

    // W
    Expr expr_W = Var::make(data_type, "W", {k, c, r, s}, {K, C, R, S});
    std::cout << "expr_W: " << checker.check(expr_W) << "\n";

    // O
    Expr expr_O = Var::make(data_type, "O", {n, k, p, q}, {N, K, P, Q});
    std::cout << "O: " << checker.check(expr_O) << "\n";

    //src
    Expr src = Binary::make(data_type, BinaryOpType::Add, expr_O,
            Binary::make(data_type, BinaryOpType::Mul, expr_I, expr_W));
    std::cout << "src: " << checker.check(src) << "\n";

    // main stmt
    Stmt main_stmt = Move::make(
        expr_O,
        src,
        MoveType::MemToMem
    );

    // loop nest
    Stmt loop_nest = LoopNest::make({n, k, p, q, c, r, s}, {main_stmt});

    // kernel
    Group kernel = Kernel::make("simple_conv2d", {expr_I, expr_W}, {expr_O}, {loop_nest}, KernelType::CPU);

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

    Expr expr_dO = Var::make(data_type, "dO", {n, k, p, q}, {N, K, P, Q});
    std::cout << "expr_dO: " << checker.check(expr_dO) << "\n";

    // auto diff
    Stmt new_stmt = Boost::Autodiff::grad_stmt(src, {n, k, p, q, c, r, s}, {0, 1, 2, 3}, expr_W.as<Var>(), expr_dO.as<Var>());

    std::cout << printer.print(new_stmt) << "\n";

    std::cout << "grad: " << checker.check(dynamic_cast<const Move&>(*new_stmt).src) << "\n";

    Boost::codegen::CodeGen_C gen;
    std::cout << gen.print(new_stmt) << "\n";

    std::cout << "Success!\n";
    return 0;
}