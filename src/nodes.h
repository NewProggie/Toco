#ifndef TOCO_NODES_H
#define TOCO_NODES_H

#include <llvm/IR/Value.h>
#include <string>
#include <vector>

namespace toco {

class LLVMCodeGenContext;
class Statement;
class Expression;
class VariableDeclaration;

using Statements = std::vector<Statement *>;
using Expressions = std::vector<Expression *>;
using Variables = std::vector<VariableDeclaration *>;

struct Node {
    virtual ~Node() {}
    virtual llvm::Value *generateCode(LLVMCodeGenContext &context) {
        return nullptr;
    }
};

struct Expression : public Node {};

struct Statement : public Node {};

struct Integer : public Expression {
    long long value;
    Integer(long long value) : value(value) {}
    virtual llvm::Value *generateCode(LLVMCodeGenContext &context);
};

struct Double : public Expression {
    double value;
    Double(double value) : value(value) {}
    virtual llvm::Value *generateCode(LLVMCodeGenContext &context);
};

struct Identifier : public Expression {
    std::string name;
    Identifier(const std::string &name) : name(name) {}
    virtual llvm::Value *generateCode(LLVMCodeGenContext &context);
};

struct MethodCall : public Expression {
    const Identifier &id;
    Expressions arguments;
    MethodCall(const Identifier &id, Expressions &arguments)
        : id(id), arguments(arguments) {}
    MethodCall(const Identifier &id) : id(id) {}
    virtual llvm::Value *generateCode(LLVMCodeGenContext &context);
};

struct BinaryOperator : public Expression {
    int op;
    Expression &lhs;
    Expression &rhs;
    BinaryOperator(Expression &lhs, int op, Expression &rhs)
        : lhs(lhs), op(op), rhs(rhs) {}
    virtual llvm::Value *generateCode(LLVMCodeGenContext &context);
};

struct Assignment : public Expression {
    Identifier &lhs;
    Expression &rhs;
    Assignment(Identifier &lhs, Expression &rhs) : lhs(lhs), rhs(rhs) {}
    virtual llvm::Value *generateCode(LLVMCodeGenContext &context);
};

struct Block : public Expression {
    Statements statements;
    Block() = default;
    virtual llvm::Value *generateCode(LLVMCodeGenContext &context);
};

struct ExpressionStatement : public Statement {
    Expression &expression;
    ExpressionStatement(Expression &expression) : expression(expression) {}
    virtual llvm::Value *generateCode(LLVMCodeGenContext &context);
};

struct ReturnStatement : public Statement {
    Expression &expression;
    ReturnStatement(Expression &expression) : expression(expression) {}
    virtual llvm::Value *generateCode(LLVMCodeGenContext &context);
};

struct VariableDeclaration : public Statement {
    const Identifier &type;
    Identifier &id;
    Expression *assignmentExpression;
    VariableDeclaration(const Identifier &type, Identifier &id)
        : type(type), id(id) {
        assignmentExpression = nullptr;
    }
    VariableDeclaration(const Identifier &type, Identifier &id,
                        Expression *assignmentExpression)
        : type(type), id(id), assignmentExpression(assignmentExpression) {}
    virtual llvm::Value *generateCode(LLVMCodeGenContext &context);
};

struct ExternDeclaration : public Statement {
    const Identifier &type;
    const Identifier &id;
    Variables arguments;
    ExternDeclaration(const Identifier &type, const Identifier &id,
                      const Variables &arguments)
        : type(type), id(id), arguments(arguments) {}
    virtual llvm::Value *generateCode(LLVMCodeGenContext &context);
};

struct FunctionDeclaration : public Statement {
    const Identifier &type;
    const Identifier &id;
    Variables arguments;
    Block &block;
    FunctionDeclaration(const Identifier &type, const Identifier &id,
                        const Variables &arguments, Block &block)
        : type(type), id(id), arguments(arguments), block(block) {}
    virtual llvm::Value *generateCode(LLVMCodeGenContext &context);
};
}

#endif // TOCO_NODES_H
