#include "analyzer.h"
#include "globalVisitor.h"
#include <memory>
#include <unordered_map>
#include "common/ast.h"
#include "common/symtab.h"

// #include "funcVisitor.h"
// #include "typeVisitor.h"

SemanticAnalyzer::SemanticAnalyzer(std::shared_ptr<Logger>& logger)
    : m_logger(logger) {
  m_symtab = std::make_shared<SymbolTable>(logger);
  // RTS functions
  std::unordered_map<std::string, Symbol> const rts = {
      {"getchar",
       FunctionSig(VType::Int, std::move(std::vector<VType>()), false, 0)},
      {"halt",
       FunctionSig(VType::Void, std::move(std::vector<VType>()), false, 0)},
      {"printb",
       FunctionSig(VType::Void, std::move(std::vector<VType>{VType::Bool}),
                   false, 0)},
      {"printc",
       FunctionSig(VType::Void, std::move(std::vector<VType>{VType::Int}),
                   false, 0)},
      {"printi",
       FunctionSig(VType::Void, std::move(std::vector<VType>{VType::Int}),
                   false, 0)},
      {"prints",
       FunctionSig(VType::Void, std::move(std::vector<VType>{VType::Str}),
                   false, 0)}};

  for (auto const& func : rts) {
    m_symtab->insert(Scope::GLOBAL, func.first, func.second);
  }
}

bool SemanticAnalyzer::analyze(nodePtr& node) {
  Visitor globals = GlobalsVisitor(m_symtab, m_logger);
  // Visitor func_anal = FunctionVisitor(m_symtab);
  // Visitor type_anal = TypeAnalVisitor(m_symtab);

  node->accept(&globals);

  // get globals
  // traverse(node,
  //         [&](nodePtr &tree) { this->getGlobals(tree); },
  //         [](nodePtr &node) {;}
  //         );

  return true;
}
