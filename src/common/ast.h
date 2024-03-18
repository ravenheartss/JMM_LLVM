#ifndef JMM_AST_H
#define JMM_AST_H

#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

// Easiest way is to just have one class Node and have enum types

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

// Base class for all nodes. Not used though.
class ASTNode {
 protected:
  ASTNode() = default;

 public:
  ~ASTNode() noexcept { children.clear(); }

 public:
  // most of these only apply to expression nodes but doesn't hurt to have them
  // if they're not used, they'll be monotone or nullopt
  std::variant<std::monostate, std::string, int32_t, bool> value;
  std::optional<VType> val_type{std::nullopt};  // Types
  std::optional<Op> op{std::nullopt};           // For operators
  uint32_t line{0};

  std::vector<std::unique_ptr<ASTNode>> children;
  virtual void print(int indent) = 0;
};

class ProgNode final : public ASTNode {
 public:
  void print(int indent) override final;
};

class StmtNode : public ASTNode {
 public:
  virtual void print(int indent) = 0;
};

class ExprNode : public ASTNode {
 public:
  virtual void print(int indent) = 0;
};

class DeclNode : public ASTNode {
 public:
  virtual void print(int indent) = 0;
};

// Stmts
class IfStmt final : public StmtNode {
 public:
  void print(int indent) override final;
};

class IfElseStmt final : public StmtNode {
 public:
  void print(int indent) override final;
};

class WhileStmt final : public StmtNode {
 public:
  void print(int indent) override final;
};

class GotoStmt final : public StmtNode {
 public:
  void print(int indent) override final;
};

class ReturnStmt final : public StmtNode {
 public:
  void print(int indent) override final;
};

class BreakStmt final : public StmtNode {
 public:
  void print(int indent) override final;
};

class BlockStmt final : public StmtNode {
 public:
  void print(int indent) override final;
};

class ExprStmt final : public StmtNode {
 public:
  void print(int indent) override final;
};

class NullStmt final : public StmtNode {
 public:
  void print(int indent) override final;
};

// Exprs
class IdExpr final : public ExprNode {
 public:
  void print(int indent) override final;
};

class LitExpr final : public ExprNode {
 public:
  void print(int indent) override final;
};

class UnaryExpr final : public ExprNode {
 public:
  void print(int indent) override final;
};

class BinaryExpr final : public ExprNode {
 public:
  void print(int indent) override final;
};

class BitwiseExpr final : public ExprNode {
 public:
  void print(int indent) override final;
};

class AssignExpr final : public ExprNode {
 public:
  void print(int indent) override final;
};

class FuncCallExpr final : public ExprNode {
 public:
  void print(int indent) override final;
};

class ActualsExpr final : public ExprNode {
 public:
  void print(int indent) override final;
};

class ActualExpr final : public ExprNode {
 public:
  void print(int indent) override final;
};

// Decls
class FuncDecl final : public DeclNode {
 public:
  void print(int indent) override final;
};

class MFuncDecl final : public DeclNode {
 public:
  void print(int indent) override final;
};

class VarDecl final : public DeclNode {
 public:
  void print(int indent) override final;
};

class GVarDecl final : public DeclNode {
 public:
  void print(int indent) override final;
};

class ParamsDecl final : public DeclNode {
 public:
  void print(int indent) override final;
};

class ParamDecl final : public DeclNode {
 public:
  void print(int indent) override final;
};

template <class PreOrderFunc, class PostOrderFunc>
void traverse(std::unique_ptr<ASTNode>& node, PreOrderFunc preProc,
              PostOrderFunc postProc) {
  if (!node) {
    return;
  }

  preProc(node);
  for (auto& child : node->children) {
    traverse(child, preProc, postProc);
  }

  postProc(node);
}

#endif  // !JMM_AST_H
