#include <iostream>
#include "llvm_codegen.h"
#include "nodes.h"

extern int yyparse();
extern toco::Block *program;

int main(int argc, char *argv[]) {

    yyparse();
    std::cout << "Address of program in memory: " << program << std::endl;
    toco::LLVMCodeGenContext context;
    context.generateCode(*program);
    context.runCode();

    return 0;
}