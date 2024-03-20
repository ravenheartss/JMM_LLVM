#ifndef JMM_AST_H
#define JMM_AST_H

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#define ACCEPT_NODE_OVERRIDE \
  void accept(Visitor* visitor) override { visitor->visit(this); }

enum class VType : uint8_t { Int, Str, Bool, Void };

enum class Op : uint8_t {
  BAND,
  BOR,
  LAND,
  LOR,
  XOR,
  ADD,
  POSTINC,
  POSTDEC,
  PREINC,
  PREDEC,
  SUB,
  MULT,
  DIV,
  MOD,
  NOT,
  EQ,
  NE,
  GT,
  LT,
  GE,
  LE,
  LSHIFT,
  RSHIFT
};

// Need to be defined for Visitor
class ASTNode;
class IfStmt;
class IfElseStmt;
class WhileStmt;
class GotoStmt;
class ReturnStmt;
class BreakStmt;
class BlockStmt;
class ExprStmt;
class NullStmt;
class IdExpr;
class LitExpr;
class UnaryExpr;
class BinaryExpr;
class BitwiseExpr;
class AssignExpr;
class FuncCallExpr;
class FuncDecl;
class MFuncDecl;
class VarDecl;
class GVarDecl;
class ParamDecl;
class Params;  // holds paramdecls
class ActualExpr;
class Actuals;  // holds actualexprs

using nodePtr = std::unique_ptr<ASTNode>;

std::ostream& operator<<(std::ostream& out, VType value_type);
std::ostream& operator<<(std::ostream& out, Op oper);

class Visitor {
 public:
  virtual void visit(ASTNode const* node) {}

  virtual void visit(IfStmt const* node) {}

  virtual void visit(IfElseStmt const* node) {}

  virtual void visit(WhileStmt const* node) {}

  virtual void visit(GotoStmt const* node) {}

  virtual void visit(ReturnStmt const* node) {}

  virtual void visit(BreakStmt const* node) {}

  virtual void visit(BlockStmt const* node) {}

  virtual void visit(ExprStmt const* node) {}

  virtual void visit(NullStmt const* node) {}

  virtual void visit(IdExpr const* node) {}

  virtual void visit(LitExpr const* node) {}

  virtual void visit(UnaryExpr const* node) {}

  virtual void visit(BinaryExpr const* node) {}

  virtual void visit(BitwiseExpr const* node) {}

  virtual void visit(AssignExpr const* node) {}

  virtual void visit(FuncCallExpr const* node) {}

  virtual void visit(FuncDecl const* node) {}

  virtual void visit(MFuncDecl const* node) {}

  virtual void visit(VarDecl const* node) {}

  virtual void visit(GVarDecl const* node) {}

  virtual void visit(ParamDecl const* node) {}

  virtual void visit(Params const* node) {}

  virtual void visit(Actuals const* node) {}

  virtual void visit(ActualExpr const* node) {}
};

class ASTNode {
 public:
  ASTNode() = default;
  virtual ~ASTNode();

  // Don't allow it to be copied
  ASTNode& operator=(ASTNode const& node) = delete;
  ASTNode(ASTNode const& node) = delete;

  ASTNode& operator=(ASTNode&& node) noexcept;
  ASTNode(ASTNode const&& node) noexcept;

  uint32_t line;
  std::vector<std::unique_ptr<ASTNode>> children;

  virtual void accept(Visitor* visitor) { visitor->visit(this); }
};

// I can have this, but I find it is useless to have one since they don't really
// do anything and as such I can just have the actual nodes inherit from astNode
// and ASTNode now becomes the root node. class ProgNode : public ASTNode
// {
// };
//
// class StmtNode : public ASTNode
// {
// };
//
// class ExprNode : public ASTNode
// {
// };
// class DeclNode
// {
//     public:
//         std::string value;
// };

class IfStmt : public ASTNode {
 public:
  IfStmt(nodePtr condition, nodePtr if_body)
      : condition(std::move(condition)), if_body(std::move(if_body)) {}

  ACCEPT_NODE_OVERRIDE

  nodePtr condition;
  nodePtr if_body;
};

class IfElseStmt : public ASTNode {
 public:
  IfElseStmt(nodePtr condition, nodePtr if_body, nodePtr else_body)
      : condition(std::move(condition)),
        if_body(std::move(if_body)),
        else_body(std::move(else_body)) {}

  ACCEPT_NODE_OVERRIDE

  nodePtr condition;
  nodePtr if_body;
  nodePtr else_body;
};

class WhileStmt : public ASTNode {
 public:
  WhileStmt(nodePtr condition, nodePtr body)
      : condition(std::move(condition)), body(std::move(body)) {}

  ACCEPT_NODE_OVERRIDE

  nodePtr condition;
  nodePtr body;
};

class GotoStmt : public ASTNode {
 public:
  explicit GotoStmt(nodePtr expr) : expr(std::move(expr)) {}

  ACCEPT_NODE_OVERRIDE

  nodePtr expr;
};

class ReturnStmt : public ASTNode {
 public:
  explicit ReturnStmt(nodePtr expr) : expr(std::move(expr)) {}

