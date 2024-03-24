#ifndef JMM_SYMTAB_H
#define JMM_SYMTAB_H

#include "errwarn.h"
#include "globals.h"
#include <memory>
#include <optional>
#include <stack>
#include <unordered_map>
#include <utility>
#include <vector>

// These are not in globals because these are essential only to the symbol table
enum class Scope { RTS, GLOBAL, FUNC, BLOCK };

enum class SymType { VAR, FUNC };

struct Symbol {
  SymType symbol_type;  // either a var or func
  VType v_type;         // type of variable or func ret type
  int line;
  std::optional<std::vector<VType>> param_types;
  bool isMain;

  Symbol(VType type, std::vector<VType> params, int lineno)
      : symbol_type(SymType::FUNC),
        v_type(type),
        line(lineno),
        param_types(std::move(params)),
        isMain(false) {}

  explicit Symbol(int lineno)
      : symbol_type(SymType::FUNC),
        v_type(VType::Void),
        line(lineno),
        param_types(std::nullopt),
        isMain(true) {}

  Symbol(VType type, int lineno)
      : symbol_type(SymType::VAR),
        v_type(type),
        line(lineno),
        param_types(std::nullopt),
        isMain(false) {}

  void print() {
    switch (symbol_type) {
      case SymType::VAR:
        std::cerr << "Symbol; Type: " << v_type << "; Declared at line " << line
                  << "\n";
        break;
      case SymType::FUNC:
        if (isMain) {
          std::cerr << "Symbol; Main function";
        } else {
          std::cerr << "Symbol; Return Type: " << v_type;
          if (param_types.has_value()) {
            std::cerr << "; Param Types: ";
            for (auto const& ptype : param_types.value()) {
              std::cerr << ptype << " ";
            }
          }
        }
        std::cerr << "; Declared at line " << line << "\n";
        break;
    }
  }
};

class SymbolTable {
 public:
  explicit SymbolTable(std::shared_ptr<Logger>& logger) : m_logger(logger) {}

  bool insert(Scope scope, std::string const& name,
              std::shared_ptr<Symbol> const& signature) {
    switch (scope) {
      case Scope::GLOBAL:
        return m_global.insert({name, signature}).second;
      case Scope::FUNC:
        return m_func.insert({name, signature}).second;
      case Scope::RTS:
        return m_rts.insert({name, signature}).second;
      case Scope::BLOCK:
        if (m_blocks.empty()) {
          m_logger->warning("Failed to insert symbol: ", name, "Not in block.");
          break;
        }
        return m_blocks.top().insert({name, signature}).second;
      default:
        break;
    }
    return false;
  }

  std::shared_ptr<Symbol> query(Scope scope, std::string const& name) {
    switch (scope) {
      case Scope::GLOBAL:
        if (auto res = m_global.find(name); res != m_global.end()) {
          return res->second;
        }
        break;
      case Scope::RTS:
        if (auto res = m_rts.find(name); res != m_rts.end()) {
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

    return nullptr;
  }

  std::optional<std::pair<Scope, std::shared_ptr<Symbol>>> query(
      std::string const& name) {
    if (auto sym = query(Scope::BLOCK, name); sym) {
      return {{Scope::BLOCK, sym}};
    }

    if (auto sym = query(Scope::FUNC, name); sym) {
      return {{Scope::FUNC, sym}};
    }

    if (auto sym = query(Scope::GLOBAL, name); sym) {
      return {{Scope::GLOBAL, sym}};
    }

    if (auto sym = query(Scope::RTS, name); sym) {
      return {{Scope::RTS, sym}};
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

  // get the current depth of the block
  size_t current_scope() { return m_blocks.size(); }

  void print() {
    std::cout << "Globals:";
    for (auto const& entry : m_global) {
      std::cerr << entry.first << '\n';
    }
  }

 private:
  // Having a shared pointer to the symbol ensures that it doesn't get
  // destroyed Or ensures it gets destroyed when the ref count hits 0
  std::shared_ptr<Logger> m_logger;
  std::unordered_map<std::string, std::shared_ptr<Symbol>> m_rts;
  std::unordered_map<std::string, std::shared_ptr<Symbol>> m_global;
  // For formals since they are available to entire function scope
  std::unordered_map<std::string, std::shared_ptr<Symbol>> m_func;
  std::stack<std::unordered_map<std::string, std::shared_ptr<Symbol>>> m_blocks;
};
#endif  // !JMM_SYMTAB_H
