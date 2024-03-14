#ifndef JMM_AST_H
#define JMM_AST_H

#include <string>
#include <variant>
#include <vector>
#include <memory>
#include <optional>

// Easiest way is to just have one class Node and have enum types

enum class NType : uint8_t
{
    Prog,
    Stmt,
    Expr,
    Decl
};

enum class StmtType : uint8_t
{
    If,
    IfElse,
    While,
    FCall,
    Goto,
    Return,
    Break,
    Block,
    Expr,
    Null,
};

enum class ExprType : uint8_t
{
    Id,
    Lit,     // Integer, String, Boolean
    Unary,
    Binary,
    Bitwise,
    Assign,
    FuncCall,
    Actuals,    // High level node/wrapper around all the actuals (args)
    Actual
};

enum class DeclType : uint8_t
{
    Func,
    MFunc,  // Main function
    Var,
    GVar,   // Global variable
    Params, // higher level node/wrapper around all the parameters
    Param
};

enum class VType : uint8_t
{
    Int,
    Str,
    Bool,
    Void
};


enum class Op : uint8_t
{
    BAND,
    BOR,
    LAND,
    LOR,
    XOR,
    ADD,
    POSTINC,
    POSTDEC,
    PREINC,
    PREDEC,
    SUB,
    MULT,
    DIV,
    MOD,
    NOT,
    EQ,
    NE,
    GT,
    LT,
    GE,
    LE,
    LSHIFT,
    RSHIFT
};

// TODO: Print helpers
class ASTNode
{
    public:
        ASTNode() = default;
        ASTNode(NType nodeType) : type(nodeType) {}
        ASTNode(NType nodeType, std::variant<StmtType,ExprType,DeclType> nodeKind)
            : type(nodeType), kind(nodeKind) {}

        ~ASTNode() noexcept
        {
            children.clear();
        }

    public:
        NType type; // node type
        std::variant<StmtType,ExprType,DeclType> kind;
        std::variant<std::monostate, std::string,int32_t,bool> value;
        std::optional<VType> val_type; // Types
        std::optional<Op> op;   // For operators
        uint32_t line{0};

        std::vector<std::unique_ptr<ASTNode>> children; 
        void print(int indent=0);

};





#endif // !JMM_AST_H
