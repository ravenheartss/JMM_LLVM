#ifndef JMM_AST_H
#define JMM_AST_H

#include <string>
#include <variant>
#include <utility>
#include "token.h"

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
    Assign,
    Null
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

        // ASTNode(ASTNode&& node) noexcept
        //     : type(node.type)
        //     , kind(node.kind)
        //     , value(node.value)
        //     , val_type(node.val_type)
        //     , op(node.op)
        //     , line(node.line)
        //     , children(node.children)
        //     , sibling(node.sibling)
        // {
        //
        // }

        // ASTNode& operator=(ASTNode&& rhs) noexcept
        // {
        //     std::swap(type, rhs.type);
        //     std::swap(kind, rhs.kind);
        //     std::swap(value, rhs.value);
        //     std::swap(val_type, rhs.val_type);
        //     std::swap(op, rhs.op);
        //     std::swap(line, rhs.line);
        //     std::swap(children, rhs.children);
        //     std::swap(sibling, rhs.sibling);
        //     return *this;
        // }

        // ASTNode& operator=(const ASTNode& rhs)
        // {
        //     if (&rhs != this)
        //     {
        //         ASTNode temp(rhs);
        //         std::swap(*this, temp);
        //     }
        //     return *this;
        // }

        ~ASTNode() noexcept
        {
            children.clear();
            sibling.reset();
        }

    public:
        NType type; // node type
        std::variant<StmtType,ExprType,DeclType> kind;
        std::variant<std::string,int32_t,bool> value;
        std::optional<VType> val_type; // Types
        std::optional<Token> op;   // For operators
        uint32_t line;

        // Idk. Maybe unique? But not possible since if it is unique, each node will have it
        // unique pointers to children but then since the child can access it's immediate sibling,
        // it would be a violation
        std::vector<std::shared_ptr<ASTNode>> children; 
        std::shared_ptr<ASTNode> sibling; 
};


// Version 2 -- using oop and visitor design

// class AST;
//     class Prog;
//     class Expr;
//     class Stmt;


#endif // !JMM_AST_H
