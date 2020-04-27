// /*
//  * MIT License
//  * 
//  * Copyright (c) 2020 Size Zheng

//  * Permission is hereby granted, free of charge, to any person obtaining a copy
//  * of this software and associated documentation files (the "Software"), to deal
//  * in the Software without restriction, including without limitation the rights
//  * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  * copies of the Software, and to permit persons to whom the Software is
//  * furnished to do so, subject to the following conditions:
 
//  * The above copyright notice and this permission notice shall be included in all
//  * copies or substantial portions of the Software.
 
//  * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  * SOFTWARE.
// */

// #include "IRFunctor.h"

// namespace Boost {

// namespace Internal {


// template<typename R, typename... Args>
// R ExprFunctor<R(const Expr&, Args...)>::visit(Ref<const IntImm> op, Args... args) {
//     return;
// }


// template<typename R, typename... Args>
// R ExprFunctor<R(const Expr&, Args...)>::visit(Ref<const IntImm> op, Args... args) {
//     return;
// }


// template<typename R, typename... Args>
// R ExprFunctor<R(const Expr&, Args...)>::visit(Ref<const UIntImm> op, Args... args) {
//     return;
// }


// template<typename R, typename... Args>
// R ExprFunctor<R(const Expr&, Args...)>::visit(Ref<const FloatImm> op, Args... args) {
//     return;
// }


// template<typename R, typename... Args>
// R ExprFunctor<R(const Expr&, Args...)>::visit(Ref<const StringImm> op, Args... args) {
//     return;
// }


// template<typename R, typename... Args>
// R ExprFunctor<R(const Expr&, Args...)>::visit(Ref<const Unary> op, Args... args) {
//     visit(op->a, std::forward<Args>(args)...);
//     return;
// }


// template<typename R, typename... Args>
// R ExprFunctor<R(const Expr&, Args...)>::visit(Ref<const Binary> op, Args... args) {
//     visit(op->a, std::forward<Args>(args)...);
//     visit(op->b, std::forward<Args>(args)...);
//     return;
// }


// template<typename R, typename... Args>
// R ExprFunctor<R(const Expr&, Args...)>::visit(Ref<const Compare> op, Args... args) {
//     visit(op->a, std::forward<Args>(args)...);
//     visit(op->b, std::forward<Args>(args)...);
//     return;
// }


// template<typename R, typename... Args>
// R ExprFunctor<R(const Expr&, Args...)>::visit(Ref<const Select> op, Args... args) {
//     visit(op->cond, std::forward<Args>(args)...);
//     visit(op->true_value, std::forward<Args>(args)...);
//     visit(op->false_value, std::forward<Args>(args)...);
//     return;
// }


// template<typename R, typename... Args>
// R ExprFunctor<R(const Expr&, Args...)>::visit(Ref<const Call> op, Args... args) {
//     for (auto arg : op->args) {
//         visit(arg, std::forward<Args>(args)...);
//     }
//     return;
// }


// template<typename R, typename... Args>
// R ExprFunctor<R(const Expr&, Args...)>::visit(Ref<const Cast> op, Args... args) {
//     visit(op->val, std::forward<Args>(args)...);
//     return;
// }


// template<typename R, typename... Args>
// R ExprFunctor<R(const Expr&, Args...)>::visit(Ref<const Ramp> op, Args... args) {
//     visit(op->base, std::forward<Args>(args)...);
//     return;
// }


// template<typename R, typename... Args>
// R ExprFunctor<R(const Expr&, Args...)>::visit(Ref<const Var> op, Args... args) {
//     for (auto arg : op->args) {
//         visit(arg, std::forward<Args>(args)...);
//     }
//     return;
// }


// template<typename R, typename... Args>
// R ExprFunctor<R(const Expr&, Args...)>::visit(Ref<const Dom> op, Args... args) {
//     visit(op->begin, std::forward<Args>(args)...);
//     visit(op->extent, std::forward<Args>(args)...);
//     return;
// }


// template<typename R, typename... Args>
// R ExprFunctor<R(const Expr&, Args...)>::visit(Ref<const Index> op, Args... args) {
//     visit(op->dom, std::forward<Args>(args)...);
//     return;
// }


// template<typename R, typename... Args>
// R StmtFunctor<R(const Stmt&, Args...)>::visit(Ref<const LoopNest> op, Args... args) {
//     // for (auto index : op->index_list) {
//     //     visit(index, std::forward<Args>(args)...);
//     // }
//     for (auto body : op->body_list) {
//         visit(body, std::forward<Args>(args)...);
//     }
//     return;
// }


// template<typename R, typename... Args>
// R StmtFunctor<R(const Stmt&, Args...)>::visit(Ref<const IfThenElse> op, Args... args) {
//     // visit(op->cond, std::forward<Args>(args)...);
//     visit(op->true_case);
//     visit(op->false_case);
//     return;
// }


// template<typename R, typename... Args>
// R StmtFunctor<R(const Stmt&, Args...)>::visit(Ref<const Move> op, Args... args) {
//     // visit(op->dst, std::forward<Args>(args)...);
//     // visit(op->src, std::forward<Args>(args)...);
//     return;
// }


// template<typename R, typename... Args>
// R GroupFunctor<R(const Group&, Args...)>::visit(Ref<const Kernel> op, Args... args) {
//     // for (auto expr : op->inputs) {
//     //     visit(expr, std::forward<Args>(args)...);
//     // }
//     // for (auto expr : op->outputs) {
//     //     visit(expr, std::forward<Args>(args)...);
//     // }
//     // for (auto stmt : op->stmt_list) {
//     //     visit(stmt, std::forward<Args>(args)...);
//     // }
//     return;
// }

// template<typename R, typename... Args>
// R OperationFunctor<R(const Operation&, Args...)>::visit(Ref<const PlaceholderOp> op, Args... args) {
//     // for (auto arg : op->args) {
//     //     visit(arg, std::forward<Args>(args)...);
//     // }
// }

// template<typename R, typename... Args>
// R OperationFunctor<R(const Operation&, Args...)>::visit(Ref<const ComputeOp> op, Args... args) {
//     // for (auto index : op->index_list) {
//     //     visit(index, std::forward<Args>(args)...);
//     // }
//     // for (auto body : op->body_list) {
//     //     visit(body, std::forward<Args>(args)...);
//     // }
// }


// }  // namespace Internal

// }  // namespace Boost
