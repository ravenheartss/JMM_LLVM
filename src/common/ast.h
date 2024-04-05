#ifndef JMM_AST_H
#define JMM_AST_H

#include "globals.h"
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>
#include "common/symtab.h"

#define ACCEPT_NODE_OVERRIDE \
  void accept(Visitor* visitor) override { visitor->visit(this); }

#define CODEGEN_OVERRIDE                                  \
  llvm::Value* codegen(IRCodeVisitor* visitor) override { \
    return visitor->codegen(this);                        \
  }

// Need to be defined for Visitor
class ASTNode;
class IfStmt;
class IfElseStmt;
class WhileStmt;
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

class Visitor {
 public:
  // Two sets of visit. One const and one not const
  virtual void visit([[maybe_unused]] ASTNode* node) {}

  virtual void visit([[maybe_unused]] IfStmt* node) {}

  virtual void visit([[maybe_unused]] IfElseStmt* node) {}

  virtual void visit([[maybe_unused]] WhileStmt* node) {}

  virtual void visit([[maybe_unused]] ReturnStmt* node) {}

  virtual void visit([[maybe_unused]] BreakStmt* node) {}

  virtual void visit([[maybe_unused]] BlockStmt* node) {}

  virtual void visit([[maybe_unused]] ExprStmt* node) {}

  virtual void visit([[maybe_unused]] NullStmt* node) {}

  virtual void visit([[maybe_unused]] IdExpr* node) {}

  virtual void visit([[maybe_unused]] LitExpr* node) {}

  virtual void visit([[maybe_unused]] UnaryExpr* node) {}

  virtual void visit([[maybe_unused]] BinaryExpr* node) {}

  virtual void visit([[maybe_unused]] BitwiseExpr* node) {}

  virtual void visit([[maybe_unused]] AssignExpr* node) {}

  virtual void visit([[maybe_unused]] FuncCallExpr* node) {}

  virtual void visit([[maybe_unused]] FuncDecl* node) {}

  virtual void visit([[maybe_unused]] MFuncDecl* node) {}

  virtual void visit([[maybe_unused]] VarDecl* node) {}

  virtual void visit([[maybe_unused]] GVarDecl* node) {}

  virtual void visit([[maybe_unused]] ParamDecl* node) {}

  virtual void visit([[maybe_unused]] Params* node) {}

  virtual void visit([[maybe_unused]] Actuals* node) {}

  virtual void visit([[maybe_unused]] ActualExpr* node) {}
};

class IRCodeVisitor {
 public:
  virtual llvm::Value* codegen(ASTNode const* node) = 0;

  virtual llvm::Value* codegen(IfStmt const* node) = 0;

  virtual llvm::Value* codegen(IfElseStmt const* node) = 0;

  virtual llvm::Value* codegen(WhileStmt const* node) = 0;

  virtual llvm::Value* codegen(ReturnStmt const* node) = 0;

  virtual llvm::Value* codegen(BreakStmt const* node) = 0;

  virtual llvm::Value* codegen(BlockStmt const* node) = 0;

  virtual llvm::Value* codegen(ExprStmt const* node) = 0;

  virtual llvm::Value* codegen(NullStmt const* node) = 0;

  virtual llvm::Value* codegen(IdExpr const* node) = 0;

  virtual llvm::Value* codegen(LitExpr const* node) = 0;

  virtual llvm::Value* codegen(UnaryExpr const* node) = 0;

  virtual llvm::Value* codegen(BinaryExpr const* node) = 0;

  virtual llvm::Value* codegen(BitwiseExpr const* node) = 0;

  virtual llvm::Value* codegen(AssignExpr const* node) = 0;

  virtual llvm::Value* codegen(FuncCallExpr const* node) = 0;

  virtual llvm::Value* codegen(FuncDecl const* node) = 0;

  virtual llvm::Value* codegen(MFuncDecl const* node) = 0;

  virtual llvm::Value* codegen(VarDecl const* node) = 0;

  virtual llvm::Value* codegen(GVarDecl const* node) = 0;

  virtual llvm::Value* codegen(ParamDecl const* node) = 0;

  virtual llvm::Value* codegen(Params const* node) = 0;

  virtual llvm::Value* codegen(Actuals const* node) = 0;

  virtual llvm::Value* codegen(ActualExpr const* node) = 0;
};

class ASTNode {
 public:
  ASTNode() = default;
  virtual ~ASTNode();

  // Don't allow it to be copied
  ASTNode& operator=(ASTNode const& node) = delete;
  ASTNode(ASTNode const& node) = delete;

  ASTNode& operator=(ASTNode&& node) noexcept;
  ASTNode(ASTNode&& node) noexcept;

  uint32_t line;
  std::vector<std::unique_ptr<ASTNode>> children;

  virtual void accept(Visitor* visitor) { visitor->visit(this); }

  std::shared_ptr<Symbol> symbol;  // stores the symbol table information
  std::optional<VType> a_type;  // stores the type of the node - Annotated type

