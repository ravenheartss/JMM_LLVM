#include "ast.h"

#include <iostream>
#include <memory>
#include <variant>

// There are wayy better ways to print the tree but this is just for debug

void Printer(StmtType stmt, std::shared_ptr<ASTNode> node, int indent);
void Printer(ExprType expr, std::shared_ptr<ASTNode> node, int indent);
void Printer(DeclType decl, std::shared_ptr<ASTNode> node, int indent);


struct ValueVisitor
{
    void operator()(std::string val) const 
    {
        std::cout << val;
    }
    void operator()(int32_t val) const
    {
        std::cout << val;
    }
    void operator()(bool val) const
    {
        std::cout << val;
    }
    void operator()(std::monostate val) const
    {
    }
};

void ASTNode::print(int indent)
{
    std::cout << std::string(indent, '\t');
    switch (this->type) 
    {
        case NType::Prog: std::cout << "Program\n";     break;
        default:                                        break;
    }

    for (auto child : children)
    {
        switch (child->type) 
        {
            case NType::Stmt:   Printer(std::get<0>(child->kind), child, indent+1); break;
            case NType::Expr:   Printer(std::get<1>(child->kind), child, indent+1); break;
            case NType::Decl:   Printer(std::get<2>(child->kind), child, indent+1); break;
            default: break;
        }
    }
}


void Printer(StmtType stmt, std::shared_ptr<ASTNode> node, int indent)
{
    std::cout << std::string(indent, '\t');
    switch(stmt)
    {
        case StmtType::If       : std::cout << "IfStmt";        break;
        case StmtType::IfElse   : std::cout << "IfElseStmt";    break;
        case StmtType::While    : std::cout << "WhileStmt";     break;
        case StmtType::FCall    : std::cout << "FuncCall";      break;
        case StmtType::Goto     : std::cout << "GotoStmt";      break;
        case StmtType::Return   : std::cout << "ReturnStmt";    break;
        case StmtType::Break    : std::cout << "BreakStmt";     break;
        case StmtType::Block    : std::cout << "BlockStmt";     break;
        case StmtType::Expr     : std::cout << "ExprStmt";      break;
        case StmtType::Null     : std::cout << "EmptyStmt";     break;
        default                 : break;
    }

    if (node->line)
        std::cout << " Line: " << node->line;

    std::cout << '\n';

    for (auto child : node->children)
    {
        switch (child->type) 
        {
            case NType::Stmt:   Printer(std::get<0>(child->kind), child, indent+1); break;
            case NType::Expr:   Printer(std::get<1>(child->kind), child, indent+1); break;
            case NType::Decl:   Printer(std::get<2>(child->kind), child, indent+1); break;
            default: break;
        }
    }
}

void Printer(ExprType expr, std::shared_ptr<ASTNode> node, int indent)
{
    std::cout << std::string(indent, '\t');
    switch(expr)
    {
        case ExprType::Id       : std::cout << "ID";            break;
        case ExprType::Lit      : std::cout << "Literal";       break;
        case ExprType::Unary    : std::cout << "Unary Expr";    break;
        case ExprType::Binary   : std::cout << "Binary Expr";   break;
        case ExprType::Bitwise  : std::cout << "Bitwise Expr";  break;
        case ExprType::Assign   : std::cout << "=";   break;
        case ExprType::FuncCall : std::cout << "Func Call";     break;
        case ExprType::Actuals  : std::cout << "Actuals";       break;
        case ExprType::Actual   : std::cout << "Actual";        break;
        default                 : break;
    }

    if (node->line)
        std::cout << " Line: " << node->line;

    if (node->val_type.has_value()) 
    {
        switch (node->val_type.value())
        {
            case VType::Int:    std::cout << " Type: Int";  break;
            case VType::Str:    std::cout << " Type: Str";  break;
            case VType::Bool:   std::cout << " Type: Bool"; break;
            case VType::Void:   std::cout << " Type: Void"; break;
                break;
        }
    }

    if (node->value.index() != 0)
    {
        std::cout << " Value: ";
        std::visit(ValueVisitor{}, node->value); // print value
    }

    if (node->op.has_value())
    {
        std::cout << " Op: ";
        switch(node->op.value())
        {
            case Op::BAND:      std::cout << "&";       break;
            case Op::BOR:       std::cout << "|";       break;
            case Op::LAND:      std::cout << "&&";      break;
            case Op::LOR:       std::cout << "||";      break;
            case Op::XOR:       std::cout << "^";       break;
            case Op::ADD:       std::cout << "+";       break;
            case Op::POSTINC:   std::cout << "Post ++"; break;
            case Op::POSTDEC:   std::cout << "Post --"; break;
            case Op::PREINC:    std::cout << "Pre ++";  break;
            case Op::PREDEC:    std::cout << "Pre --";  break;
            case Op::SUB:       std::cout << "-";       break;
            case Op::MULT:      std::cout << "*";       break;
            case Op::DIV:       std::cout << "/";       break;
            case Op::MOD:       std::cout << "%";       break;
            case Op::NOT:       std::cout << "!";       break;
            case Op::EQ:        std::cout << "==";      break;
            case Op::NE:        std::cout << "!=";      break;
            case Op::GT:        std::cout << ">";       break;
            case Op::LT:        std::cout << "<";       break;
            case Op::GE:        std::cout << ">=";      break;
            case Op::LE:        std::cout << "<=";      break;
            case Op::LSHIFT:    std::cout << "<<";      break;
            case Op::RSHIFT:    std::cout << ">>";      break;
            default:            break;
          break;
        }
    }

    std::cout << '\n';

    for (auto child : node->children)
    {
        switch (child->type) 
        {
            case NType::Stmt:   Printer(std::get<0>(child->kind), child, indent+1); break;
            case NType::Expr:   Printer(std::get<1>(child->kind), child, indent+1); break;
            case NType::Decl:   Printer(std::get<2>(child->kind), child, indent+1); break;
            default: break;
        }
    }
    // for (auto &child : node->children)
    //     std::visit(Visitor{}, child->kind, child, indent+1);
}

void Printer(DeclType decl, std::shared_ptr<ASTNode> node, int indent)
{
    std::cout << std::string(indent, '\t');
    switch(decl)
    {
        case DeclType::Func     : std::cout << "Func Decl";	break;
        case DeclType::MFunc    : std::cout << "Main Func Decl";	break;
        case DeclType::Var      : std::cout << "Var Decl";	break;
        case DeclType::GVar     : std::cout << "Global Var Decl";	break;
        case DeclType::Params   : std::cout << "Params";	break;
        case DeclType::Param    : std::cout << "Param";	break;
        default                 : break;
    }
    if (node->line)
        std::cout << " Line: " << node->line;

    if (node->val_type.has_value()) 
    {
        switch (node->val_type.value())
        {
            case VType::Int:    std::cout << " Type: Int";  break;
            case VType::Str:    std::cout << " Type: Str";  break;
            case VType::Bool:   std::cout << " Type: Bool"; break;
            case VType::Void:   std::cout << " Type: Void"; break;
                break;
        }
    }

    if (node->value.index() != 0)
    {
        std::cout << " Value: ";
        std::visit(ValueVisitor{}, node->value); // print value
    }

    std::cout << '\n';

    for (auto child : node->children)
    {
        switch (child->type) 
        {
            case NType::Stmt:   Printer(std::get<0>(child->kind), child, indent+1); break;
            case NType::Expr:   Printer(std::get<1>(child->kind), child, indent+1); break;
            case NType::Decl:   Printer(std::get<2>(child->kind), child, indent+1); break;
            default: break;
        }
    }

}
