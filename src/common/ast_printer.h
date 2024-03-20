#ifndef JMM_AST_PRINTER_H
#define JMM_AST_PRINTER_H

#include "ast.h"

class ASTPrinter : public Visitor {
 public:
  void visit(ASTNode const* node) override;
  void visit(IfStmt const* node) override;
  void visit(IfElseStmt const* node) override;
  void visit(WhileStmt const* node) override;
  void visit(GotoStmt const* node) override;
  void visit(ReturnStmt const* node) override;
  void visit(BreakStmt const* node) override;
  void visit(BlockStmt const* node) override;
  void visit(ExprStmt const* node) override;
  void visit(NullStmt const* node) override;
  void visit(IdExpr const* node) override;
  void visit(LitExpr const* node) override;
  void visit(UnaryExpr const* node) override;
  void visit(BinaryExpr const* node) override;
  void visit(BitwiseExpr const* node) override;
  void visit(AssignExpr const* node) override;
  void visit(FuncCallExpr const* node) override;
  void visit(FuncDecl const* node) override;
  void visit(MFuncDecl const* node) override;
  void visit(VarDecl const* node) override;
  void visit(GVarDecl const* node) override;
  void visit(ParamDecl const* node) override;
  void visit(Params const* node) override;
  void visit(ActualExpr const* node) override;
  void visit(Actuals const* node) override;

 private:
  int m_indent{0};
};


#endif // !JMM_AST_PRINTER_H