  virtual llvm::Value* codegen(IRCodeVisitor* visitor) {
    return visitor->codegen(this);
  }
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
  CODEGEN_OVERRIDE

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
  CODEGEN_OVERRIDE

  nodePtr condition;
  nodePtr if_body;
  nodePtr else_body;
};

class WhileStmt : public ASTNode {
 public:
  WhileStmt(nodePtr condition, nodePtr body)
      : condition(std::move(condition)), body(std::move(body)) {}

  ACCEPT_NODE_OVERRIDE
  CODEGEN_OVERRIDE

  nodePtr condition;
  nodePtr body;
};

class ReturnStmt : public ASTNode {
 public:
  explicit ReturnStmt(nodePtr expr) : expr(std::move(expr)) {}

  ReturnStmt() = default;

  ACCEPT_NODE_OVERRIDE
  CODEGEN_OVERRIDE

  nodePtr expr;
};

class BreakStmt : public ASTNode {
 public:
  ACCEPT_NODE_OVERRIDE
  CODEGEN_OVERRIDE
};

class BlockStmt : public ASTNode {
 public:
  ACCEPT_NODE_OVERRIDE
  CODEGEN_OVERRIDE
};

class ExprStmt : public ASTNode {
 public:
  explicit ExprStmt(nodePtr expr) : expr(std::move(expr)) {}

  ACCEPT_NODE_OVERRIDE
  CODEGEN_OVERRIDE

  nodePtr expr;
};

class NullStmt : public ASTNode {
 public:
  ACCEPT_NODE_OVERRIDE
  CODEGEN_OVERRIDE
};

class IdExpr : public ASTNode {
 public:
  explicit IdExpr(std::string value) : value(std::move(value)) {}

  ACCEPT_NODE_OVERRIDE
  CODEGEN_OVERRIDE

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
  CODEGEN_OVERRIDE

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
  CODEGEN_OVERRIDE

  Op op;
  nodePtr expr;
};

class BinaryExpr : public ASTNode {
 public:
  BinaryExpr(Op op, nodePtr lhs, nodePtr rhs)
      : op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}

  ACCEPT_NODE_OVERRIDE
  CODEGEN_OVERRIDE

  Op op;
  nodePtr lhs;
  nodePtr rhs;
};

class BitwiseExpr : public ASTNode {
 public:
  BitwiseExpr(Op op, nodePtr lhs, nodePtr rhs)
      : op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}

  ACCEPT_NODE_OVERRIDE
  CODEGEN_OVERRIDE

  Op op;
  nodePtr lhs;
  nodePtr rhs;
};

class AssignExpr : public ASTNode {
 public:
  AssignExpr(nodePtr lhs, nodePtr rhs)
      : lhs(std::move(lhs)), rhs(std::move(rhs)) {}

  ACCEPT_NODE_OVERRIDE
  CODEGEN_OVERRIDE

  nodePtr lhs;
  nodePtr rhs;
};

class ActualExpr : public ASTNode {
 public:
  explicit ActualExpr(nodePtr expr) : expr(std::move(expr)) {}

  ACCEPT_NODE_OVERRIDE
  CODEGEN_OVERRIDE

  nodePtr expr;
};

class Actuals : public ASTNode {
 public:
  explicit Actuals(std::vector<std::unique_ptr<ActualExpr>> actuals)
      : actuals(std::move(actuals)) {}

  ACCEPT_NODE_OVERRIDE
  CODEGEN_OVERRIDE

  std::vector<std::unique_ptr<ActualExpr>> actuals;
};

class FuncCallExpr : public ASTNode {
 public:
  FuncCallExpr(std::string id, std::unique_ptr<Actuals> args)
      : id(std::move(id)), args(std::move(args)) {}

  ACCEPT_NODE_OVERRIDE
  CODEGEN_OVERRIDE

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
  CODEGEN_OVERRIDE

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
  CODEGEN_OVERRIDE

  std::string id;
  nodePtr body;
};

class VarDecl : public ASTNode {
 public:
  VarDecl(std::string id, VType type) : id(std::move(id)), type(type) {}

  ACCEPT_NODE_OVERRIDE
  CODEGEN_OVERRIDE

  std::string id;
  VType type;
};

// Only for visitor
class GVarDecl : public VarDecl {
 public:
  GVarDecl(std::string id, VType type) : VarDecl(std::move(id), type) {}

  ACCEPT_NODE_OVERRIDE
  CODEGEN_OVERRIDE
};

// Again only for visitor
class ParamDecl : public VarDecl {
 public:
  ParamDecl(std::string id, VType type) : VarDecl(std::move(id), type) {}

  ACCEPT_NODE_OVERRIDE
  CODEGEN_OVERRIDE
};

class Params : public ASTNode {
 public:
  explicit Params(std::vector<std::unique_ptr<ParamDecl>> params)
      : params(std::move(params)) {}

  ACCEPT_NODE_OVERRIDE
  CODEGEN_OVERRIDE
  std::vector<std::unique_ptr<ParamDecl>> params;
};

#endif  //! JMM_AST_H
