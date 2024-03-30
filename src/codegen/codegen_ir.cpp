#include "codegen.h"
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Support/raw_ostream.h>
#include "common/globals.h"
#include "common/symtab.h"

llvm::Value* IRCodegenVisitor::codegen(ASTNode const* node) {
  for (auto const& child : node->children) {
    child->codegen(this);
  }
}

llvm::Value* IRCodegenVisitor::codegen(IfStmt const* node) {
  llvm::Function* func = m_builder->GetInsertBlock()->getParent();

  // Generate code for the conditional expression.
  auto* cond = node->condition->codegen(this);

  // Create basic blocks for "then" branch, "else" branch (if any), and the join
  // point.
  auto* then = llvm::BasicBlock::Create(*m_context, "if.then", func);
  auto* end = llvm::BasicBlock::Create(*m_context, "if.end", func);

  m_builder->CreateCondBr(cond, then, end);

  m_builder->SetInsertPoint(then);
  node->if_body->codegen(this);

  // No need to create an unconditional branch here to end as we know there's no
  // else!

  // Set the builder insertion point in the join block.
  m_builder->SetInsertPoint(end);
  return nullptr;
}

llvm::Value* IRCodegenVisitor::codegen(IfElseStmt const* node) {
  llvm::Function* func = m_builder->GetInsertBlock()->getParent();

  auto* cond = node->condition->codegen(this);
  auto* then = llvm::BasicBlock::Create(*m_context, "if.then", func);
  auto* else_block = llvm::BasicBlock::Create(*m_context, "if.else", func);
  auto* end = llvm::BasicBlock::Create(*m_context, "if.end", func);

  m_builder->CreateCondBr(cond, then, else_block);

  m_builder->SetInsertPoint(then);
  node->if_body->codegen(this);

  m_builder->CreateBr(end);

  m_builder->SetInsertPoint(else_block);
  node->else_body->codegen(this);

  // Set the builder insertion point in the join block.
  m_builder->SetInsertPoint(end);
  return nullptr;
}

/*
 * While template:
 * br label %while.cond
 * while.cond: .....
 * while.body: .....
 * while.end: stuff after while loop
 */
llvm::Value* IRCodegenVisitor::codegen(WhileStmt const* node) {
  auto* func = m_builder->GetInsertBlock()->getParent();
  llvm::BasicBlock* cond =
      llvm::BasicBlock::Create(*m_context, "while.cond", func);
  m_builder->SetInsertPoint(cond);

  auto* cond_val = node->condition->codegen(this);  // This works! Good job!

  llvm::BasicBlock* body = llvm::BasicBlock::Create(*m_context, "while.body");
  llvm::BasicBlock* end = llvm::BasicBlock::Create(*m_context, "while.end");

  m_builder->CreateCondBr(cond_val, body, end);  // This works as well!

  m_while_context.push(end);  // Push the current end block to while_context

  body->insertInto(func);
  m_builder->SetInsertPoint(body);
  node->body->accept(this);
  body = m_builder->GetInsertBlock();

  m_builder->CreateBr(cond);

  end->insertInto(func);
  m_builder->SetInsertPoint(end);
  m_while_context.pop();

  return nullptr;
}

llvm::Value* IRCodegenVisitor::codegen(ReturnStmt const* node) {
  if (node->a_type.value() == VType::Void) {
    return m_builder->CreateRetVoid();
  }
  auto* ret = node->expr->codegen(this);
  return m_builder->CreateRet(node->expr->codegen(this));
}

llvm::Value* IRCodegenVisitor::codegen(BreakStmt const* node) {
  // needs to jump to current while's end
  m_builder->CreateBr(m_while_context.top());
  return nullptr;
}

llvm::Value* IRCodegenVisitor::codegen(BlockStmt const* node) {
  for (auto const& child : node->children) {
    child->codegen(this);
  }

  return nullptr;
}

llvm::Value* IRCodegenVisitor::codegen(ExprStmt const* node) {
  return node->expr->codegen(this);
}

llvm::Value* IRCodegenVisitor::codegen(IdExpr const* node) {
  auto* val = node->symbol->llvm_Value;
  if (!val) {
    m_logger->error("Failed to get symbol of ID at line ", node->line);
  }

  // Always return load ??
  return m_builder->CreateLoad(getType(node->a_type.value()), val);
  // val);
  // return val;
}

llvm::Value* IRCodegenVisitor::codegen(LitExpr const* node) {
  switch (node->type) {
    case VType::Int:
      return llvm::ConstantInt::getSigned(Int32(), node->ival.value());
    case VType::Str:
      return m_builder->CreateGlobalString(llvm::StringRef(node->sval.value()),
                                           "str", 0, m_module.get());
    case VType::Bool:
      return llvm::ConstantInt::getBool(Boolean(), node->bval.value());
    case VType::Void:
      m_logger->error("void constant!! Not possible");
      break;
  }
}

