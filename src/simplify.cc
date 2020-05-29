#include <cmath>

#include "simplify.h"
#include "utils.h"

namespace Boost {

using namespace Internal;

namespace Simplify {

Expr SimplifyUnitElement::visit(Ref<const Unary> op) {
  Expr new_value = mutate(op->a);
  if (op->op_type == UnaryOpType::Neg) {
    Ref<const IntImm> as_int = new_value.as<IntImm>();
    if (as_int.defined()) {
      return Expr(-as_int->value());
    }
    Ref<const FloatImm> as_float = new_value.as<FloatImm>();
    if (as_float.defined()) {
      return Expr(-as_float->value());
    }
    Ref<const Unary> as_unary = new_value.as<Unary>();
    if (as_unary.defined() && as_unary->op_type == UnaryOpType::Neg) {
      return as_unary->a;
    }
  } else if (op->op_type == UnaryOpType::Not) {
    Ref<const IntImm> as_int = new_value.as<IntImm>();
    if (as_int.defined()) {
      if (as_int->value() != 0) {
        return Utils::make_const(op->type(), 0);
      } else {
        return Utils::make_const(op->type(), 1);
      }
    }
    Ref<const Unary> as_unary = new_value.as<Unary>();
    if (as_unary.defined() && as_unary->op_type == UnaryOpType::Not) {
      return as_unary->a;
    }
  }
  return Unary::make(op->type(), op->op_type, new_value);
}


Expr SimplifyUnitElement::visit(Ref<const Binary> op) {
  Expr new_a = mutate(op->a);
  Expr new_b = mutate(op->b);
  Ref<const IntImm> a_as_int = new_a.as<IntImm>();
  Ref<const FloatImm> a_as_float = new_a.as<FloatImm>();
  Ref<const IntImm> b_as_int = new_b.as<IntImm>();
  Ref<const FloatImm> b_as_float = new_b.as<FloatImm>();

  if (op->op_type == BinaryOpType::Add) {
    if (a_as_int.defined() && b_as_int.defined()) {
      return Expr(a_as_int->value() + b_as_int->value());
    } else if (a_as_float.defined() && b_as_float.defined()) {
      return Expr(a_as_float->value() + b_as_float->value());
    } else if (a_as_int.defined()) {
      if (a_as_int->value() == 0) {
        return new_b;
      }
    } else if (b_as_int.defined()) {
      if (b_as_int->value() == 0) {
        return new_a;
      }
    } else if (a_as_float.defined()) {
      if (std::abs(a_as_float->value()) < 1e-20) {
        return new_b;
      }
    } else if (b_as_float.defined()) {
      if (std::abs(b_as_float->value() < 1e-20)) {
        return new_a;
      }
    }
  } else if (op->op_type == BinaryOpType::Sub) {
    if (a_as_int.defined() && b_as_int.defined()) {
      return Expr(a_as_int->value() - b_as_int->value());
    } else if (a_as_float.defined() && b_as_float.defined()) {
      return Expr(a_as_float->value() - b_as_float->value());
    } else if (a_as_int.defined()) {
      if (a_as_int->value() == 0) {
        return Unary::make(new_b->type(), UnaryOpType::Neg, new_b);
      }
    } else if (b_as_int.defined()) {
      if (b_as_int->value() == 0) {
        return new_a;
      }
    } else if (a_as_float.defined()) {
      if (std::abs(a_as_float->value()) < 1e-20) {
        return Unary::make(new_b->type(), UnaryOpType::Neg, new_b);
      }
    } else if (b_as_float.defined()) {
      if (std::abs(b_as_float->value() < 1e-20)) {
        return new_a;
      }
    }
  } else if (op->op_type == BinaryOpType::Mul) {
    if (a_as_int.defined() && b_as_int.defined()) {
      return Expr(a_as_int->value() * b_as_int->value());
    } else if (a_as_float.defined() && b_as_float.defined()) {
      return Expr(a_as_float->value() * b_as_float->value());
    } else if (a_as_int.defined()) {
      if (a_as_int->value() == 0) {
        return Utils::make_const(new_b->type(), 0);
      } else if (a_as_int->value() == 1) {
        return new_b;
      } else if (a_as_int->value() == -1) {
        return Unary::make(new_b->type(), UnaryOpType::Neg, new_b);
      }
    } else if (b_as_int.defined()) {
      if (b_as_int->value() == 0) {
        return Utils::make_const(new_a->type(), 0);
      } else if (b_as_int->value() == 1) {
        return new_a;
      } else if (b_as_int->value() == -1) {
        return Unary::make(new_a->type(), UnaryOpType::Neg, new_a);
      }
    } else if (a_as_float.defined()) {
      if (std::abs(a_as_float->value()) < 1e-20) {
        return Utils::make_const(new_b->type(), 0.0);
      } else if (std::abs(a_as_float->value() - 1) < 1e-20) {
        return new_b;
      } else if (std::abs(a_as_float->value() + 1) < 1e-20) {
        return Unary::make(new_b->type(), UnaryOpType::Neg, new_b);
      }
    } else if (b_as_float.defined()) {
      if (std::abs(b_as_float->value()) < 1e-20) {
        new_a->type();
        auto tmp = Utils::make_const(new_a->type(), 0.0);
        return tmp;
      } else if (std::abs(b_as_float->value() - 1) < 1e-20) {
        return new_a;
      } else if (std::abs(b_as_float->value() + 1) < 1e-20) {
        return Unary::make(new_a->type(), UnaryOpType::Neg, new_a);
      }
    }
  } else if (op->op_type == BinaryOpType::Div) {
    if (a_as_int.defined() && b_as_int.defined()) {
      return Expr(a_as_int->value() / b_as_int->value());
    } else if (a_as_float.defined() && b_as_float.defined()) {
      return Expr(a_as_float->value() / b_as_float->value());
    } else if (a_as_int.defined()) {
      if (a_as_int->value() == 0) {
        return Utils::make_const(new_b->type(), 0);
      }
    } else if (b_as_int.defined()) {
      if (b_as_int->value() == 1) {
        return new_a;
      } else if (b_as_int->value() == -1) {
        return Unary::make(new_a->type(), UnaryOpType::Neg, new_a);
      }
    } else if (a_as_float.defined()) {
      if (std::abs(a_as_float->value()) < 1e-20) {
        return Utils::make_const(new_b->type(), 0.0);
      }
    } else if (b_as_float.defined()) {
      if (std::abs(b_as_float->value() - 1) < 1e-20) {
        return new_a;
      } else if (std::abs(b_as_float->value() + 1) < 1e-20) {
        return Unary::make(new_a->type(), UnaryOpType::Neg, new_a);
      }
    }
  } else if (op->op_type == BinaryOpType::Mod) {
    if (a_as_int.defined() && b_as_int.defined()) {
      return Expr(a_as_int->value() % b_as_int->value());
    // } else if (a_as_float.defined() && b_as_float.defined()) {
    //   return Expr((a_as_float->value()) % (b_as_float->value()));
    } else if (a_as_int.defined()) {
      if (a_as_int->value() == 0) {
        return Utils::make_const(new_b->type(), 0);
      }
    } else if (b_as_int.defined()) {
      if (b_as_int->value() == 1) {
        return Utils::make_const(new_a->type(), 0);
      } else if (b_as_int->value() == -1) {
        return Utils::make_const(new_a->type(), 0);
      }
    } else if (a_as_float.defined()) {
      if (std::abs(a_as_float->value()) < 1e-20) {
        return Utils::make_const(new_b->type(), 0.0);
      }
    } else if (b_as_float.defined()) {
      if (std::abs(b_as_float->value() - 1) < 1e-20) {
        return Utils::make_const(new_a->type(), 0.0);
      } else if (std::abs(b_as_float->value() + 1) < 1e-20) {
        return Utils::make_const(new_a->type(), 0.0);
      }
    }
  } else if (op->op_type == BinaryOpType::FloorDiv) {
    if (a_as_int.defined() && b_as_int.defined()) {
      return Expr(a_as_int->value() / b_as_int->value());
    } else if (a_as_int.defined()) {
      if (a_as_int->value() == 0) {
        return Utils::make_const(new_b->type(), 0);
      }
    } else if (b_as_int.defined()) {
      if (b_as_int->value() == 1) {
        return new_a;
      } else if (b_as_int->value() == -1) {
        return Unary::make(new_a->type(), UnaryOpType::Neg, new_a);
      }
    }
  } else if (op->op_type == BinaryOpType::FloorMod) {
    if (a_as_int.defined() && b_as_int.defined()) {
      return Expr(a_as_int->value() % b_as_int->value());
    } else if (a_as_int.defined()) {
      if (a_as_int->value() == 0) {
        return Utils::make_const(new_b->type(), 0);
      }
    } else if (b_as_int.defined()) {
      if (b_as_int->value() == 1) {
        return Utils::make_const(new_a->type(), 0);
      } else if (b_as_int->value() == -1) {
        return Utils::make_const(new_a->type(), 0);
      }
    }
  }
  return Binary::make(op->type(), op->op_type, new_a, new_b);
}


Expr simplify_unit_element(const Expr &expr) {
  SimplifyUnitElement sue;
  return sue.mutate(expr);
}


}  // namespace Simplify

}  // namespace Boost