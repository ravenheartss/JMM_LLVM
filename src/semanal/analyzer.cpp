#include "analyzer.h"
#include "global_visitor.h"
#include <memory>
#include <unordered_map>
#include "common/ast.h"
#include "common/symtab.h"
#include "semanal/func_visitor.h"
#include "semanal/type_visitor.h"

SemanticAnalyzer::SemanticAnalyzer(std::shared_ptr<Logger>& logger)
    : m_logger(logger) {
  m_symtab = std::make_shared<SymbolTable>(logger);
  // RTS functions
  std::unordered_map<std::string, std::shared_ptr<Symbol>> const rts = {
      {"getchar",
       std::make_shared<Symbol>(VType::Int, std::vector<VType>(), 0)},
      {"halt", std::make_shared<Symbol>(VType::Void, std::vector<VType>(), 0)},
      {"printb", std::make_shared<Symbol>(VType::Void,
                                          std::vector<VType>{VType::Bool}, 0)},
      {"printc", std::make_shared<Symbol>(VType::Void,
                                          std::vector<VType>{VType::Int}, 0)},
      {"printi", std::make_shared<Symbol>(VType::Void,
                                          std::vector<VType>{VType::Int}, 0)},
      {"prints", std::make_shared<Symbol>(VType::Void,
                                          std::vector<VType>{VType::Str}, 0)}};

  for (auto const& func : rts) {
    m_symtab->insert(Scope::RTS, func.first, func.second);
  }
}

bool SemanticAnalyzer::analyze(nodePtr& node) {
  auto globals = GlobalsVisitor(m_symtab, m_logger);
  node->accept(&globals);

  FunctionVisitor funcs = FunctionVisitor(m_symtab, m_logger);
  node->accept(&funcs);

  TypeVisitor typeanal = TypeVisitor(m_symtab, m_logger);
  node->accept(&typeanal);

  return true;
}
