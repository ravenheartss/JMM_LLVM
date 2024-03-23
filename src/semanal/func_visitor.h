#ifndef JMM_FUNCVISITOR_H
#define JMM_FUNCVISITOR_H

#include "common/ast.h"
#include "common/symtab.h"
#include "semanal/analyzer.h"

class FunctionVisitor : public Visitor {
 public:
  void visit(ASTNode* node) override;
  void visit(IfStmt* node) override;
  void visit(IfElseStmt* node) override;
  void visit(WhileStmt* node) override;
  void visit(GotoStmt* node) override;
  void visit(ReturnStmt* node) override;
  void visit(BreakStmt* node) override;
  void visit(BlockStmt* node) override;
  void visit(ExprStmt* node) override;
  // void visit(NullStmt* node) override;
  void visit(IdExpr* node) override;
  // void visit(LitExpr* node) override;
  void visit(UnaryExpr* node) override;
  void visit(BinaryExpr* node) override;
  void visit(BitwiseExpr* node) override;
  void visit(AssignExpr* node) override;
  void visit(FuncCallExpr* node) override;
  void visit(FuncDecl* node) override;
  void visit(MFuncDecl* node) override;
  void visit(VarDecl* node) override;
  void visit(ParamDecl* node) override;
  void visit(Params* node) override;
  void visit(ActualExpr* node) override;
  void visit(Actuals* node) override;

 private:
  explicit FunctionVisitor(std::shared_ptr<SymbolTable>& symtab,
                           std::shared_ptr<Logger>& logger)
      : m_symtab(symtab), m_logger(logger) {}

  std::shared_ptr<SymbolTable> m_symtab;
  std::shared_ptr<Logger> m_logger;
  bool m_inside_while{false};
  bool m_visited_main{false};
  uint32_t m_cur_block_scope{0};
  friend class SemanticAnalyzer;
};

#endif  // !JMM_FUNCVISITOR_H
