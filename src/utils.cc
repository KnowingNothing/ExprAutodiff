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

#include <sstream>
#include <vector>

#include "debug.h"
#include "IR.h"
#include "utils.h"


namespace Boost {

using namespace Internal;

namespace Utils {


std::string NameGenerator::unique_name(const std::string &name_hint) {
  std::ostringstream oss;
  oss << name_hint;
  if (name_map_.count(name_hint) != 0) {
    name_map_[name_hint]++; 
  } else {
    name_map_[name_hint] = 0;
  }
  oss << name_map_[name_hint];
  return oss.str();
}


bool NameGenerator::has_name(const std::string &name_hint) {
  return (name_map_.count(name_hint) != 0);
}


std::string NameGenerator::operator()(const std::string &name_hint) {
  return unique_name(name_hint);
}


Expr substitute_index(const Expr &expr,
  std::unordered_map<std::shared_ptr<const Index>, Expr> &vmap) {
    SubstituteIndex suber(vmap);
    return suber.substitute(expr);
}


Expr substitute_index_by_name(const Expr &expr,
  std::unordered_map<std::shared_ptr<const Index>, Expr> &vmap) {
    SubstituteIndexByName suber(vmap);
    return suber.substitute(expr);
}

}  // namespace Utils

}  // namespace Boost