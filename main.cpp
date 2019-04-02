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
#include "llvm/IR/Constants.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstrTypes.h"
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

class MyModule {
private:
  LLVMContext ctx;
  Module *module;
  int strConstCnt = 0;
  std::unordered_map<std::string, GlobalVariable *> strConstAlloc;
  Function *PrintF;

public:
  MyModule() {
    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();
    InitializeNativeTargetAsmParser();

    module = new Module("VLisp", ctx);
    std::unique_ptr<TargetMachine> TM(EngineBuilder().selectTarget());
    ASS(TM);
    module->setDataLayout(TM->createDataLayout());
    PrintF =
        Function::Create(FunctionType::get(Type::getVoidTy(ctx),
                                           {Type::getInt8PtrTy(ctx)}, true),
                         Function::ExternalLinkage, "printf", module);
  }
  std::pair<Function *, BasicBlock *> createFunction(std::string const &name,
                                                     Type *RetTy,
                                                     ArrayRef<Type *> Params) {
    auto *Func =
        Function::Create(FunctionType::get(RetTy, Params, false),
                         Function::ExternalLinkage, name.c_str(), module);
    return {Func, BasicBlock::Create(ctx, "entry", Func)};
  }
  Value *createCall(BasicBlock *BB, std::string const &name,
                    ArrayRef<Value *> Params) {
    CallInst *call =
        CallInst::Create(module->getFunction(name.c_str()), Params, "", BB);
    call->setTailCall(false);
    return call;
  }
  Value *createString(char const *str) {
    Constant *msg = ConstantDataArray::getString(ctx, str, true);
    auto iter = strConstAlloc.find(str);
    int constId = -1;
    if (iter == strConstAlloc.end()) {
      constId = strConstCnt++;
      std::string name = "g_const_" + std::to_string(constId);
      GlobalVariable *msgGlob =
          new GlobalVariable(*module, msg->getType(), true,
                             GlobalValue::InternalLinkage, msg, name.c_str());
      strConstAlloc.emplace(str, msgGlob);
      iter = strConstAlloc.find(str);
    }
    ASS(iter != strConstAlloc.end());
    Constant *zero_32 = Constant::getNullValue(IntegerType::getInt32Ty(ctx));
    Constant *gep_params[] = {zero_32, zero_32};
    Constant *msgptr = ConstantExpr::getGetElementPtr(
        iter->second->getValueType(), iter->second, gep_params);
    return msgptr;
  }
  void printVal(BasicBlock *BB, const char *format, Value *P) {
    auto fmt = createString(format);
    Value *puts_params[] = {fmt, P};
    CallInst *putsCall = CallInst::Create(PrintF, puts_params, "", BB);
  }
  Value *createConst(int c) {
    ConstantInt *val_mem = ConstantInt::get(ctx, APInt(32, c));
    return val_mem;
  }
  Value *createAdd(BasicBlock *BB, Value *v1, Value *v2) {
    std::unique_ptr<IRBuilder<>> builder(new IRBuilder<>(BB));
    return builder->CreateAdd(v1, v2);
  }
  void returnInst(BasicBlock *BB, Value *v) { ReturnInst::Create(ctx, v, BB); }

  LLVMContext &getCtx() { return ctx; }

  void dump(char const *filename) {
    std::error_code EC;
    raw_fd_ostream out(filename, EC, sys::fs::F_None);

    module->print(out, nullptr, true);
  }
};

int main(int argc, char **argv) {

  MyModule mm;

  {
    auto FuncDesc = mm.createFunction("main", Type::getInt32Ty(mm.getCtx()),
                                      {Type::getInt32Ty(mm.getCtx())});
    auto *BB = FuncDesc.second;

    {
      auto tmpFunc = mm.createFunction("tmp", Type::getInt32Ty(mm.getCtx()),
                                       {Type::getInt32Ty(mm.getCtx())});
      auto *BB = tmpFunc.second;
      mm.printVal(BB, "%s\n", mm.createString("hello"));

      mm.printVal(
          BB, "%i\n",
          mm.createAdd(BB, mm.createConst(10), tmpFunc.first->args().begin()));
      {
        auto v = ConstantInt::get(mm.getCtx(), APInt(32, 0));
        mm.returnInst(BB, v);
      }
    }

    {
      mm.createCall(BB, "tmp", {ConstantInt::get(mm.getCtx(), APInt(32, 0))});
      auto v = ConstantInt::get(mm.getCtx(), APInt(32, 0));

      mm.returnInst(BB, v);
    }
  }

  mm.dump("out.ll");

  llvm_shutdown();

  return 0;
}
