#include "codegen.h"
#include <llvm/ADT/StringRef.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>
#include <memory>
#include "common/globals.h"

IRCodegenVisitor::IRCodegenVisitor(std::string const& filename) {
  m_context = std::make_unique<llvm::LLVMContext>();
  m_module = std::make_unique<llvm::Module>(filename, *m_context);
  m_builder = std::make_unique<llvm::IRBuilder<>>(*m_context);

  buildRTS();
}

// I'll exclusively be relying on syscalls for all of my RTS functions
// LLVM IR does not have any way to use syscalls or if it does, I haven't
// managed to find any after the surfing the internet for a few minutes.
// However, making syscalls would defeat the entire purpose of the IR!
void IRCodegenVisitor::buildRTS() {
  llvm::Function* libc_func;
  llvm::FunctionType* libc_ftype;
  llvm::Function* rts_func;
  llvm::FunctionType* rts_ftype;
  std::vector<llvm::Type*> arg_types;
  std::vector<llvm::Value*> args;

  // prints
  // Create format string first. Can call puts as well.
  auto* fmt = m_builder->CreateGlobalString(llvm::StringRef("%s"), "fmtstr", 0,
                                            m_module.get());
  arg_types.emplace_back(llvm::Type::getInt8PtrTy(*m_context));
  libc_ftype = llvm::FunctionType::get(llvm::Type::getInt32Ty(*m_context),
                                       arg_types, true);

  libc_func =
      llvm::Function::Create(libc_ftype, llvm::Function::ExternalLinkage,
                             llvm::Twine("printf"), *m_module);
  libc_func->setCallingConv(llvm::CallingConv::C);
  arg_types.clear();

  arg_types.emplace_back(Str());
  rts_ftype = llvm::FunctionType::get(Void(), arg_types, false);
  rts_func = llvm::Function::Create(rts_ftype, llvm::Function::PrivateLinkage,
                                    llvm::Twine("prints"), *m_module);
  for (auto& arg : rts_func->args()) {
    arg.setName("str");
  }
  llvm::BasicBlock* body =
      llvm::BasicBlock::Create(*m_context, "entry", rts_func);
  m_builder->SetInsertPoint(body);
  args.emplace_back(fmt);
  args.emplace_back(rts_func->getArg(0));

  m_builder->CreateCall(libc_func, args, "call");
  m_builder->CreateRetVoid();
  llvm::verifyFunction(*rts_func);

  // ----------------------------
  args.clear();
  arg_types.clear();

  // halt -- no param
  libc_ftype = llvm::FunctionType::get(Void(), arg_types, false);

  libc_func =
      llvm::Function::Create(libc_ftype, llvm::Function::ExternalLinkage,
                             llvm::Twine("exit"), *m_module);
  libc_func->setCallingConv(llvm::CallingConv::C);
  arg_types.clear();

  rts_ftype = llvm::FunctionType::get(Void(), false);
  rts_func = llvm::Function::Create(rts_ftype, llvm::Function::PrivateLinkage,
                                    llvm::Twine("halt"), *m_module);
  body = llvm::BasicBlock::Create(*m_context, "entry", rts_func);
  m_builder->SetInsertPoint(body);
  args.emplace_back(llvm::ConstantInt::getSigned(Int32(), 0));

  m_builder->CreateCall(libc_func, args, "call");
  m_builder->CreateRetVoid();
  llvm::verifyFunction(*rts_func);

  // ----------------------------
  args.clear();
  arg_types.clear();

  // printi
  fmt = m_builder->CreateGlobalString(llvm::StringRef("%d"), "fmtint", 0,
                                      m_module.get());
  libc_func = m_module->getFunction(llvm::StringRef("printf"));

  arg_types.emplace_back(Int32());
  rts_ftype = llvm::FunctionType::get(Void(), arg_types, false);
  rts_func = llvm::Function::Create(rts_ftype, llvm::Function::PrivateLinkage,
                                    llvm::Twine("printi"), *m_module);
  for (auto& arg : rts_func->args()) {
    arg.setName("int");
  }
  body = llvm::BasicBlock::Create(*m_context, "entry", rts_func);
  m_builder->SetInsertPoint(body);
  args.emplace_back(fmt);
  args.emplace_back(rts_func->getArg(0));

  m_builder->CreateCall(libc_func, args, "call");
  m_builder->CreateRetVoid();
  llvm::verifyFunction(*rts_func);

  // ------------------------------------------------------------------------

  // printb
  args.clear();
  arg_types.clear();

  auto* true_str = m_builder->CreateGlobalString(llvm::StringRef("true"),
                                                 "booltrue", 0, m_module.get());
  auto* false_str = m_builder->CreateGlobalString(
      llvm::StringRef("false"), "boolfalse", 0, m_module.get());
  libc_func = m_module->getFunction(llvm::StringRef("prints"));

  arg_types.emplace_back(Boolean());
  rts_ftype = llvm::FunctionType::get(Void(), arg_types, false);
  rts_func = llvm::Function::Create(rts_ftype, llvm::Function::PrivateLinkage,
                                    llvm::Twine("printb"), *m_module);
  for (auto& arg : rts_func->args()) {
    arg.setName("bool");
  }
  body = llvm::BasicBlock::Create(*m_context, "entry", rts_func);
  m_builder->SetInsertPoint(body);
  auto* true_print =
      llvm::BasicBlock::Create(*m_context, "true_print", rts_func);
  auto* false_print =
      llvm::BasicBlock::Create(*m_context, "false_print", rts_func);
  auto* cond = m_builder->CreateICmpEQ(
      rts_func->getArg(0), llvm::ConstantInt::getBool(Boolean(), true));
  m_builder->CreateCondBr(cond, true_print, false_print);
  m_builder->SetInsertPoint(true_print);
  args.emplace_back(true_str);
  m_builder->CreateCall(libc_func, args, "call");
  m_builder->SetInsertPoint(false_print);
  args.clear();
  args.emplace_back(false_str);
  m_builder->CreateCall(libc_func, args, "call");

  m_builder->CreateRetVoid();
  llvm::verifyFunction(*rts_func);

  // ------------------------------------------------------------------------

  args.clear();
  arg_types.clear();

  // getchar
  libc_ftype = llvm::FunctionType::get(Int32(), false);

  libc_func =
      llvm::Function::Create(libc_ftype, llvm::Function::ExternalLinkage,
                             llvm::Twine("getchar"), *m_module);
  libc_func->setCallingConv(llvm::CallingConv::C);

  // printc
  fmt = m_builder->CreateGlobalString(llvm::StringRef("%c"), "fmtchar", 0,
                                      m_module.get());
  libc_func = m_module->getFunction(llvm::StringRef("printf"));

  arg_types.emplace_back(Int32());
  rts_ftype = llvm::FunctionType::get(Void(), arg_types, false);
  rts_func = llvm::Function::Create(rts_ftype, llvm::Function::PrivateLinkage,
                                    llvm::Twine("printc"), *m_module);
  for (auto& arg : rts_func->args()) {
    arg.setName("char");
  }
  body = llvm::BasicBlock::Create(*m_context, "entry", rts_func);
  m_builder->SetInsertPoint(body);
  args.emplace_back(fmt);
  args.emplace_back(rts_func->getArg(0));

  auto* res = m_builder->CreateCall(libc_func, args, "call");
  m_builder->CreateRet(res);
  llvm::verifyFunction(*rts_func);
}

