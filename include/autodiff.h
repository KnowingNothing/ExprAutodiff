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

#ifndef BOOST_AUTODIFF_H
#define BOOST_AUTODIFF_H

#include "IR.h"
#include "arith.h"

namespace Boost {

using namespace Internal;

namespace Autodiff {


// class SubstituteContext {
//  public:
//   SubstituteContext() { bound_begin = -1; }
//   std::vector<std::string> index_names;
//   std::unordered_map<std::string, Var> var_map;
//   int bound_begin;
//   std::unordered_map<std::string, ExtRange> range_map;
//   std::unordered_map<std::string, PrimExpr> var2expr;
//   // not found availabe map for PrimExpr by structural equal
//   // so use vector
//   // TODO: use optimized container
//   std::vector<std::pair<PrimExpr, std::string>> expr2var;
//   // std::unordered_map<PrimExpr, std::string, StructuralHash, CheckExprEqual> expr2var;

//   int find_bound(PrimExpr &expr);

//   std::string get_bound_name(PrimExpr &expr);

//   void add(std::string &name, Var var, PrimExpr expr, Utils::ExtRange range);

//   SubstituteContext copy() {
//     SubstituteContext ret;
//     for (auto name: index_names) {
//       ret.index_names.push_back(name);
//     }
//     for (auto kv : var_map) {
//       ret.var_map[kv.first] = kv.second;
//     }
//     ret.bound_begin = bound_begin;
//     for (auto kv : range_map) {
//       ret.range_map[kv.first] = kv.second;
//     }
//     for (auto kv : var2expr) {
//       ret.var2expr[kv.first] = kv.second;
//     }
//     for (auto kv : expr2var) {
//       ret.expr2var.push_back(std::make_pair(kv.first, kv.second));
//     }
//     return ret;
//   }

//   friend std::ostream &operator<<(std::ostream &out, SubstituteContext &context) {
//     out << "Show Substitute Context";
//     out << "\nindices:\n";
//     for (auto name : context.index_names) {
//       out << name << " ";
//     }
//     out << "\nvariable map:\n";
//     for (auto kv : context.var_map) {
//       out << kv.first << " = " << kv.second << "\n";
//     }
//     out << "\nrange map:\n";
//     for (auto kv : context.range_map) {
//       out << kv.first << " [" << kv.second.left << ", " << kv.second.right << "]\n";
//     }
//     out << "\nbindings:\n";
//     for (auto kv : context.var2expr) {
//       out << kv.first << " = " << kv.second << "\n";
//     }
//     out << "substitutions:\n";
//     for (auto kv : context.expr2var) {
//       out << kv.first << " -> " << kv.second << "\n";
//     }
//     return out;
//   }
// };


Stmt grad_stmt(Expr expr, Ref<const Var> grad_to, Ref<const Var> doutput);

}  // namespace Autodiff

}  // namespace Boost


#endif  // BOOST_AUTODIFF_H