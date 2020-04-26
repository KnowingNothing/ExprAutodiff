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

#include "debug.h"
#include "codegen_C.h"

namespace Boost {

using namespace Internal;

namespace codegen {


std::string CodeGen_C::print_type(const Type &t) {
  std::ostringstream out;
  switch (t.code)
  {
  case Internal::TypeCode::Int:
    out << "int" << t.bits << "_t";
    break;
  case Internal::TypeCode::UInt:
    out << "uint" << t.bits << "_t";
    break;
  case Internal::TypeCode::Float:
    if ((int)t.bits == 32) {
      out << "float";
    } else if ((int)t.bits == 64) {
      out << "double";
    } else {
      LOG(ERROR) << "No support for float of bits: " << t.bits << ".";
    }
    break;
  case Internal::TypeCode::String:
    out << "";
    CHECK((int)t.bits == 1, "Not support string with bits: %ud\n", t.bits);
    break;
  case Internal::TypeCode::Handle:
    out << "(void*)";
    CHECK((int)t.bits == 1, "Not support handle with bits: %ud\n", t.bits);
    break;
  default: LOG(ERROR) << "Type unknown: " << t;
    break;
  }
  CHECK((int)t.is_scalar(), "Do not support vector type\n");
  return out.str();
}


std::string CodeGen_C::print(const Expr &expr) {
  oss.str("");
  oss.clear();
  expr.visit_expr(this);
  return oss.str();
}

std::string CodeGen_C::print(const Stmt &stmt) {
  oss.str("");
  oss.clear();
  stmt.visit_stmt(this);
  return oss.str();
}


std::string CodeGen_C::print(const Group &group) {
  oss.str("");
  oss.clear();
  group.visit_group(this);
  return oss.str();
}


void CodeGen_C::visit(Ref<const IntImm> op) {
  oss << op->value();
}


void CodeGen_C::visit(Ref<const UIntImm> op) {
  oss << op->value();
}


void CodeGen_C::visit(Ref<const FloatImm> op) {
  oss << op->value();
}


void CodeGen_C::visit(Ref<const StringImm> op) {
  oss << op->value();
}


void CodeGen_C::visit(Ref<const Unary> op) {
  if (op->op_type == UnaryOpType::Neg) {
      oss << "-";
  } else if (op->op_type == UnaryOpType::Not) {
      oss << "!";
  }
  (op->a).visit_expr(this);
}


void CodeGen_C::visit(Ref<const Binary> op) {
  (op->a).visit_expr(this);
  if (op->op_type == BinaryOpType::Add) {
      oss << " + ";
  } else if (op->op_type == BinaryOpType::Sub) {
      oss << " - ";
  } else if (op->op_type == BinaryOpType::Mul) {
      oss << " * ";
  } else if (op->op_type == BinaryOpType::Div) {
      oss << " / ";
  } else if (op->op_type == BinaryOpType::Mod) {
      oss << " % ";
  } else if (op->op_type == BinaryOpType::FloorDiv) {
      oss << " / ";
  } else if (op->op_type == BinaryOpType::FloorMod) {
      oss << " % ";
  } else if (op->op_type == BinaryOpType::And) {
      oss << " && ";
  } else if (op->op_type == BinaryOpType::Or) {
      oss << " || ";
  } else {
    LOG(ERROR) << "Unknown binay OpType.";
  }
  (op->b).visit_expr(this);
}


void CodeGen_C::visit(Ref<const Compare> op) {
  (op->a).visit_expr(this);
  if (op->op_type == CompareOpType::LT) {
      oss << " < ";
  } else if (op->op_type == CompareOpType::LE) {
      oss << " <= ";
  } else if (op->op_type == CompareOpType::EQ) {
      oss << " == ";
  } else if (op->op_type == CompareOpType::GE) {
      oss << " >= ";
  } else if (op->op_type == CompareOpType::GT) {
      oss << " > ";
  } else if (op->op_type == CompareOpType::NE) {
      oss << " != ";
  }
  (op->b).visit_expr(this);
}


void CodeGen_C::visit(Ref<const Select> op) {
  LOG(ERROR) << "Find select, should be preprocessed to if_then_else.";
}


void CodeGen_C::visit(Ref<const Call> op) {
  oss << "(" << op->func_name;
  for (size_t i = 0; i < op->args.size(); ++i) {
      oss << ", ";
      op->args[i].visit_expr(this);
  }
  oss << ")";
}


void CodeGen_C::visit(Ref<const Cast> op) {
  oss << "((" << print_type(op->new_type) << ")";
  (op->val).visit_expr(this);
  oss << ")";
}


void CodeGen_C::visit(Ref<const Ramp> op) {
  LOG(ERROR) << "Find ramp, not supported in C/C++ source code.";
}


void CodeGen_C::visit(Ref<const Var> op) {
  if (print_arg) {
    oss << print_type(op->type());
    oss << " (&" << op->name << ")";
    for (size_t i = 0; i < op->shape.size(); ++i) {
      oss << "[";
      oss << op->shape[i];
      oss << "]";
    }
  } else { 
    oss << op->name;
    for (size_t i = 0; i < op->args.size(); ++i) {
      oss << "[";
      op->args[i].visit_expr(this);
      oss << "]";
    }
  }
}


void CodeGen_C::visit(Ref<const Dom> op) {
    LOG(ERROR) << "Find dom, no support in C/C++ source code.";
}


void CodeGen_C::visit(Ref<const Index> op) {
    oss << op->name;
}


void CodeGen_C::visit(Ref<const LoopNest> op) {
    print_range = true;
    for (auto index : op->index_list) {
        print_indent();
        oss << "for (";
        oss << print_type(index.type()) << " ";
        index.visit_expr(this);
        oss << " = 0; ";
        index.visit_expr(this);
        std::shared_ptr<const Index> as_index = index.as<Index>();
        CHECK(as_index.get() != nullptr, "Expect Index");
        std::shared_ptr<const Dom> dom = as_index->dom.as<Dom>();
        CHECK(dom.get() != nullptr, "Expect Dom");
        oss << " < ";
        dom->extent.visit_expr(this);
        oss << "; ";
        index.visit_expr(this);
        oss << " = ";
        index.visit_expr(this);
        oss << " + 1) ";
        oss << "{\n";
        enter();
    }
    print_range = false;
    for (auto body : op->body_list) {
        body.visit_stmt(this);
    }
    for (auto index : op->index_list) {
        exit();
        print_indent();
        oss << "}\n";
    }
}


void CodeGen_C::visit(Ref<const IfThenElse> op) {
    print_indent();
    oss << "if (";
    (op->cond).visit_expr(this);
    oss << ") {\n";
    enter();
    (op->true_case).visit_stmt(this);
    exit();
    print_indent();
    oss << "} else {\n";
    enter();
    (op->false_case).visit_stmt(this);
    exit();
    print_indent();
    oss << "}\n";
}


void CodeGen_C::visit(Ref<const Move> op) {
    print_indent();
    (op->dst).visit_expr(this);
    oss << " = ";
    (op->src).visit_expr(this);
    oss << "\n";
}


void CodeGen_C::visit(Ref<const Kernel> op) {
    print_indent();
    if (op->kernel_type == KernelType::CPU) {
        oss << "void";
    } else if (op->kernel_type == KernelType::GPU) {
        LOG(ERROR) << "Can't generate GPU kernel by C/C++ source code generation.";
    }
    oss << " " << op->name << "(";
    print_arg = true;
    for (size_t i = 0; i < op->inputs.size(); ++i) {
        op->inputs[i].visit_expr(this);
        if (i < op->inputs.size() - 1) {
            oss << ", ";
        }
    }
    for (size_t i = 0; i < op->outputs.size(); ++i) {
        oss << ", ";
        op->outputs[i].visit_expr(this);
    }
    print_arg = false;
    oss << ") {\n";
    enter();
    for (auto stmt : op->stmt_list) {
        stmt.visit_stmt(this);
    }
    exit();
    oss << "}\n";
}

void CodeGen_C::visit(Ref<const PlaceholderOp> op){
    LOG(ERROR) << "Find placeholder.";
}

void CodeGen_C::visit(Ref<const ComputeOp> op){
    LOG(ERROR) << "Find compute_op.";
}

}  // namespace codegen

}  // namespace Boost