  ReturnStmt() = default;

  ACCEPT_NODE_OVERRIDE

  nodePtr expr;
};

class BreakStmt : public ASTNode {
 public:
  ACCEPT_NODE_OVERRIDE
};

class BlockStmt : public ASTNode {
 public:
  ACCEPT_NODE_OVERRIDE
};

class ExprStmt : public ASTNode {
 public:
  explicit ExprStmt(nodePtr expr) : expr(std::move(expr)) {}

  ACCEPT_NODE_OVERRIDE

  nodePtr expr;
};

class NullStmt : public ASTNode {
 public:
  ACCEPT_NODE_OVERRIDE
};

class IdExpr : public ASTNode {
 public:
  explicit IdExpr(std::string value) : value(std::move(value)) {}

  ACCEPT_NODE_OVERRIDE

  std::string value;
};

class LitExpr : public ASTNode {
 public:
  explicit LitExpr(int32_t val)
      : ival(val), bval(std::nullopt), sval(std::nullopt), type(VType::Int) {}

  explicit LitExpr(bool val)
      : ival(std::nullopt), bval(val), sval(std::nullopt), type(VType::Bool) {}

  explicit LitExpr(std::string val)
      : ival(std::nullopt), bval(std::nullopt), sval(val), type(VType::Str) {}

  ACCEPT_NODE_OVERRIDE

  // Yes, I can use variants, but they're a pain!!!
  std::optional<int32_t> ival;
  std::optional<bool> bval;
  std::optional<std::string> sval;
  VType type;
};

class UnaryExpr : public ASTNode {
 public:
  UnaryExpr(Op op, nodePtr expr) : op(op), expr(std::move(expr)) {}

  ACCEPT_NODE_OVERRIDE

  Op op;
  nodePtr expr;
};

class BinaryExpr : public ASTNode {
 public:
  BinaryExpr(Op op, nodePtr lhs, nodePtr rhs)
      : op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}

  ACCEPT_NODE_OVERRIDE

  Op op;
  nodePtr lhs;
  nodePtr rhs;
};

class BitwiseExpr : public ASTNode {
 public:
  BitwiseExpr(Op op, nodePtr lhs, nodePtr rhs)
      : op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}

  ACCEPT_NODE_OVERRIDE

  Op op;
  nodePtr lhs;
  nodePtr rhs;
};

class AssignExpr : public ASTNode {
 public:
  AssignExpr(nodePtr lhs, nodePtr rhs)
      : lhs(std::move(lhs)), rhs(std::move(rhs)) {}

  ACCEPT_NODE_OVERRIDE

  nodePtr lhs;
  nodePtr rhs;
};

class ActualExpr : public ASTNode {
 public:
  explicit ActualExpr(nodePtr expr) : expr(std::move(expr)) {}

  ACCEPT_NODE_OVERRIDE

  nodePtr expr;
};

class Actuals : public ASTNode {
 public:
  explicit Actuals(std::vector<std::unique_ptr<ActualExpr>> actuals)
      : actuals(std::move(actuals)) {}

  ACCEPT_NODE_OVERRIDE

  std::vector<std::unique_ptr<ActualExpr>> actuals;
};

class FuncCallExpr : public ASTNode {
 public:
  FuncCallExpr(std::string id, std::unique_ptr<Actuals> args)
      : id(std::move(id)), args(std::move(args)) {}

  ACCEPT_NODE_OVERRIDE

  std::string id;
  std::unique_ptr<Actuals> args;
};

class FuncDecl : public ASTNode {
 public:
  FuncDecl(std::string id, VType return_type, std::unique_ptr<Params> params,
           nodePtr body)
      : id(std::move(id)),
        return_type(return_type),
        params(std::move(params)),
        body(std::move(body)) {}

  ACCEPT_NODE_OVERRIDE

  std::string id;
  VType return_type;
  std::unique_ptr<Params> params;
  nodePtr body;
};

class MFuncDecl : public ASTNode {
 public:
  MFuncDecl(std::string id, nodePtr body)
      : id(std::move(id)), body(std::move(body)) {}

  ACCEPT_NODE_OVERRIDE

  std::string id;
  nodePtr body;
};

class VarDecl : public ASTNode {
 public:
  VarDecl(std::string id, VType type) : id(std::move(id)), type(type) {}

  ACCEPT_NODE_OVERRIDE

  std::string id;
  VType type;
};

// Only for visitor
class GVarDecl : public VarDecl {
 public:
  GVarDecl(std::string id, VType type) : VarDecl(std::move(id), type) {}

  ACCEPT_NODE_OVERRIDE
};

// Again only for visitor
class ParamDecl : public VarDecl {
 public:
  ParamDecl(std::string id, VType type) : VarDecl(std::move(id), type) {}

  ACCEPT_NODE_OVERRIDE
};

class Params : public ASTNode {
 public:
  explicit Params(std::vector<std::unique_ptr<ParamDecl>> params)
      : params(std::move(params)) {}

  ACCEPT_NODE_OVERRIDE
  std::vector<std::unique_ptr<ParamDecl>> params;
};

#endif  //! JMM_AST_H
