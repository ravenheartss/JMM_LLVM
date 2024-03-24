#ifndef JMM_AST_PRINTER_H
#define JMM_AST_PRINTER_H

#include "ast.h"

class ASTPrinter : public Visitor {
 public:
  void visit(ASTNode* node) override;
  void visit(IfStmt* node) override;
  void visit(IfElseStmt* node) override;
  void visit(WhileStmt* node) override;
  void visit(ReturnStmt* node) override;
  void visit(BreakStmt* node) override;
  void visit(BlockStmt* node) override;
  void visit(ExprStmt* node) override;
  void visit(NullStmt* node) override;
  void visit(IdExpr* node) override;
  void visit(LitExpr* node) override;
  void visit(UnaryExpr* node) override;
  void visit(BinaryExpr* node) override;
  void visit(BitwiseExpr* node) override;
  void visit(AssignExpr* node) override;
  void visit(FuncCallExpr* node) override;
  void visit(FuncDecl* node) override;
  void visit(MFuncDecl* node) override;
  void visit(VarDecl* node) override;
  void visit(GVarDecl* node) override;
  void visit(ParamDecl* node) override;
  void visit(Params* node) override;
  void visit(ActualExpr* node) override;
  void visit(Actuals* node) override;

 private:
  int m_indent{0};
};

#endif  // !JMM_AST_PRINTER_H
