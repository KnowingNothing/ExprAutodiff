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

#ifndef BOOST_TYPECHECKER_H
#define BOOST_TYPECHECKER_H

#include <string>
#include <sstream>
#include <stack>
#include <vector>
#include <map>
#include <exception>
#include <Eigen/Sparse>

#include "IRVisitor.h"

using namespace Boost::Internal;

namespace Boost
{

  namespace Typechecker
  {
    enum class BaseType : uint8_t
    {
      Int,
      Float
    };

    class TypecheckException : public std::exception
    {
    public:
      TypecheckException() {}
      TypecheckException(const std::string &msg_) : msg(msg_) {}
      const char *what() const throw()
      {
        return msg.c_str();
      }

    private:
      std::string msg;
    };

    class IndexConstraint
    {
    public:
      std::vector<std::string> index_names;
      std::vector<Eigen::Triplet<double>> index_constraints_A_array;
      Eigen::VectorXd index_constraints_b;

      IndexConstraint() {}
      IndexConstraint(const IndexConstraint &other) : index_names(other.index_names),
                                                      index_constraints_A_array(other.index_constraints_A_array),
                                                      index_constraints_b(other.index_constraints_b) {}

      IndexConstraint(const std::vector<std::string> &_index_names,
                      const std::vector<Eigen::Triplet<double>> &_index_constraints_A_array,
                      const Eigen::VectorXd &_index_constraints_b) : index_names(_index_names),
                                                                     index_constraints_A_array(_index_constraints_A_array),
                                                                     index_constraints_b(_index_constraints_b) {}
      IndexConstraint &merge(const IndexConstraint &other);

      std::map<std::string, int> solve() const;

      friend std::ostream &operator<<(std::ostream &out, const IndexConstraint &ic)
      {
        // for (auto it = ic.index_names.begin(); it != ic.index_names.end(); ++it)
        // {
        //   std::cout << *it << ", ";
        // }
        // std::cout << "\n";
        // for (auto it = ic.index_constraints_A_array.begin(); it != ic.index_constraints_A_array.end(); ++it)
        // {
        //   std::cout << "(" << it->row() << "," << it->col() << "," << it->value() << ")\n";
        // }
        auto index_constraints_A = Eigen::SparseMatrix<double>(ic.index_constraints_b.size(), ic.index_names.size());
        index_constraints_A.setFromTriplets(ic.index_constraints_A_array.begin(),
                                            ic.index_constraints_A_array.end());
        out << "A:\n"
            << static_cast<const Eigen::SparseMatrixBase<Eigen::SparseMatrix<double>> &>(index_constraints_A)
            << "b:\n"
            << ic.index_constraints_b << "\n";

        return out;
      }
    };

    class TSType
    {
    public:
      BaseType basetype;
      IndexConstraint ic;
      bool is_tensor;

      TSType() {}
      TSType(const TSType &other) : basetype(other.basetype),
                                    ic(other.ic),
                                    is_tensor(other.is_tensor) {}
      TSType(BaseType _basetype,
             IndexConstraint _ic,
             bool _is_tensor) : basetype(_basetype),
                                ic(_ic),
                                is_tensor(_is_tensor) {}

      TSType meet(const TSType &other)
      {
        auto new_type = *this;
        if (basetype == BaseType::Float || other.basetype == BaseType::Float)
        {
          new_type.basetype = BaseType::Float;
        }
        else
        {
          new_type.basetype = BaseType::Int;
        }

        new_type.ic = ic.merge(other.ic);
        return new_type;
      }

      friend std::ostream &operator<<(std::ostream &out, const TSType &t)
      {
        if (!t.is_tensor)
        {
          out << "Scalar(";
        }
        else
        {
          out << "Tensor(IndexConstraint:\n"
              << t.ic << "(PossibleSolution=(";
          auto x = t.ic.solve();
          bool first_flag = true;
          // out << "Solution:\n";
          for (auto it = t.ic.index_names.begin(); it != t.ic.index_names.end(); ++it)
          {
            if (!first_flag)
            {
              out << ", ";
            }
            out << (*it) << ": " << x[*it];
            first_flag = false;
          }
          out << "), ";
        }
        out << "BaseType=";
        if (t.basetype == BaseType::Int)
        {
          out << "int";
        }
        else if (t.basetype == BaseType::Float)
        {
          out << "float";
        }
        out << ")";
        return out;
      }
    };

    class Typechecker : public Internal::IRVisitor
    {
    public:
      Typechecker() : IRVisitor() {}

      const TSType &check(const Expr &);

      void visit(Ref<const IntImm>) override;
      void visit(Ref<const FloatImm>) override;
      void visit(Ref<const Unary>) override;
      void visit(Ref<const Binary>) override;
      void visit(Ref<const Var>) override;

    private:
      // store types of visited IRNodes (since visit returns void)
      std::stack<TSType> type_stack;
      std::stack<std::string> op_stack; // consider using enum
    };

    class ArgVisitor : public Internal::IRVisitor
    {
    public:
      ArgVisitor() : IRVisitor() {}
      ArgVisitor(int _shape) : IRVisitor(), shape(_shape) {}
      IndexConstraint retrieve()
      {
        auto names = std::vector<std::string>();
        auto triplets = std::vector<Eigen::Triplet<double>>();
        auto shapes = Eigen::VectorXd(1);

        int i = 0;
        for (auto it = index_weight.begin(); it != index_weight.end(); ++it)
        {
          names.push_back(it->first);
          triplets.push_back(Eigen::Triplet<double>(0, i++, it->second));
        }
        shapes(0) = shape;
        auto ic = IndexConstraint(names, triplets, shapes);
        return ic;
      }

      // void visit(Ref<const IntImm>) override;
      // void visit(Ref<const FloatImm>) override;
      // void visit(Ref<const Unary>) override;
      void visit(Ref<const Binary>) override;
      void visit(Ref<const Index>) override;

    private:
      int shape;
      std::map<std::string, int> index_weight;
    };
  } // namespace Typechecker

} // namespace Boost

#endif // BOOST_TYPECHECKER_H