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

#ifndef BOOST_IRFUNCTOR_H
#define BOOST_IRFUNCTOR_H

#include "debug.h"
#include "IR.h"


#define VISIT_DEFAULT {LOG(ERROR) << "Trap into default visit of " << FUNCTOR << "."; throw; }


namespace Boost {

namespace Internal {

template <typename FType>
class ExprFunctor;


template <typename FType>
class StmtFunctor;


template <typename FType>
class GroupFunctor;


template <typename FType>
class OperationFunctor;


template <typename R, typename... Args>
class ExprFunctor<R(const Expr&, Args...)> {
 public:
  #define FUNCTOR "ExprFunctor"
  virtual R visit_expr(const Expr &expr, Args... args) {
    #define X(T)                                                  \
      if (expr.node_type() == T::node_type_) {                    \
        return visit(expr.as<T>(), std::forward<Args>(args)...);  \
      }
      IRNODE_EXPR_TYPE
    #undef X
    VISIT_DEFAULT
   }
   
   virtual R visit(Ref<const IntImm> op, Args... args) VISIT_DEFAULT
   virtual R visit(Ref<const UIntImm> op, Args... args) VISIT_DEFAULT
   virtual R visit(Ref<const FloatImm> op, Args... args) VISIT_DEFAULT
   virtual R visit(Ref<const StringImm> op, Args... args) VISIT_DEFAULT
   virtual R visit(Ref<const Unary> op, Args... args) VISIT_DEFAULT
   virtual R visit(Ref<const Binary> op, Args... args) VISIT_DEFAULT
   virtual R visit(Ref<const Select> op, Args... args) VISIT_DEFAULT
   virtual R visit(Ref<const Compare> op, Args... args) VISIT_DEFAULT
   virtual R visit(Ref<const Call> op, Args... args) VISIT_DEFAULT
   virtual R visit(Ref<const Var> op, Args... args) VISIT_DEFAULT
   virtual R visit(Ref<const Cast> op, Args... args) VISIT_DEFAULT
   virtual R visit(Ref<const Ramp> op, Args... args) VISIT_DEFAULT
   virtual R visit(Ref<const Index> op, Args... args) VISIT_DEFAULT
   virtual R visit(Ref<const Dom> op, Args... args) VISIT_DEFAULT
  #undef FUNCTOR
 private:
};

template <typename R, typename... Args>
class StmtFunctor<R(const Stmt&, Args...)> {
 public:
  #define FUNCTOR "StmtFunctor"
   virtual R visit_stmt(const Stmt& stmt, Args... args) {
     #define X(T)                                                  \
      if (stmt.node_type() == T::node_type_) {                    \
        return visit(stmt.as<T>(), std::forward<Args>(args)...);  \
      }
      IRNODE_STMT_TYPE
    #undef X
    VISIT_DEFAULT
   }
   
   virtual R visit(Ref<const LoopNest> op, Args... args) VISIT_DEFAULT
   virtual R visit(Ref<const IfThenElse> op, Args... args) VISIT_DEFAULT
   virtual R visit(Ref<const Move> op, Args... args) VISIT_DEFAULT
  #undef FUNCTOR
 private:
};


template <typename R, typename... Args>
class GroupFunctor<R(const Group&, Args...)> {
 public:
  #define FUNCTOR "GroupFunctor"
   virtual R visit_group(const Group& group, Args... args) {
     #define X(T)                                                  \
      if (group.node_type() == T::node_type_) {                    \
        return visit(group.as<T>(), std::forward<Args>(args)...);  \
      }
      IRNODE_GROUP_TYPE
    #undef X
    VISIT_DEFAULT
   }
   
   virtual R visit(Ref<const Kernel> op, Args... args) VISIT_DEFAULT
  #undef FUNCTOR
 private:
};


template <typename R, typename... Args>
class OperationFunctor<R(const Operation&, Args...)> {
 public:
  #define FUNCTOR "OperationFunctor"
   virtual R visit_operation(const Operation& operation, Args... args) {
     #define X(T)                                                  \
      if (operation.node_type() == T::node_type_) {                    \
        return visit(operation.as<T>(), std::forward<Args>(args)...);  \
      }
      IRNODE_OPERATION_TYPE
    #undef X
    VISIT_DEFAULT
   }
   
   virtual R visit(Ref<const PlaceholderOp> op, Args... args) VISIT_DEFAULT
   virtual R visit(Ref<const ComputeOp> op, Args... args) VISIT_DEFAULT
  #undef FUNCTOR
 private:
};

}  // namespace Internal

}  // namespace Boost


#endif  // BOOST_IRFUNCTOR_H