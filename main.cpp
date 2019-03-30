/*
(* 1 2 )
(print cquery-project-roots)
(defun insert-fori ()
(insert "for (int i = 0; i < N; i++) {}")
)
(insert-fori)
 */
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/IR/AssemblyAnnotationWriter.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <istream>
#include <llvm/Analysis/CFGPrinter.h>
#include <memory>
#include <string>
#include <system_error>
#include <unordered_map>
#include <vector>

using namespace llvm;

#define ASS(x)                                                                 \
  if (!(x)) {                                                                  \
    std::cout << "ASS@" << __LINE__ << " " << #x << "\n";                      \
    std::abort();                                                              \
  }

int main(int argc, char **argv) {
  InitializeNativeTarget();
  InitializeNativeTargetAsmPrinter();
  InitializeNativeTargetAsmParser();
  //  cl::ParseCommandLineOptions(argc, argv, "VLisp compiler\n");

  LLVMContext Context;

  std::error_code EC;

  auto &ctx = Context;
  auto *module = new Module("VLisp", ctx);
  std::unique_ptr<TargetMachine> TM(EngineBuilder().selectTarget());
  ASS(TM);
  module->setDataLayout(TM->createDataLayout());
  {
    auto *putsFunc =
        Function::Create(FunctionType::get(Type::getVoidTy(ctx),
                                           {Type::getInt8PtrTy(ctx)}, false),
                         Function::ExternalLinkage, "puts", module);

    auto *vlispFunc =
        Function::Create(FunctionType::get(Type::getVoidTy(ctx), {}, false),
                         Function::ExternalLinkage, "vlisp", module);
    std::unique_ptr<IRBuilder<>> builder(
        new IRBuilder<>(BasicBlock::Create(ctx, "vlistp", vlispFunc)));
    BasicBlock *BB = builder->GetInsertBlock();
    Constant *msg = ConstantDataArray::getString(ctx, "Static string!", true);
    GlobalVariable *msgGlob =
        new GlobalVariable(*module, msg->getType(), true,
                           GlobalValue::InternalLinkage, msg, "staticString");
    Constant *zero_32 = Constant::getNullValue(IntegerType::getInt32Ty(ctx));
    Constant *gep_params[] = {zero_32, zero_32};
    Constant *msgptr = ConstantExpr::getGetElementPtr(msgGlob->getValueType(),
                                                      msgGlob, gep_params);

    Value *puts_params[] = {msgptr};
    CallInst *putsCall = CallInst::Create(putsFunc, puts_params, "", BB);
    ReturnInst::Create(ctx, BB);
  }
  std::unique_ptr<Module> mod(module);
  // insert int main(int argc, char **argv) { vlisp(); return 0; }
  {
    Function *main_func =
        Function::Create(FunctionType::get(Type::getInt32Ty(ctx), {}, false),
                         Function::ExternalLinkage, "main", module);
    // {
    // Function::arg_iterator args = main_func->arg_begin();
    // Value *arg_0 = &*args++;
    // arg_0->setName("argc");
    // Value *arg_1 = &*args++;
    // arg_1->setName("argv");
    // }

    BasicBlock *bb = BasicBlock::Create(mod->getContext(), "main.0", main_func);

    {
      CallInst *brainf_call =
          CallInst::Create(mod->getFunction("vlisp"), "", bb);
      brainf_call->setTailCall(false);
    }
    ReturnInst::Create(mod->getContext(),
                       ConstantInt::get(mod->getContext(), APInt(32, 0)), bb);
  }

  // assert(!verifyModule(*mod, &llvm::errs()) &&
  // "llvm module verification failed");

  raw_fd_ostream out("out.ll", EC, sys::fs::F_None);

  mod->print(out, nullptr, true);
  llvm_shutdown();

  return 0;
}
