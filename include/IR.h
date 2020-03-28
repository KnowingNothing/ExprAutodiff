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

#include <memory>
#include <string>

#include "type.h"
#include "arith.h"

namespace Boost {

namespace Internal {

/**
 * This class is inspired by Halide IntrusivePtr
 * The difference is that we use std::shared_ptr
 */
template <typename T>
class Ref {

 protected:
    std::shared_ptr<T> ptr = nullptr;
 public:
    Ref() {}

    Ref(Ref<T> &other) : ptr(other.ptr) {}

    Ref(Ref<T> &&other) : ptr(std::move(other.ptr)) {}

    Ref(std::shared_ptr<T> _ptr) : ptr(_ptr) {}

    bool defined() { return ptr != nullptr; }

    T *get() const { return ptr.get(); }

    T &operator*() const { return *ptr; }

    T *operator->() const { return ptr.operator->(); }

    Ref<T> &operator=(Ref<T> &b) {
        this->ptr = b.ptr;
        return *this;
    }

    Ref<T> &operator=(Ref<T> &&b) {
        swap(this->ptr, b.ptr);
        return *this;
    }

    bool operator<(Ref<T> &b) const {
        /* Don't directly compare shared_ptr, for C++20 removes operator< */
        return this->get() < b.get();
    }
};


enum class IRNodeType : short {
    // Stmts
    For,
    Seq,
    LetStmt,
    IfThenElse,
    Move,
    Allocate,
    // Exprs
    Unary,
    Binary,
    Select,
    Compare,
    Call,
    Let,
    Var,
    Cast,
    Ramp,
    Index,
    IntImm,
    UIntImm,
    FloatImm,
    StringImm,
    Dom
};


class IRNode {
 public:
    IRNode(IRNodeType _type) : _node_type(_type) {}

    IRNodeType node_type() const {
        return this->_node_type;
    }
 private:
    IRNodeType _node_type;
};


class IRNodeRef : public Ref<const IRNode> {
 public:
    IRNodeRef() {}
    IRNodeRef(std::shared_ptr<const IRNode> _ptr) : Ref<const IRNode>(_ptr) {}

    IRNodeType node_type() {
        return this->get()->node_type();
    }
};


class ExprNode : public IRNode {
 private:
    Type type_;
 public:
    ExprNode(Type _type, IRNodeType node_type) : IRNode(node_type), type_(_type) {} 

    Type type() {
        return type_;
    }
};


class StmtNode : public IRNode {
 private:

 public:
    StmtNode(IRNodeType _type) : IRNode(_type) {}
};


class IntImm : public ExprNode {
 private:
    int64_t value_;
 public:
    IntImm(Type _type, int64_t _value) : ExprNode(_type, IRNodeType::IntImm), value_(_value)  {}

    int64_t &value() {
        return value_;
    }

    static std::shared_ptr<const IntImm> make(Type t, int64_t _value) {
        return std::make_shared<const IntImm>(t, _value);
    }

    static const IRNodeType node_type_ = IRNodeType::IntImm;
};


class UIntImm : public ExprNode {
 private:
    uint64_t value_;
 public:
    UIntImm(Type _type, int64_t _value) : ExprNode(_type, IRNodeType::UIntImm), value_(_value) {}

    /**
     * May need consider bits
     */ 
    uint64_t &value() {
        return value_;
    }

    static std::shared_ptr<const UIntImm> make(Type t, uint64_t _value) {
        return std::make_shared<const UIntImm>(t, _value);
    }

    static const IRNodeType node_type_ = IRNodeType::UIntImm;
};


class FloatImm : public ExprNode {
 private:
    double value_;
 public:
    FloatImm(Type _type, double _value) : ExprNode(_type, IRNodeType::FloatImm), value_(_value) {}

    /**
     * May need consider bits
     */ 
    double &value() {
        return value_;
    }

    static std::shared_ptr<const FloatImm> make(Type t, double _value) {
        return std::make_shared<const FloatImm>(t, _value);
    }

