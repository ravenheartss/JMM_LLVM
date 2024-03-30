#include "codegen.h"
#include <llvm/ADT/StringRef.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Verifier.h>
#include <memory>

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
  libc_ftype =
      llvm::FunctionType::get(Int32(), false);

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

  auto * res = m_builder->CreateCall(libc_func, args, "call");
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
}

// Root node - program. Entry point here.
void IRCodegenVisitor::visit(ASTNode* node) {
  // We need to do a global pass first to get all the globals since the order
  // does not matter in J--
  CodegenGlobals global_pass(this);
  node->accept(&global_pass);

  for (auto const& child : node->children) {
    child->accept(this);
  }
}

void IRCodegenVisitor::visit(IfStmt* node) { node->codegen(this); }

void IRCodegenVisitor::visit(IfElseStmt* node) { node->codegen(this); }

void IRCodegenVisitor::visit(WhileStmt* node) { node->codegen(this); }

void IRCodegenVisitor::visit(ReturnStmt* node) { node->codegen(this); }

void IRCodegenVisitor::visit(BreakStmt* node) { node->codegen(this); }

void IRCodegenVisitor::visit(BlockStmt* node) {
  for (auto const& child : node->children) {
    child->accept(this);
  }
}

void IRCodegenVisitor::visit(ExprStmt* node) { node->codegen(this); }

// All expressions in a block are part of ExprStmt so the above will take care
// of all this.

// void IRCodegenVisitor::visit(NullStmt* node) { /* none */}

// void IRCodegenVisitor::visit(IdExpr* node) { } // codegen

// void IRCodegenVisitor::visit(LitExpr* node) { /* see codegen */
// }

// void IRCodegenVisitor::visit(UnaryExpr* node) {}

// void IRCodegenVisitor::visit(BinaryExpr* node) {}

// void IRCodegenVisitor::visit(BitwiseExpr* node) {}

// void IRCodegenVisitor::visit(AssignExpr* node) {}

// void IRCodegenVisitor::visit(FuncCallExpr* node) {} // codegen

void IRCodegenVisitor::visit(FuncDecl* node) {
  llvm::Function* func = m_module->getFunction(llvm::StringRef(node->id));
  if (!func) {
    m_logger->error("Failed to get function ", node->id, " at line ",
                    node->line);
  }
  m_curr_func = func;

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

  node->body->accept(this);

  if (!body->getTerminator()) {
    if (func->getReturnType()->isVoidTy()) {
      m_builder->CreateRetVoid();
    } 
  }

  llvm::verifyFunction(*func);
  m_curr_func = nullptr;
}

void IRCodegenVisitor::visit(MFuncDecl* node) {
  llvm::Function* func = m_module->getFunction(llvm::StringRef(node->id));
  if (!func) {
    m_logger->error("Failed to get function ", node->id, " at line ",
                    node->line);
  }
  m_curr_func = func;
  m_mainFuncID = node->id;

  llvm::BasicBlock* body = llvm::BasicBlock::Create(*m_context, "entry", func);

  m_builder->SetInsertPoint(body);
  node->body->accept(this);

  // if (!body->getTerminator()) {
  m_builder->CreateRetVoid();
  // }

  llvm::verifyFunction(*func);
  m_curr_func = nullptr;
}

void IRCodegenVisitor::visit(VarDecl* node) {
  llvm::Constant* zero_ival = llvm::ConstantInt::getSigned(Int32(), 0);
  llvm::Constant* zero_bval = llvm::ConstantInt::getBool(Boolean(), false);
  llvm::AllocaInst* var;

  auto curr_func = m_builder->GetInsertBlock()->getParent();

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
  }
  node->symbol->llvm_Value = var;
  m_builder->restoreIP(revert_point);  // Revert back IP
}

// void IRCodegenVisitor::visit(GVarDecl* node) {
// }

// void IRCodegenVisitor::visit(ParamDecl* node) {}
//
// void IRCodegenVisitor::visit(Params* node) {}

// void IRCodegenVisitor::visit(ActualExpr* node) {}
//
// void IRCodegenVisitor::visit(Actuals* node) {}