llvm::Value* IRCodegenVisitor::codegen(UnaryExpr const* node) {
  // PostFix - Return value before inc/dec
  // Prefix -- Return value after inc/dec
  auto* val = node->expr->codegen(this);

  llvm::LoadInst* load;
  llvm::Value* res;

  switch (node->op) {
    case Op::POSTINC:
      // Only ints can be inc/dec
      load = m_builder->CreateLoad(Int32(), val);
      res = m_builder->CreateNSWAdd(
          load, llvm::ConstantInt::getSigned(Int32(), 1), "inc");
      m_builder->CreateStore(res, val);
      return load;
    case Op::POSTDEC:
      load = m_builder->CreateLoad(Int32(), val);
      res = m_builder->CreateNSWSub(
          load, llvm::ConstantInt::getSigned(Int32(), 1), "dec");
      m_builder->CreateStore(res, val);
      return load;
    case Op::PREINC:
      load = m_builder->CreateLoad(Int32(), val);
      res = m_builder->CreateNSWAdd(
          load, llvm::ConstantInt::getSigned(Int32(), 1), "inc");
      m_builder->CreateStore(res, val);
      return res;
    case Op::PREDEC:
      load = m_builder->CreateLoad(Int32(), val);
      res = m_builder->CreateNSWSub(
          load, llvm::ConstantInt::getSigned(Int32(), 1), "dec");
      m_builder->CreateStore(res, val);
      return res;
    case Op::NOT:
      return m_builder->CreateNot(val);
    case Op::SUB:
      return m_builder->CreateNeg(val);
    default:
      break;
  }
}

llvm::Value* IRCodegenVisitor::codegen(BinaryExpr const* node) {
  // Binary Expressions are all expressions but bitwise
  auto* lhs = node->lhs->codegen(this);
  std::cerr << node->lhs->a_type.value() << "<- Type of lhs";
  if (node->lhs->symbol->symbol_type == SymType::VAR) std::cerr << " --- VAR\n";
  if (lhs->getType()->isIntegerTy()) std::cerr << "INDEED\n";

  auto* rhs = node->rhs->codegen(this);

  std::cerr << node->rhs->a_type.value() << "<- Type of rhs\n";
  if (rhs->getType()->isIntegerTy()) std::cerr << "INDEED\n";

  // TODO(shankar): Make short-circuiting
  switch (node->op) {
    case Op::LAND:
      return m_builder->CreateLogicalAnd(lhs, rhs, "and");
    case Op::LOR:
      return m_builder->CreateLogicalOr(lhs, rhs, "or");
    case Op::ADD:
      return m_builder->CreateNSWAdd(lhs, rhs, "add");
    case Op::SUB:
      return m_builder->CreateNSWSub(lhs, rhs, "sub");
    case Op::MULT:
      return m_builder->CreateNSWMul(lhs, rhs, "mult");
    case Op::DIV:
      return m_builder->CreateSDiv(lhs, rhs, "div");
    case Op::MOD:
      return m_builder->CreateSRem(lhs, rhs, "mod");
    case Op::EQ:
      return m_builder->CreateICmpEQ(lhs, rhs, "eq");
    case Op::NE:
      return m_builder->CreateICmpNE(lhs, rhs, "neq");
    case Op::GT:
      return m_builder->CreateICmpSGT(lhs, rhs, "gt");
    case Op::LT:
      return m_builder->CreateICmpSLT(lhs, rhs, "lt");
    case Op::GE:
      return m_builder->CreateICmpSGE(lhs, rhs, "ge");
    case Op::LE:
      return m_builder->CreateICmpSLE(lhs, rhs, "le");
    default:
      break;
  }

  return nullptr;
}

llvm::Value* IRCodegenVisitor::codegen(BitwiseExpr const* node) {
  auto* lhs = node->lhs->codegen(this);
  auto* rhs = node->rhs->codegen(this);

  switch (node->op) {
    case Op::BAND:
      return m_builder->CreateAnd(lhs, rhs, "and");
    case Op::BOR:
      return m_builder->CreateOr(lhs, rhs, "xor");
    case Op::XOR:
      return m_builder->CreateXor(lhs, rhs, "xor");
    case Op::LSHIFT:
      return m_builder->CreateShl(lhs, rhs, "shl");  // shift left
    case Op::RSHIFT:
      return m_builder->CreateAShr(lhs, rhs, "asr");  // arithemtic shift right
    default:
      break;
  }

  return nullptr;
}

llvm::Value* IRCodegenVisitor::codegen(AssignExpr const* node) {
  // No need to call codegen for lhs since we know it is a valid ID
  // thanks to the semantic analyzer. Another downside to calling codegen on ID
  // is that it returns a load instruction which we don't want here. So just get
  // the value from the symbol table
  llvm::Value* lhs = node->lhs->symbol->llvm_Value;
  llvm::Value* rhs = node->rhs->codegen(this);

  m_builder->CreateStore(rhs, lhs);
  return rhs;  // carry this value up the tree
}

llvm::Value* IRCodegenVisitor::codegen(FuncCallExpr const* node) {
  auto* func = m_module->getFunction(llvm::StringRef(node->id));

  std::vector<llvm::Value*> argv(node->args->actuals.size());
  for (auto const& actual : node->args->actuals) {
    argv.push_back(actual->codegen(this));
  }

  return m_builder->CreateCall(func, argv, "call");
}

llvm::Value* IRCodegenVisitor::codegen(FuncDecl const* node) { return nullptr; }

llvm::Value* IRCodegenVisitor::codegen(MFuncDecl const* node) {
  return nullptr;
}

llvm::Value* IRCodegenVisitor::codegen(VarDecl const* node) { return nullptr; }

llvm::Value* IRCodegenVisitor::codegen(GVarDecl const* node) { return nullptr; }

llvm::Value* IRCodegenVisitor::codegen(ParamDecl const* node) {
  return nullptr;
}

llvm::Value* IRCodegenVisitor::codegen(Params const* node) { return nullptr; }

llvm::Value* IRCodegenVisitor::codegen(Actuals const* node) { return nullptr; }

llvm::Value* IRCodegenVisitor::codegen(ActualExpr const* node) {
  return nullptr;
}

llvm::Value* IRCodegenVisitor::codegen(NullStmt const* node) { return nullptr; }
