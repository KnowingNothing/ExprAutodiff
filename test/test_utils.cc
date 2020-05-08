#include <iostream>

#include "debug.h"
#include "IR.h"
#include "utils.h"

using namespace std;
using namespace Boost::Internal;
using namespace Boost::Utils;


Expr get_gemm_expr_rhs() {
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

  // RHS
  Expr rhs = Binary::make(data_type, BinaryOpType::Add, expr_C,
          Binary::make(data_type, BinaryOpType::Mul, expr_A, expr_B));
  return rhs;
}


void test_expr_equal_by_value() {
  Expr expr1 = get_gemm_expr_rhs();
  Expr expr2 = get_gemm_expr_rhs();
  ExprEqualByValue eev;
  ASSERT(eev.visit_expr(expr1, expr2)) << "Test ExprEqualByValue failed.";
  cout << "Test ExprEqualByValue success!\n";
}


int main() {
  test_expr_equal_by_value();
  return 0;
}