    static const IRNodeType node_type_ = IRNodeType::FloatImm;
};


class StringImm : public ExprNode {
 private:
    std::string value_;
 public:
    StringImm(Type _type, std::string _value) : ExprNode(_type, IRNodeType::StringImm), value_(_value) {}

    std::string &value() {
        return value_;
    }

    static std::shared_ptr<const StringImm> make(Type t, std::string _value) {
        return std::make_shared<const StringImm>(t, _value);
    }

    static const IRNodeType node_type_ = IRNodeType::StringImm;
};


class Expr : public Ref<const ExprNode> {
 public:
    Expr() : Ref<const ExprNode>() {}
    
    Expr(Expr &other) : Ref<const ExprNode>(other) {}

    Expr(Expr &&other) : Ref<const ExprNode>(std::move(other)) {}

    Expr(std::shared_ptr<const ExprNode> _ptr) : Ref<const ExprNode>(_ptr) {}

    IRNodeType node_type() const {
        return this->get()->node_type();
    }

    template <typename T>
    std::shared_ptr<const T> as() {
        if (this->node_type() == T::node_type_) {
            return std::static_pointer_cast<T>(this->get());
        }
        return nullptr;
    }
};


class Stmt : public Ref<const StmtNode> {
 public:
    Stmt() : Ref<const StmtNode>() {}

    Stmt(Stmt &other) : Ref<const StmtNode>(other) {}

    Stmt(Stmt &&other) : Ref<const StmtNode>(std::move(other)) {}

    Stmt(std::shared_ptr<const StmtNode> _ptr) : Ref<const StmtNode>(_ptr) {}

    IRNodeType node_type() const {
        return this->get()->node_type();
    }

    template <typename T>
    std::shared_ptr<const T> as() {
        if (this->node_type() == T::node_type_) {
            return std::static_pointer_cast<T>(this->get());
        }
        return nullptr;
    }
};


enum class UnaryOpType : uint8_t {
    Neg,
    Not
};


class Unary : public ExprNode {
 public:
    UnaryOpType op_type;
    Expr a;

    Unary(Type _type, UnaryOpType _op_type, Expr _a) : ExprNode(_type, IRNodeType::Unary),
        op_type(_op_type), a(_a) {}

    static std::shared_ptr<const Unary> make(Type t, UnaryOpType _op_type, Expr _a) {
        return std::make_shared<const Unary>(t, _op_type, _a);
    }

    static const IRNodeType node_type_ = IRNodeType::Unary;
};


enum class BinaryOpType : uint8_t {
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    And,
    Or
};


class Binary : public ExprNode {
 public:
    BinaryOpType op_type;
    Expr a, b;

    Binary(Type _type, BinaryOpType _op_type, Expr _a, Expr _b) : ExprNode(_type, IRNodeType::Binary),
        op_type(_op_type), a(_a), b(_b) {}

    static std::shared_ptr<const Binary> make(Type t, BinaryOpType _op_type, Expr _a, Expr _b) {
        return std::make_shared<const Binary>(t, _op_type, _a, _b);
    }

    static const IRNodeType node_type_ = IRNodeType::Binary;
};


enum class CompareOpType : uint8_t {
    LT,
    LE,
    EQ,
    NE,
    GE,
    GT
};


class Compare : public ExprNode {
 public:
    CompareOpType op_type;
    Expr a, b;

    Compare(Type _type, CompareOpType _op_type, Expr _a, Expr _b) : ExprNode(_type, IRNodeType::Compare),
        op_type(_op_type), a(_a), b(_b) {}

    static std::shared_ptr<const Compare> make(Type t, CompareOpType _op_type, Expr _a, Expr _b) {
        return std::make_shared<const Compare>(t, _op_type, _a, _b);
    }

    static const IRNodeType node_type_ = IRNodeType::Compare;
};


class Select : public ExprNode {
 public:
    Expr cond;
    Expr true_value, false_value;

    Select(Type _type, Expr _cond, Expr _true_value, Expr _false_value) : ExprNode(_type, IRNodeType::Select),
        cond(_cond), true_value(_true_value), false_value(_false_value) {}

