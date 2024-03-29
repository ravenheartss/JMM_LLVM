#ifndef JMM_CODEGEN_H
#define JMM_CODEGEN_H

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <memory>
#include "common/ast.h"

// TODO(shankar): Fix this design. Just have the IRCodeVisitor. The visit functions are not really needed.
// The codegens can just return nullptr

class IRCodegenVisitor : public Visitor, public IRCodeVisitor {
 public:
  explicit IRCodegenVisitor(std::string const& filename);
  void visit(ASTNode* node) override;
  void visit(IfStmt* node) override;
  void visit(IfElseStmt* node) override;
  void visit(WhileStmt* node) override;
  void visit(ReturnStmt* node) override;
  void visit(BreakStmt* node) override;
  void visit(BlockStmt* node) override;
  void visit(ExprStmt* node) override;
  // void visit(NullStmt* node) override;
  // void visit(IdExpr* node) override;
  // void visit(LitExpr* node) override;
  // void visit(UnaryExpr* node) override;
  // void visit(BinaryExpr* node) override;
  // void visit(BitwiseExpr* node) override;
  // void visit(AssignExpr* node) override;
  // void visit(FuncCallExpr* node) override;
  void visit(FuncDecl* node) override;
  void visit(MFuncDecl* node) override;
  void visit(VarDecl* node) override;
  // void visit(GVarDecl* node) override;
  // void visit(ParamDecl* node) override;
  // void visit(Params* node) override;
  // void visit(ActualExpr* node) override;
  // void visit(Actuals* node) override;

  // IRCode builder
  llvm::Value* codegen(ASTNode const* node) override;
  llvm::Value* codegen(IfStmt const* node) override;
  llvm::Value* codegen(IfElseStmt const* node) override;
  llvm::Value* codegen(WhileStmt const* node) override;
  llvm::Value* codegen(ReturnStmt const* node) override;
  llvm::Value* codegen(BreakStmt const* node) override;
  llvm::Value* codegen(BlockStmt const* node) override;
  llvm::Value* codegen(ExprStmt const* node) override;
  llvm::Value* codegen(NullStmt const* node) override;
  llvm::Value* codegen(IdExpr const* node) override;
  llvm::Value* codegen(LitExpr const* node) override;
  llvm::Value* codegen(UnaryExpr const* node) override;
  llvm::Value* codegen(BinaryExpr const* node) override;
  llvm::Value* codegen(BitwiseExpr const* node) override;
  llvm::Value* codegen(AssignExpr const* node) override;
  llvm::Value* codegen(FuncCallExpr const* node) override;
  llvm::Value* codegen(FuncDecl const* node) override;
  llvm::Value* codegen(MFuncDecl const* node) override;
  llvm::Value* codegen(VarDecl const* node) override;
  llvm::Value* codegen(GVarDecl const* node) override;
  llvm::Value* codegen(ParamDecl const* node) override;
  llvm::Value* codegen(Params const* node) override;
  llvm::Value* codegen(Actuals const* node) override;
  llvm::Value* codegen(ActualExpr const* node) override;


  void output() { m_module->print(llvm::errs(), nullptr); }

 private:
  friend class CodegenGlobals;

  class CodegenGlobals : public Visitor {
   public:
    explicit CodegenGlobals(IRCodegenVisitor const* gen);

    void visit(ASTNode* node) override;
    void visit(FuncDecl* node) override;
    void visit(MFuncDecl* node) override;
    void visit(GVarDecl* node) override;

   private:
    IRCodegenVisitor const* m_gen;
  };

  void buildRTS();
  [[nodiscard]] llvm::Type* getType(VType type) const;

  [[nodiscard]] llvm::Type* Int32() const {
    return llvm::Type::getInt32Ty(*m_context);
  };

  [[nodiscard]] llvm::Type* Void() const {
    return llvm::Type::getVoidTy(*m_context);
  };

  [[nodiscard]] llvm::Type* Str() const {
    return llvm::Type::getInt32PtrTy(*m_context);
  };

  [[nodiscard]] llvm::Type* Boolean() const {
    return llvm::Type::getInt1Ty(*m_context);  // bools are 1-bit
  };

  std::unique_ptr<llvm::LLVMContext> m_context;
  std::unique_ptr<llvm::Module> m_module;
  std::unique_ptr<llvm::IRBuilder<>> m_builder;
  std::shared_ptr<Logger> m_logger;
  llvm::Function * m_curr_func;
  std::stack<llvm::BasicBlock*> m_while_context;
  std::string m_mainFuncID;
};

#endif