llvm::Type* IRCodegenVisitor::getType(VType type) const {
  switch (type) {
    case VType::Int:
      return Int32();
    case VType::Str:
      return Str();
    case VType::Bool:
      return Boolean();
    case VType::Void:
      return Void();
      break;
  }
  return Void();
}

// Root node - program. Entry point here.
void IRCodegenVisitor::visit(ASTNode* node) {
  // We need to do a global pass first to get all the globals since the order
  // does not matter in J--
  CodegenGlobals global_pass(this);
  node->accept(&global_pass);

  for (auto const& child : node->children) {
    child->codegen(this);
  }
}

llvm::Value* IRCodegenVisitor::codegen(ASTNode const* node) {
  for (auto const& child : node->children) {
    child->codegen(this);
  }
  // Return a null value
  return llvm::ConstantInt::getNullValue(Int32());
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
  node->body->codegen(this);
  body = m_builder->GetInsertBlock();

  m_builder->CreateBr(cond);

  end->insertInto(func);
  m_builder->SetInsertPoint(end);
  m_while_context.pop();

  return nullptr;
}

llvm::Value* IRCodegenVisitor::codegen(ReturnStmt const* node) {
  if (!node->expr) {
    return m_builder->CreateRetVoid();
  }
  return m_builder->CreateRet(node->expr->codegen(this));
}

llvm::Value* IRCodegenVisitor::codegen(BreakStmt const* /*node*/) {
  // needs to jump to current while's end
  m_builder->CreateBr(m_while_context.top());
  return llvm::ConstantInt::getNullValue(Int32());
}

llvm::Value* IRCodegenVisitor::codegen(BlockStmt const* node) {
  for (auto const& child : node->children) {
    child->codegen(this);
  }
  return llvm::ConstantInt::getNullValue(Int32());
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
  return llvm::ConstantInt::getNullValue(Int32());
}