    static std::shared_ptr<const Select> make(Type t, Expr _cond, Expr _true_value, Expr _false_value) {
        return std::make_shared<const Select>(t, _cond, _true_value, _false_value);
    }

    static const IRNodeType node_type_ = IRNodeType::Select;
};


enum class CallType : uint8_t {
    Pure,
    SideEffect
};


class Call : public ExprNode {
 public:
    std::vector<Expr> args;
    std::string func_name;
    CallType call_type;

    Call(Type _type, std::vector<Expr> &_args, std::string _func_name, CallType _call_type) : ExprNode(_type, IRNodeType::Call),
        args(_args), func_name(_func_name), call_type(_call_type) {}

    static std::shared_ptr<const Call> make(Type t, std::vector<Expr> &_args, std::string _func_name, CallType _call_type) {
        return std::make_shared<const Call>(t, _args, _func_name, _call_type);
    }

    static const IRNodeType node_type_ = IRNodeType::Call;
};


class Let : public ExprNode {
 public:
    std::string name;
    Expr val;
    Expr body;

    Let(Type _type, std::string _name, Expr _val, Expr _body) : ExprNode(_type, IRNodeType::Let),
        name(_name), val(_val), body(_body) {}

    static std::shared_ptr<const Let> make(Type t, std::string _name, Expr _val, Expr _body) {
        return std::make_shared<const Let>(t, _name, _val, _body);
    }

    static const IRNodeType node_type_ = IRNodeType::Let;
};


class Cast : public ExprNode {
 public:
    Type new_type;
    Expr val;

    Cast(Type _type, Type _new_type, Expr _val) : ExprNode(_type, IRNodeType::Cast),
        new_type(_new_type), val(_val) {}

    static std::shared_ptr<const Cast> make(Type t, Type _new_type, Expr _val) {
        return std::make_shared<const Cast>(t, _new_type, _val);
    }

    static const IRNodeType node_type_ = IRNodeType::Cast;
};


class Ramp : public ExprNode {
 public:
    Expr base;
    uint16_t stride;
    uint16_t lanes;

    Ramp(Type _type, Expr _base, uint16_t _stride, uint16_t _lanes) : ExprNode(_type, IRNodeType::Ramp),
        base(_base), stride(_stride), lanes(_lanes) {}

    static std::shared_ptr<const Ramp> make(Type t, Expr _base, uint16_t _stride, uint16_t _lanes) {
        return std::make_shared<const Ramp>(t, _base, _stride, _lanes);
    }

    static const IRNodeType node_type_ = IRNodeType::Ramp;
};


class Var : public ExprNode {
 public:
    std::vector<Expr> args;
    std::vector<size_t> shape;

    Var(Type _type, std::vector<Expr> &_args, std::vector<size_t> &_shape) : ExprNode(_type, IRNodeType::Var),
        args(_args), shape(_shape) {}

    static std::shared_ptr<const Var> make(Type t, std::vector<Expr> &_args, std::vector<size_t> &_shape) {
        return std::make_shared<const Var>(t, _args, _shape);
    }

    static const IRNodeType node_type_ = IRNodeType::Var;
};


class Dom : public ExprNode {
 public:
    Expr begin;
    Expr extent;

    Dom(Type _type, Expr _begin, Expr _extent) : ExprNode(_type, IRNodeType::Dom), begin(_begin), extent(_extent) {}

    static std::shared_ptr<const Dom> make(Type t, Expr _begin, Expr _extent) {
        return std::make_shared<const Dom>(t, _begin, _extent);
    }

    static const IRNodeType node_type_ = IRNodeType::Dom;
};


enum class IndexType : uint8_t {
    Spatial,
    Reduce
};


class Index : public ExprNode {
 public:
    Expr dom;
    IndexType index_type;

    Index(Type _type, Expr _dom, IndexType _index_type) : ExprNode(_type, IRNodeType::Index),
        dom(_dom), index_type(_index_type) {}

    static std::shared_ptr<const Index> make(Type t, Expr _dom, IndexType _index_type) {
        return std::make_shared<const Index>(t, _dom, _index_type);
    }

