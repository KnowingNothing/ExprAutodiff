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

#include "IR.h"


namespace Boost {

namespace Internal {

template <typename FType>
class IRFunctor;


template <typename R, typename... Args>
class IRFunctor<R(Args...)> {
 public:
    virtual R visit(Ref<const IntImm>, Args... args);
    virtual R visit(Ref<const UIntImm>, Args... args);
    virtual R visit(Ref<const FloatImm>, Args... args);
    virtual R visit(Ref<const StringImm>, Args... args);
    virtual R visit(Ref<const Unary>, Args... args);
    virtual R visit(Ref<const Binary>, Args... args);
    virtual R visit(Ref<const Select>, Args... args);
    virtual R visit(Ref<const Compare>, Args... args);
    virtual R visit(Ref<const Call>, Args... args);
    virtual R visit(Ref<const Var>, Args... args);
    virtual R visit(Ref<const Cast>, Args... args);
    virtual R visit(Ref<const Ramp>, Args... args);
    virtual R visit(Ref<const Index>, Args... args);
    virtual R visit(Ref<const Dom>, Args... args);
    virtual R visit(Ref<const LoopNest>, Args... args);
    virtual R visit(Ref<const IfThenElse>, Args... args);
    virtual R visit(Ref<const Move>, Args... args);
    virtual R visit(Ref<const Kernel>, Args... args);
    virtual R visit(Ref<const PlaceholderOp>, Args... args);
    virtual R visit(Ref<const ComputeOp>, Args... args);
 private:
};

}  // namespace Internal

}  // namespace Boost


#endif  // BOOST_IRFUNCTOR_H