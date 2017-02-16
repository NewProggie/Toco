#include "llvm_codegen.h"
#include <llvm-c/Core.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/Type.h>
#include <iostream>
#include <memory>
#include <vector>
#include "toco_parser.h"

namespace toco {

namespace internal {

llvm::LLVMContext &GetGlobalContext() {
    static llvm::LLVMContext context;
    return context;
}

static llvm::Type *typeOf(const Identifier &type) {
    if (type.name.compare("int") == 0) {
        return llvm::Type::getInt64Ty(GetGlobalContext());
    } else if (type.name.compare("double") == 0) {
        return llvm::Type::getDoubleTy(GetGlobalContext());
    }
    return llvm::Type::getVoidTy(GetGlobalContext());
}
}

LLVMCodeGenContext::LLVMCodeGenContext() {
    module = new llvm::Module("main", internal::GetGlobalContext());
}
void LLVMCodeGenContext::generateCode(Block &root) {
    std::cout << "Generating code" << std::endl;

    // Create the top level interpreter function to call as entry
    std::vector<llvm::Type *> arg_types;
    llvm::FunctionType *ftype = llvm::FunctionType::get(
        llvm::Type::getVoidTy(internal::GetGlobalContext()),
        llvm::makeArrayRef(arg_types), false);
    main_function_ = llvm::Function::Create(
        ftype, llvm::GlobalValue::InternalLinkage, "main", module);
    llvm::BasicBlock *bblock = llvm::BasicBlock::Create(
        internal::GetGlobalContext(), "entry", main_function_, 0);

    // Push new block/variable context
    pushBlock(bblock);
    root.generateCode(*this); // emit bytecode for the toplevel block
    llvm::ReturnInst::Create(internal::GetGlobalContext(), bblock);
    popBlock();

    // Print bytecode to see, if our program compiled properly
    std::cout << "Generated code" << std::endl;
    llvm::PassManager<llvm::Module> pass_manager;
    pass_manager.addPass(llvm::PrintModulePass(llvm::outs()));
    llvm::AnalysisManager<llvm::Module> am;
    pass_manager.run(*module, am);
}

llvm::GenericValue LLVMCodeGenContext::runCode() {
    std::cout << "Running code" << std::endl;
    llvm::ExecutionEngine *engine =
        llvm::EngineBuilder(std::unique_ptr<llvm::Module>(module)).create();
    engine->finalizeObject();
    std::vector<llvm::GenericValue> no_args;
    llvm::GenericValue v = engine->runFunction(main_function_, no_args);
    std::cout << "Code was run" << std::endl;
    return v;
}

std::map<std::string, llvm::Value *> &LLVMCodeGenContext::getLocals() {
    return blocks_.top()->locals;
}

llvm::BasicBlock *LLVMCodeGenContext::getCurrentBlock() {
    return blocks_.top()->block;
}

void LLVMCodeGenContext::pushBlock(llvm::BasicBlock *block) {
    blocks_.push(new LLVMCodeGenBlock());
    blocks_.top()->returnValue = nullptr;
    blocks_.top()->block = block;
}

void LLVMCodeGenContext::popBlock() {
    auto *top = blocks_.top();
    blocks_.pop();
    delete top;
}

void LLVMCodeGenContext::setCurrentReturnValue(llvm::Value *value) {
    blocks_.top()->returnValue = value;
}

llvm::Value *LLVMCodeGenContext::getCurrentReturnValue() {
    return blocks_.top()->returnValue;
}

// Code generation //
llvm::Value *Identifier::generateCode(LLVMCodeGenContext &context) {
    std::cout << "Creating identifier reference " << name << std::endl;
    if (context.getLocals().find(name) == context.getLocals().end()) {
        std::cerr << "Undeclared variable " << name << std::endl;
        return nullptr;
    }
    return new llvm::LoadInst(context.getLocals()[name], "", false,
                              context.getCurrentBlock());
}

llvm::Value *Assignment::generateCode(LLVMCodeGenContext &context) {
    std::cout << "Creating assignment for " << lhs.name << std::endl;
    if (context.getLocals().find(lhs.name) == context.getLocals().end()) {
        std::cerr << "Undeclared variable " << lhs.name << std::endl;
        return nullptr;
    }
    return new llvm::StoreInst(rhs.generateCode(context),
                               context.getLocals()[lhs.name], false,
                               context.getCurrentBlock());
}

llvm::Value *MethodCall::generateCode(LLVMCodeGenContext &context) {
    llvm::Function *function = context.module->getFunction(id.name.c_str());
    if (function == nullptr) {
        std::cerr << "No such function " << id.name << std::endl;
    }
    std::vector<llvm::Value *> args;
    Expressions::const_iterator it;
    for (it = arguments.begin(); it != arguments.end(); it++) {
        args.push_back((**it).generateCode(context));
    }
    llvm::CallInst *call = llvm::CallInst::Create(
        function, llvm::makeArrayRef(args), "", context.getCurrentBlock());
    std::cout << "Creating method call " << id.name << std::endl;
    return call;
}

llvm::Value *BinaryOperator::generateCode(LLVMCodeGenContext &context) {
    std::cout << "Creating binary operation " << op << std::endl;
    llvm::Instruction::BinaryOps instr;
    switch (op) {
        case TOK_PLUS:
            instr = llvm::Instruction::Add;
            goto math;
        case TOK_MINUS:
            instr = llvm::Instruction::Sub;
            goto math;
        case TOK_MUL:
            instr = llvm::Instruction::Mul;
            goto math;
        case TOK_DIV:
            instr = llvm::Instruction::SDiv;
            goto math;
    }

    return nullptr;

math:
    return llvm::BinaryOperator::Create(instr, lhs.generateCode(context),
                                        rhs.generateCode(context), "",
                                        context.getCurrentBlock());
}

llvm::Value *ExpressionStatement::generateCode(LLVMCodeGenContext &context) {
    std::cout << "Generating code for " << typeid(expression).name()
              << std::endl;
    return expression.generateCode(context);
}

llvm::Value *FunctionDeclaration::generateCode(LLVMCodeGenContext &context) {
    std::vector<llvm::Type *> arg_types;
    Variables::const_iterator it;
    for (it = arguments.begin(); it != arguments.end(); it++) {
        arg_types.push_back(internal::typeOf((**it).type));
    }
    llvm::FunctionType *ftype = llvm::FunctionType::get(
        internal::typeOf(type), llvm::makeArrayRef(arg_types), false);
    llvm::Function *function =
        llvm::Function::Create(ftype, llvm::GlobalValue::InternalLinkage,
                               id.name.c_str(), context.module);
    llvm::BasicBlock *bblock = llvm::BasicBlock::Create(
        internal::GetGlobalContext(), "entry", function, 0);
    context.pushBlock(bblock);

    llvm::Function::arg_iterator args_values = function->arg_begin();
    llvm::Value *argument_value;
    for (it = arguments.begin(); it != arguments.end(); it++) {
        (**it).generateCode(context);
        argument_value = &*args_values++;
        argument_value->setName((*it)->id.name.c_str());
        llvm::StoreInst *inst = new llvm::StoreInst(
            argument_value, context.getLocals()[(*it)->id.name], false, bblock);
    }

    block.generateCode(context);
    llvm::ReturnInst::Create(internal::GetGlobalContext(),
                             context.getCurrentReturnValue(), bblock);
    context.popBlock();
    std::cout << "Creating function " << id.name << std::endl;
    return function;
}

llvm::Value *VariableDeclaration::generateCode(LLVMCodeGenContext &context) {
    std::cout << "Creating variable declaration " << type.name << " " << id.name
              << std::endl;
    llvm::AllocaInst *alloc = new llvm::AllocaInst(
        internal::typeOf(type), id.name.c_str(), context.getCurrentBlock());
    context.getLocals()[id.name] = alloc;
    if (assignmentExpression != nullptr) {
        Assignment assign(id, *assignmentExpression);
        assign.generateCode(context);
    }
    return alloc;
}

llvm::Value *Block::generateCode(LLVMCodeGenContext &context) {
    Statements::const_iterator it;
    llvm::Value *last = nullptr;
    for (it = statements.begin(); it != statements.end(); it++) {
        std::cout << "Generating code for " << typeid(**it).name() << std::endl;
        last = (**it).generateCode(context);
    }
    std::cout << "Creating block" << std::endl;
    return last;
}

llvm::Value *Double::generateCode(LLVMCodeGenContext &context) {
    std::cout << "Creating double " << value << std::endl;
    return llvm::ConstantFP::get(
        llvm::Type::getDoubleTy(internal::GetGlobalContext()), value);
}

llvm::Value *Integer::generateCode(LLVMCodeGenContext &context) {
    std::cout << "Creating integer " << value << std::endl;
    return llvm::ConstantInt::get(
        llvm::Type::getInt64Ty(internal::GetGlobalContext()), value, true);
}
}