llvm::Value* IRCodegenVisitor::codegen(UnaryExpr const* node) {
  // PostFix - Return value before inc/dec
  // Prefix -- Return value after inc/dec
  auto* expr = node->expr->codegen(this);

  llvm::LoadInst* load;
  llvm::Value* res;

  switch (node->op) {
    case Op::POSTINC:
      // Only ints can be inc/dec
      load = m_builder->CreateLoad(Int32(), expr);
      res = m_builder->CreateNSWAdd(
          load, llvm::ConstantInt::getSigned(Int32(), 1), "inc");
      m_builder->CreateStore(res, expr);
      return load;
    case Op::POSTDEC:
      load = m_builder->CreateLoad(Int32(), expr);
      res = m_builder->CreateNSWSub(
          load, llvm::ConstantInt::getSigned(Int32(), 1), "dec");
      m_builder->CreateStore(res, expr);
      return load;
    case Op::PREINC:
      load = m_builder->CreateLoad(Int32(), expr);
      res = m_builder->CreateNSWAdd(
          load, llvm::ConstantInt::getSigned(Int32(), 1), "inc");
      m_builder->CreateStore(res, expr);
      return res;
    case Op::PREDEC:
      load = m_builder->CreateLoad(Int32(), expr);
      res = m_builder->CreateNSWSub(
          load, llvm::ConstantInt::getSigned(Int32(), 1), "dec");
      m_builder->CreateStore(res, expr);
      return res;
    case Op::NOT:
      return m_builder->CreateNot(expr);
    case Op::SUB:
      return m_builder->CreateNeg(expr);
    default:
      break;
  }
  return llvm::ConstantInt::getNullValue(Int32());
}

llvm::Value* IRCodegenVisitor::codegen(BinaryExpr const* node) {
  // Binary Expressions are all expressions but bitwise

  llvm::Value* lhs;
  llvm::Value* rhs;

  if (node->op != Op::LAND && node->op != Op::LOR) {
    lhs = node->lhs->codegen(this);
    rhs = node->rhs->codegen(this);
  }

  // TODO(shankar): Make short-circuiting
  switch (node->op) {
    case Op::LAND: {
      lhs = node->lhs->codegen(this);
      auto * curr_block = m_builder->GetInsertBlock();

      auto* lhstrue = llvm::BasicBlock::Create(
          *m_context, "lhs.true", m_builder->GetInsertBlock()->getParent());
      auto* lhsend = llvm::BasicBlock::Create(*m_context, "lhs.end");

      m_builder->CreateCondBr(lhs, lhstrue, lhsend);
      m_builder->SetInsertPoint(lhstrue);

      rhs = node->rhs->codegen(this);
      m_builder->CreateBr(lhsend);

      lhsend->insertInto(m_builder->GetInsertBlock()->getParent());
      m_builder->SetInsertPoint(lhsend);

      auto* phival = m_builder->CreatePHI(Boolean(), 2, "res");
      phival->addIncoming(lhs, curr_block);
      phival->addIncoming(rhs, lhstrue);

      return phival;
    }
    case Op::LOR: {
      lhs = node->lhs->codegen(this);
      auto * curr_block = m_builder->GetInsertBlock();

      auto* lhsfalse = llvm::BasicBlock::Create(
          *m_context, "lhs.false", m_builder->GetInsertBlock()->getParent());
      auto* lhsend = llvm::BasicBlock::Create(*m_context, "lhs.end");
      
      m_builder->CreateCondBr(lhs, lhsend, lhsfalse);
      m_builder->SetInsertPoint(lhsfalse);

      rhs = node->rhs->codegen(this);
      m_builder->CreateBr(lhsend);

      lhsend->insertInto(m_builder->GetInsertBlock()->getParent());
      m_builder->SetInsertPoint(lhsend);

      auto* phival = m_builder->CreatePHI(Boolean(), 2, "res");
      phival->addIncoming(lhs, curr_block);
      phival->addIncoming(rhs, lhsfalse);
      return phival;
    }
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
      return m_builder->CreateICmpEQ(lhs, rhs, "cmp");
    case Op::NE:
      return m_builder->CreateICmpNE(lhs, rhs, "cmp");
    case Op::GT:
      return m_builder->CreateICmpSGT(lhs, rhs, "cmp");
    case Op::LT:
      return m_builder->CreateICmpSLT(lhs, rhs, "cmp");
    case Op::GE:
      return m_builder->CreateICmpSGE(lhs, rhs, "cmp");
    case Op::LE:
      return m_builder->CreateICmpSLE(lhs, rhs, "cmp");
    default:
      break;
  }

  return llvm::ConstantInt::getNullValue(Int32());
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

  return llvm::ConstantInt::getNullValue(Int32());
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

  std::vector<llvm::Value*> argv;
  for (auto const& actual : node->args->actuals) {
    argv.push_back(actual->codegen(this));
  }

  return m_builder->CreateCall(func, argv, "call");
}

