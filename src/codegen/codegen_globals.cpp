#include "codegen/codegen.h"

// Only process globals and adds function prototypes and global variables.

IRCodegenVisitor::CodegenGlobals::CodegenGlobals(IRCodegenVisitor const* gen)
    : m_gen(gen) {}

void IRCodegenVisitor::CodegenGlobals::visit(ASTNode* node) {
  for (auto const& child : node->children) {
    child->accept(this);
  }
}

void IRCodegenVisitor::CodegenGlobals::visit(FuncDecl* node) {
  llvm::FunctionType* func_type;

  if (!node->symbol->param_types.has_value()) {
    func_type =
        llvm::FunctionType::get(m_gen->getType(node->return_type), false);
  } else {
    std::vector<llvm::Type*> types;
    for (auto const& ptype : node->symbol->param_types.value()) {
      types.emplace_back(m_gen->getType(ptype));
    }
    func_type = llvm::FunctionType::get(m_gen->getType(node->return_type),
                                        types, false);
  }

  llvm::Function* func =
      llvm::Function::Create(func_type, llvm::Function::PrivateLinkage,
                             llvm::StringRef(node->id), *m_gen->m_module);

  auto* it1 = func->args().begin();
  for (auto const& param : node->params->params) {
    if (m_gen->getType(param->type) == it1->getType()) {
      it1->setName(param->id);
    }
  }
}

void IRCodegenVisitor::CodegenGlobals::visit(MFuncDecl* node) {
  llvm::FunctionType* func_type = llvm::FunctionType::get(m_gen->Void(), false);

  llvm::Function::Create(func_type, llvm::Function::PrivateLinkage,
                         llvm::StringRef(node->id), *m_gen->m_module);
}

void IRCodegenVisitor::CodegenGlobals::visit(GVarDecl* node) {
  // Since variable declaration, create a globalvariable in LLVM and add it to
  // symbol table
  llvm::Constant* zero_ival = llvm::ConstantInt::getSigned(m_gen->Int32(), 0);
  llvm::Constant* zero_bval =
      llvm::ConstantInt::getBool(m_gen->Boolean(), false);
  llvm::GlobalVariable* gvar;

  switch (node->type) {
    case VType::Int:
      gvar = new llvm::GlobalVariable(*m_gen->m_module, m_gen->Int32(),
                                      /*isConstant*/ false,
                                      llvm::GlobalValue::PrivateLinkage,
                                      zero_ival, llvm::StringRef(node->id));
      if (!gvar) {
        m_gen->m_logger->error(
            "Failed to create LLVM global variable of type Int");
      }
      break;
    case VType::Str:
      gvar = new llvm::GlobalVariable(*m_gen->m_module, m_gen->Str(),
                                      /*isConstant*/ true,
                                      llvm::GlobalValue::PrivateLinkage,
                                      nullptr, llvm::StringRef(node->id));
      if (!gvar) {
        m_gen->m_logger->error(
            "Failed to create LLVM global variable of type string");
      }
      break;
    case VType::Bool:
      gvar = new llvm::GlobalVariable(*m_gen->m_module, m_gen->Boolean(),
                                      /*isConstant*/ false,
                                      llvm::GlobalValue::PrivateLinkage,
                                      zero_bval, llvm::StringRef(node->id));
      if (!gvar) {
        m_gen->m_logger->error(
            "Failed to create LLVM global variable of type bool");
      }
      break;
    case VType::Void:
      m_gen->m_logger->error("Cannot create VOID type variable");
    default:
      m_gen->m_logger->error("Failed to create variable. Unknown type");
  }
  node->symbol->llvm_Value = gvar;
}
