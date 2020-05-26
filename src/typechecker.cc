/*
 * MIT License
 * 
 * Copyright (c) 2020 Muzhi Yu

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

#include <sstream>

#include "debug.h"
#include "typechecker.h"

namespace Boost
{

  using namespace Internal;

  namespace Typechecker
  {

    IndexConstraint &IndexConstraint::merge(const IndexConstraint &other)
    {
      // std::cout << "Before merge\n"
      //           << *this << "\n"
      //           << other << "\n";
      IndexConstraint &ret = *new IndexConstraint(*this);

      // fill new names and build map for (new) name->new_index
      int i = 0;
      std::map<std::string, int> new_index_of;

      for (auto it = other.index_names.begin(); it != other.index_names.end(); ++it)
      {
        auto it_name = std::find(index_names.begin(), index_names.end(), *it);
        if (it_name == index_names.end()) // index name only in other
        {
          ret.index_names.push_back(*it);
          new_index_of[*it] = i++;
        }
      }

      for (auto it = other.index_constraints_A_array.begin(); it != other.index_constraints_A_array.end(); ++it)
      {
        auto it_name = std::find(index_names.begin(), index_names.end(), other.index_names[it->col()]);
        if (it_name == index_names.end()) // index name only in other
        {
          ret.index_constraints_A_array.push_back(Eigen::Triplet<double>(
              it->row() + index_constraints_b.size(),
              new_index_of[other.index_names[it->col()]] + index_names.size(),
              it->value()));
        }
        else // index name also in ret
        {
          int i_index = it_name - index_names.begin();
          // *it = Eigen::Triplet<double>(it->row() + index_constraints_A_array.size(), i_index, it->value());
          ret.index_constraints_A_array.push_back(Eigen::Triplet<double>(
              it->row() + index_constraints_b.size(),
              i_index,
              it->value()));
        }
      }

      Eigen::VectorXd vec_joined(other.index_constraints_b.size() + index_constraints_b.size());
      vec_joined << index_constraints_b, other.index_constraints_b;
      ret.index_constraints_b = vec_joined;

      // std::cout << "After merge\n"
      //           << ret << "\n";
      return ret;
    }

    std::map<std::string, int> IndexConstraint::solve() const
    {
      auto index_constraints_A = Eigen::SparseMatrix<double>(index_constraints_b.size(), index_names.size());
      index_constraints_A.setFromTriplets(index_constraints_A_array.begin(),
                                          index_constraints_A_array.end());

      Eigen::SparseQR<Eigen::SparseMatrix<double>, Eigen::COLAMDOrdering<int>> solver(index_constraints_A);
      auto x = solver.solve(index_constraints_b);
      if (!index_constraints_b.isApprox(index_constraints_A * x))
      {
        throw TypecheckException("Index onstraints in conflict");
      }
      // std::cout << x << "\n";
      // TODO: check x > 0
      auto ret = std::map<std::string, int>();
      int i = 0;
      for (auto it = index_names.begin(); it != index_names.end(); ++it)
      {
        ret[*it] = std::round(x[i]);
        ++i;
      }
      return ret;
    }

    const TSType &Typechecker::check(const Expr &expr)
    {
      while (!type_stack.empty())
      {
        type_stack.pop();
      }
      while (!op_stack.empty())
      {
        op_stack.pop();
      }
      expr.visit_expr(this);
      return type_stack.top();
    }

    /* Typechecker visit methods */

    void Typechecker::visit(Ref<const IntImm> op)
    {
      type_stack.push(TSType(BaseType::Int, {}, false));
    }

    void Typechecker::visit(Ref<const FloatImm> op)
    {
      type_stack.push(TSType(BaseType::Float, {}, false));
    }

    void Typechecker::visit(Ref<const Unary> op)
    {
      (op->a).visit_expr(this);
    }

    void Typechecker::visit(Ref<const Binary> op)
    {
      (op->a).visit_expr(this);
      (op->b).visit_expr(this);

      TSType type_b = type_stack.top();
      type_stack.pop();
      TSType type_a = type_stack.top();
      type_stack.pop();

      if (op->op_type == BinaryOpType::Add ||
          op->op_type == BinaryOpType::Sub)
      {
        type_stack.push(type_a.meet(type_b));
      }
      else if (op->op_type == BinaryOpType::Mul)
      {
        type_stack.push(type_a.meet(type_b));
      }
      else if (op->op_type == BinaryOpType::Div)
      {
        CHECK((type_a.is_tensor && type_b.is_tensor), "Tensor div unsupported.");
        auto new_type = type_a.meet(type_b);
        new_type.basetype = BaseType::Float;
        type_stack.push(new_type);
      }
      else if (op->op_type == BinaryOpType::FloorDiv)
      {
        CHECK((type_a.is_tensor && type_b.is_tensor), "Tensor floordiv unsupported.");
        auto new_type = type_a.meet(type_b);
        new_type.basetype = BaseType::Int;
        type_stack.push(new_type);
      }
      else
      {
        throw TypecheckException("Unknown BinOP");
      }
    }

    void Typechecker::visit(Ref<const Var> op)
    {
      BaseType base_type = op->type().is_float() ? BaseType::Float : BaseType::Int;
      // bool is_tensor = !op->type().is_scalar();
      bool is_tensor = true; // hastle

      std::map<std::string, int> shape;

      int arg_pos = 0;
      auto ic = IndexConstraint();
      for (auto it = op->args.begin(); it != op->args.end(); ++it)
      {
        auto arg_visitor = ArgVisitor(op->shape[arg_pos]);
        it->visit_expr(&arg_visitor);
        ic = ic.merge(arg_visitor.retrieve());
        // if (it->get()->node_type() == IRNodeType::Index)
        // {
        //   shape[dynamic_cast<const Index *>(it->get())->name] = op->shape[arg_pos];
        // }
        ++arg_pos;
      }
      type_stack.push(TSType(base_type, ic, is_tensor));
    }

    /* ArgVisitor */
    void ArgVisitor::visit(Ref<const Index> op)
    {
      index_weight[op->name] = 1;
    }

    void ArgVisitor::visit(Ref<const Binary> op)
    {
      (op->a).visit_expr(this);
      (op->b).visit_expr(this);

      if (op->op_type == BinaryOpType::Add)
      {
        shape += 1;
      }
      else if (op->op_type == BinaryOpType::Sub)
      {
        // TODO
      }
      else if (op->op_type == BinaryOpType::Mul)
      {
        // TODO
      }
      else if (op->op_type == BinaryOpType::FloorDiv)
      {
        // TODO
      }
      else
      {
        LOG(ERROR) << int(op->op_type) << "Index operations unsupported yet.";
      }
      return;
    }

  } // namespace Typechecker

} // namespace Boost
