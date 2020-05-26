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

#ifndef BOOST_SIMPLIFY_H
#define BOOST_SIMPLIFY_H

#include <memory>
#include <vector>
#include <unordered_map>

#include "debug.h"
#include "IR.h"
#include "IRVisitor.h"
#include "IRPrinter.h"
#include "IRMutator.h"
#include "IRFunctor.h"

namespace Boost {

using namespace Internal;


namespace Simplify {

class SimplifyUnitElement : IRMutator {
 public:
  using IRMutator::mutate;
  using IRMutator::visit;
  Expr visit(Ref<const Unary>) override;
  Expr visit(Ref<const Binary>) override;
};


Expr simplify_unit_element(const Expr &expr);

}  // namespace Simplify

}  // namespace Boost


#endif  // BOOST_SIMPLIFY_H