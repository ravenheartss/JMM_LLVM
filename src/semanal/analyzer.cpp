#include "analyzer.h"
#include "global_visitor.h"
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
       Symbol(VType::Int, std::move(std::vector<VType>()), false, 0)},
      {"halt", Symbol(VType::Void, std::move(std::vector<VType>()), false, 0)},
      {"printb", Symbol(VType::Void, std::move(std::vector<VType>{VType::Bool}),
                        false, 0)},
      {"printc", Symbol(VType::Void, std::move(std::vector<VType>{VType::Int}),
                        false, 0)},
      {"printi", Symbol(VType::Void, std::move(std::vector<VType>{VType::Int}),
                        false, 0)},
      {"prints", Symbol(VType::Void, std::move(std::vector<VType>{VType::Str}),
                        false, 0)}};

  for (auto const& func : rts) {
    m_symtab->insert(Scope::GLOBAL, func.first, func.second);
  }
}

bool SemanticAnalyzer::analyze(nodePtr& node) {
  auto globals = GlobalsVisitor(m_symtab, m_logger);

  node->accept(&globals);

  m_symtab->print();
  return true;
}
