#ifndef JMM_AST_H
#define JMM_AST_H

#include <string>
#include <variant>
#include "token.h"

// Easiest way is to just have one class Node and have enum types

// Not yet tested. WIP
// TODO: Print helpers
class ASTNode
{
    public:
        enum class NType 
        {
            Prog,
            Stmt,
            Expr,
            Decl
        };

        enum class StmtType 
        {
            If,
            IfElse,
            While,
            FCall,
            Return,
            Break,
            Block,
            Null
        };

        enum class ExprType 
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

        enum class DeclType 
        {
            Func,
            MFunc,  // Main function
            Var,
            Params, // higher level node/wrapper around all the parameters
            Param
        };

        enum class VType
        {
            Array,
            Int,
            Str,
            Bool,
            Void
        };

    public:
        ASTNode() = default;
        ASTNode(NType nodeType) : type(nodeType) {}
        ASTNode(NType nodeType, std::variant<StmtType,ExprType,DeclType> nodeKind)
            : type(nodeType), kind(nodeKind) {}

        NType type; // node type
        std::variant<StmtType,ExprType,DeclType> kind;
        std::variant<std::string,int32_t,bool> value;
        VType val_type; // Types
        Token op;   // For operators
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
