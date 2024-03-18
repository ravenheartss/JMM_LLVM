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

// Forward Decls is needed since visitor relies on these classes and the ASTNode
// accept relies on visitor
class ASTNode;
class ProgNode;
class StmtNode;
class ExprNode;
class DeclNode;
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
class ActualsExpr;
class ActualExpr;
class FuncDecl;
class MFuncDecl;
class VarDecl;
class GVarDecl;
class ParamsDecl;
class ParamDecl;

class Visitor {
 public:
  virtual void visit(const ASTNode* node) {}

  virtual void visit(const ProgNode* node) {}

  virtual void visit(const StmtNode* node) {}

  virtual void visit(const ExprNode* node) {}

  virtual void visit(const DeclNode* node) {}

  virtual void visit(const IfStmt* node) {}

  virtual void visit(const IfElseStmt* node) {}

  virtual void visit(const WhileStmt* node) {}

  virtual void visit(const GotoStmt* node) {}

  virtual void visit(const ReturnStmt* node) {}

  virtual void visit(const BreakStmt* node) {}

  virtual void visit(const BlockStmt* node) {}

  virtual void visit(const ExprStmt* node) {}

  virtual void visit(const NullStmt* node) {}

  virtual void visit(const IdExpr* node) {}

  virtual void visit(const LitExpr* node) {}

  virtual void visit(const UnaryExpr* node) {}

  virtual void visit(const BinaryExpr* node) {}

  virtual void visit(const BitwiseExpr* node) {}

  virtual void visit(const AssignExpr* node) {}

  virtual void visit(const FuncCallExpr* node) {}

  virtual void visit(const ActualsExpr* node) {}

  virtual void visit(const ActualExpr* node) {}

  virtual void visit(const FuncDecl* node) {}

  virtual void visit(const MFuncDecl* node) {}

  virtual void visit(const VarDecl* node) {}

  virtual void visit(const GVarDecl* node) {}

  virtual void visit(const ParamsDecl* node) {}

  virtual void visit(const ParamDecl* node) {}
};


// inline const std::string& getStrValue(const std::variant<std::monostate, std::string, int32_t, bool>& val)
// {
//     return std::get<std::string>(val);
// }



// Base class for all nodes. Not used though.
class ASTNode {
 protected:
  ASTNode() = default;

 public:
  ~ASTNode() noexcept { children.clear(); }

  // most of these only apply to expression nodes but doesn't hurt to have them
  // if they're not used, they'll be monotone or nullopt
  std::variant<std::monostate, std::string, int32_t, bool> value;
  std::optional<VType> val_type{std::nullopt};  // Types
  std::optional<Op> op{std::nullopt};           // For operators
  uint32_t line{0};

  std::vector<std::unique_ptr<ASTNode>> children;
  virtual void print(int indent) = 0;
  virtual void accept(Visitor* visitor);
};

class ProgNode final : public ASTNode {
 public:
  void print(int indent) final;
};

class StmtNode : public ASTNode {
 public:
   void print(int indent) override = 0;
};

class ExprNode : public ASTNode {
 public:
   void print(int indent) override = 0;
};

class DeclNode : public ASTNode {
 public:
   void print(int indent) override = 0;
};

// Stmts
class IfStmt final : public StmtNode {
 public:
  void print(int indent)  final;
};

class IfElseStmt final : public StmtNode {
 public:
  void print(int indent)  final;
};

class WhileStmt final : public StmtNode {
 public:
  void print(int indent)  final;
};

class GotoStmt final : public StmtNode {
 public:
  void print(int indent)  final;
};

class ReturnStmt final : public StmtNode {
 public:
  void print(int indent)  final;
};

class BreakStmt final : public StmtNode {
 public:
  void print(int indent)  final;
};

class BlockStmt final : public StmtNode {
 public:
  void print(int indent) final;
};

class ExprStmt final : public StmtNode {
 public:
  void print(int indent) final;
};

class NullStmt final : public StmtNode {
 public:
  void print(int indent) final;
};

// Exprs
class IdExpr final : public ExprNode {
 public:
  void print(int indent) final;
};

class LitExpr final : public ExprNode {
 public:
  void print(int indent) final;
};

class UnaryExpr final : public ExprNode {
 public:
  void print(int indent) final;
};

class BinaryExpr final : public ExprNode {
 public:
  void print(int indent) final;
};

class BitwiseExpr final : public ExprNode {
 public:
  void print(int indent) final;
};

class AssignExpr final : public ExprNode {
 public:
  void print(int indent) final;
};

class FuncCallExpr final : public ExprNode {
 public:
  void print(int indent) final;
};

class ActualsExpr final : public ExprNode {
 public:
  void print(int indent) final;
};

class ActualExpr final : public ExprNode {
 public:
  void print(int indent) final;
};

// Decls
class FuncDecl final : public DeclNode {
 public:
  void print(int indent) final;
};

class MFuncDecl final : public DeclNode {
 public:
  void print(int indent) final;
};

class VarDecl final : public DeclNode {
 public:
  void print(int indent) final;
};

class GVarDecl final : public DeclNode {
 public:
  void print(int indent) final;
};

class ParamsDecl final : public DeclNode {
 public:
  void print(int indent) final;
};

class ParamDecl final : public DeclNode {
 public:
  void print(int indent) final;
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