    static const IRNodeType node_type_ = IRNodeType::Index;
};


enum class ForType : uint8_t {
    Serial,
    Block,
    Thread,
    Vectorized,
    Unrolled,
};


class For : public StmtNode {
 public:
    std::string for_name;
    Expr init;
    Expr cond;
    Expr update;
    Stmt body;
    ForType for_type;

    For(std::string _for_name, Expr _init, Expr _cond, Expr _update, Stmt _body, ForType _for_type) :
        StmtNode(IRNodeType::For), for_name(_for_name), init(_init), cond(_cond), update(_update),
        body(_body), for_type(_for_type) {}

    static std::shared_ptr<const For> make(std::string _for_name, Expr _init, Expr _cond, Expr _update,
        Stmt _body, ForType _for_type) {
        return std::make_shared<const For>(_for_name, _init, _cond, _update, _body, _for_type);
    }

    static const IRNodeType node_type_ = IRNodeType::For;
};


class Seq : public StmtNode {
 public:
    Expr first;
    Expr rest;

    Seq(Expr _first, Expr _rest) :
        StmtNode(IRNodeType::Seq), first(_first), rest(_rest) {}

    static std::shared_ptr<const Seq> make(Expr _first, Expr _rest) {
        return std::make_shared<const Seq>(_first, _rest);
    }

    static const IRNodeType node_type_ = IRNodeType::Seq;
};


class LetStmt : public StmtNode {
 public:
    std::string name;
    Expr val;
    Stmt body;

    LetStmt(std::string _name, Expr _val, Stmt _body) :
        StmtNode(IRNodeType::LetStmt), val(_val), body(_body) {}

    static std::shared_ptr<const LetStmt> make(std::string _name, Expr _val, Stmt _body) {
        return std::make_shared<const LetStmt>(_name, _val, _body);
    }

    static const IRNodeType node_type_ = IRNodeType::LetStmt;
};


class IfThenElse : public StmtNode {
 public:
    Expr cond;
    Stmt true_case;
    Stmt false_case;

    IfThenElse(Expr _cond, Stmt _true_case, Stmt _false_case) :
        StmtNode(IRNodeType::IfThenElse), cond(_cond), true_case(_true_case), false_case(_false_case) {}

    static std::shared_ptr<const IfThenElse> make(Expr _cond, Stmt _true_case, Stmt _false_case) {
        return std::make_shared<const IfThenElse>(_cond, _true_case, _false_case);
    }

    static const IRNodeType node_type_ = IRNodeType::IfThenElse;
};


enum class MoveType : uint8_t {
    DeviceToHost,
    HostToDevice,
    MemToShared,
    SharedToMem,
    MemToLocal,
    LocalToMem,
    SharedToLocal,
    LocalToShared,
    SharedToShared,
    MemToMem,
    LocalToLocal
};


class Move : public StmtNode {
 public:
    Expr dst;
    Expr src;
    MoveType move_type;

    Move(Expr _dst, Expr _src, MoveType _move_type) :
        StmtNode(IRNodeType::Move), dst(_dst), src(_src), move_type(_move_type) {}

    static std::shared_ptr<const Move> make(Expr _dst, Expr _src, MoveType _move_type) {
        return std::make_shared<const Move>(_dst, _src, _move_type);
    }

    static const IRNodeType node_type_ = IRNodeType::Move;
};


enum class MemType : uint8_t {
    Global,
    Shared,
    Local
};


class Allocate : public StmtNode {
 public:
    std::string name;
    Arith::Bounds bounds;
    Stmt body;
    MemType mem_type;

    Allocate(std::string _name, Arith::Bounds &_bounds, Stmt _body, MemType _mem_type) :
        StmtNode(IRNodeType::Allocate), name(_name), bounds(_bounds), body(_body), mem_type(_mem_type) {}

    static std::shared_ptr<const Allocate> make(std::string _name, Arith::Bounds &_bounds, Stmt _body, MemType _mem_type) {
        return std::make_shared<const Allocate>(_name, _bounds, _body, _mem_type);
    }

    static const IRNodeType node_type_ = IRNodeType::Allocate;
};



}  // namespace Internal

}  // namespace Boost