llvm::Value* IRCodegenVisitor::codegen(GVarDecl const* /*node*/) {
  return llvm::ConstantInt::getNullValue(Int32());
}

llvm::Value* IRCodegenVisitor::codegen(ParamDecl const* /*node*/) {
  return llvm::ConstantInt::getNullValue(Int32());
}

llvm::Value* IRCodegenVisitor::codegen(Params const* /*node*/) {
  return llvm::ConstantInt::getNullValue(Int32());
}

llvm::Value* IRCodegenVisitor::codegen(Actuals const* /*node*/) {
  return llvm::ConstantInt::getNullValue(Int32());
}

llvm::Value* IRCodegenVisitor::codegen(ActualExpr const* /*node*/) {
  return llvm::ConstantInt::getNullValue(Int32());
}

llvm::Value* IRCodegenVisitor::codegen(NullStmt const* /*node*/) {
  return llvm::ConstantInt::getNullValue(Int32());
}

llvm::Value* IRCodegenVisitor::codegen(FuncDecl const* node) {
  llvm::Function* func = m_module->getFunction(llvm::StringRef(node->id));
  if (!func) {
    m_logger->error("Failed to get function ", node->id, " at line ",
                    node->line);
  }

  llvm::BasicBlock* body = llvm::BasicBlock::Create(*m_context, "entry", func);

  m_builder->SetInsertPoint(body);

  auto it = node->params->params.begin();
  for (auto& arg : func->args()) {
    auto* alloc =
        m_builder->CreateAlloca(arg.getType(), nullptr, (arg.getName()));
    m_builder->CreateStore(&arg, alloc);
    (*it)->symbol->llvm_Value = alloc;
    ++it;
  }

  node->body->codegen(this);

  if (!body->getTerminator()) {
    if (func->getReturnType()->isVoidTy()) {
      m_builder->CreateRetVoid();
    }
  }

  llvm::verifyFunction(*func);
  return func;
}

llvm::Value* IRCodegenVisitor::codegen(MFuncDecl const* node) {
  llvm::Function* func = m_module->getFunction(llvm::StringRef(node->id));
  if (!func) {
    m_logger->error("Failed to get function ", node->id, " at line ",
                    node->line);
  }
  m_mainFuncID = node->id;

  llvm::BasicBlock* body = llvm::BasicBlock::Create(*m_context, "entry", func);

  m_builder->SetInsertPoint(body);
  node->body->codegen(this);

  // if (!body->getTerminator()) {
  m_builder->CreateRetVoid();
  // }

  llvm::verifyFunction(*func);
  return func;
}

llvm::Value* IRCodegenVisitor::codegen(VarDecl const* node) {
  // llvm::Constant* zero_ival = llvm::ConstantInt::getSigned(Int32(), 0);
  // llvm::Constant* zero_bval = llvm::ConstantInt::getBool(Boolean(), false);
  llvm::AllocaInst* var;

  auto* curr_func = m_builder->GetInsertBlock()->getParent();

  auto revert_point = m_builder->saveAndClearIP();  // save restore IP
  m_builder->SetInsertPointPastAllocas(curr_func);  // set IP to last alloca
  switch (node->type) {
    case VType::Int:
      var =
          m_builder->CreateAlloca(Int32(), nullptr, llvm::StringRef(node->id));
      if (!var) {
        m_logger->error("Failed to create local variable of type Int");
      }
      // m_builder->CreateStore(zero_ival, var);
      break;
    case VType::Str:
      // Basically just a pointer to some string buffer. And these pointers can
      // change! Strings are immutable so all the strings used will be declared
      var = m_builder->CreateAlloca(llvm::Type::getInt8PtrTy(*m_context),
                                    nullptr, llvm::StringRef(node->id));
      if (!var) {
        m_logger->error("Failed to create local variable of type Str");
      }
      // m_builder->CreateStore(ero_bval, var);
      break;
    case VType::Bool:
      var = m_builder->CreateAlloca(Boolean(), nullptr,
                                    llvm::StringRef(node->id));
      if (!var) {
        m_logger->error("Failed to create local variable of type bool");
      }
      // m_builder->CreateStore(zero_bval, var);
      break;
    case VType::Void:
      m_logger->error("Cannot create VOID type variable");
    default:
      m_logger->error("Failed to create variable. Unknown type");
  }
  node->symbol->llvm_Value = var;
  m_builder->restoreIP(revert_point);  // Revert back IP
  return var;
}
