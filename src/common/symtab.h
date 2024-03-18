#ifndef JMM_SYMTAB_H
#define JMM_SYMTAB_H

#include "ast.h"
#include "errwarn.h"
#include <memory>
#include <optional>
#include <stack>
#include <unordered_map>
#include <utility>
#include <variant>

enum class Scope {
  GLOBAL,
  FUNC,
  BLOCK

};

struct FunctionSig {
  VType return_type;
  std::vector<VType> param_types;
  bool isMain;
  int line;

  FunctionSig(VType type, std::vector<VType> params, bool main_func, int lineno)
      : return_type(type), param_types(std::move(params)), isMain(main_func), line(lineno) {}
};

struct VarSig {
  VType var_type;
  int line;

  VarSig(VType type, int lineno) : var_type(type), line(lineno) {}
};

using Symbol = std::variant<VarSig, FunctionSig>;

// Helper functions to check if symbol returned by the symbol table is a function or a global variable
// Mainly for better error messages.
inline bool isVar(Symbol const& sym) {
    return sym.index() == 0;
}

inline bool isFunc(Symbol const& sym) {
    return sym.index() == 1;
}

class SymbolTable {
 public:
  explicit SymbolTable(std::shared_ptr<Logger>& logger) : m_logger(logger) {}

  void insert(Scope scope, std::string const& name, Symbol const& signature) {
    switch (scope) {
      case Scope::GLOBAL:
        m_global.insert({name, signature});
        break;
      case Scope::FUNC:
        m_func.insert({name, signature});
        break;
      case Scope::BLOCK:
        if (m_blocks.empty()) {
          m_logger->warning("Failed to insert symbol: ", name, "Not in block.");
          break;
        }
        m_blocks.top().insert({name, signature});
        break;
      default:
        break;
    }
  }

  std::optional<Symbol> query(Scope scope, std::string const& name) {
    switch (scope) {
      case Scope::GLOBAL:
        if (auto res = m_global.find(name); res != m_global.end()) {
          return res->second;
        }
        break;
      case Scope::FUNC:
        if (auto res = m_func.find(name); res != m_func.end()) {
          return res->second;
        }
        break;
      case Scope::BLOCK: {
        if (m_blocks.empty()) {
          break;
        }
        auto temp_stack = m_blocks;
        while (!temp_stack.empty()) {
          auto const& curr_block = temp_stack.top();
          if (auto res = curr_block.find(name); res != curr_block.end()) {
            return res->second;
          }
          temp_stack.pop();
        }
        break;
      }
      default:
        break;
    }

    return {};
  }

  std::optional<std::pair<Scope, Symbol>> query(std::string const& name) {
    if (auto sym = query(Scope::BLOCK, name); sym.has_value()) {
      return {{Scope::BLOCK, sym.value()}};
    }

    if (auto sym = query(Scope::FUNC, name); sym.has_value()) {
      return {{Scope::FUNC, sym.value()}};
    }

    if (auto sym = query(Scope::GLOBAL, name); sym.has_value()) {
      return {{Scope::GLOBAL, sym.value()}};
    }

    return {};
  }

  void enter_block() { m_blocks.emplace(); }

  void exit_func() {
    m_func.clear();
    while (!m_blocks.empty()) {
      m_blocks.pop();  // We don't lose the ref.
    }
  }

  void exit_block() { m_blocks.pop(); }

  size_t current_scope() { return m_blocks.size(); }

 private:
  std::shared_ptr<Logger> m_logger;
  std::unordered_map<std::string, Symbol> m_global;
  std::unordered_map<std::string, Symbol> m_func;
  std::stack<std::unordered_map<std::string, Symbol>> m_blocks;
};
#endif  // !JMM_SYMTAB_H
