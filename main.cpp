#include "Expression.h"

#include "Parser.h"

#include "Lexer.h"

#include <cassert>
#include <stdio.h>

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

  std::vector<Function *> fstack;
  std::vector<BasicBlock *> bbstack;

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
  void pushFun(Function *F) { fstack.push_back(F); }
  Value *getFirstArgument() { return fstack.back()->args().begin(); }
  void popFun() { fstack.pop_back(); }
  void pushBB(BasicBlock *BB) { bbstack.push_back(BB); }
  BasicBlock *popBB() {
    auto back = bbstack.back();
    bbstack.pop_back();
    return back;
  }
  std::pair<Function *, BasicBlock *> createFunction(std::string const &name,
                                                     Type *RetTy,
                                                     ArrayRef<Type *> Params) {
    auto *Func =
        Function::Create(FunctionType::get(RetTy, Params, false),
                         Function::ExternalLinkage, name.c_str(), module);
    return {Func, BasicBlock::Create(ctx, "entry", Func)};
  }
  Value *createCall(std::string const &name, ArrayRef<Value *> Params) {
    auto BB = bbstack.back();
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
  void printVal(const char *format, Value *P) {
    auto BB = bbstack.back();
    auto fmt = createString(format);
    Value *puts_params[] = {fmt, P};
    CallInst *putsCall = CallInst::Create(PrintF, puts_params, "", BB);
  }
  Value *createConst(int c) {
    auto BB = bbstack.back();
    std::unique_ptr<IRBuilder<>> builder(new IRBuilder<>(BB));
    return builder->CreateAdd(ConstantInt::get(ctx, APInt(32, c)),
                              ConstantInt::get(ctx, APInt(32, 0)));
  }
  Value *createAdd(Value *v1, Value *v2) {
    auto BB = bbstack.back();
    std::unique_ptr<IRBuilder<>> builder(new IRBuilder<>(BB));
    return builder->CreateAdd(v1, v2);
  }
  Value *createSub(Value *v1, Value *v2) {
    auto BB = bbstack.back();
    std::unique_ptr<IRBuilder<>> builder(new IRBuilder<>(BB));
    return builder->CreateSub(v1, v2);
  }
  Value *createDiv(Value *v1, Value *v2) {
    auto BB = bbstack.back();
    std::unique_ptr<IRBuilder<>> builder(new IRBuilder<>(BB));
    return builder->CreateSDiv(v1, v2);
  }
  Value *createMul(Value *v1, Value *v2) {
    auto BB = bbstack.back();
    std::unique_ptr<IRBuilder<>> builder(new IRBuilder<>(BB));
    return builder->CreateMul(v1, v2);
  }
  Value *createEqeq(Value *v1, Value *v2) {
    auto BB = bbstack.back();
    std::unique_ptr<IRBuilder<>> builder(new IRBuilder<>(BB));
    return builder->CreateICmpEQ(v1, v2);
  }
  Value *createLess(Value *v1, Value *v2) {
    auto BB = bbstack.back();
    std::unique_ptr<IRBuilder<>> builder(new IRBuilder<>(BB));
    return builder->CreateICmpSLT(v1, v2);
  }
  void createGoto(BasicBlock *toBB) {
    auto BB = bbstack.back();
    std::unique_ptr<IRBuilder<>> builder(new IRBuilder<>(BB));
    builder->CreateBr(toBB);
  }
  struct CondBranch {
    BasicBlock *then;
    BasicBlock *belse;
    BasicBlock *merge;
  };
  CondBranch createCond(Value *cond) {
    auto BB = bbstack.back();
    // bbstack.pop_back();
    std::unique_ptr<IRBuilder<>> builder(new IRBuilder<>(BB));
    auto CondExp = cond;
    // builder->CreateICmpNE(cond, ConstantInt::get(ctx, APInt(32, 0)));

    BasicBlock *ThenBB = BasicBlock::Create(ctx, "then");
    BasicBlock *ElseBB = BasicBlock::Create(ctx, "else");
    BasicBlock *MergeBB = BasicBlock::Create(ctx, "merge");

    ThenBB->insertInto(fstack.back());
    ElseBB->insertInto(fstack.back());
    MergeBB->insertInto(fstack.back());

    builder->CreateCondBr(CondExp, ThenBB, ElseBB);

    return {ThenBB, ElseBB, MergeBB};
  }
  Value *merge(BasicBlock *b1, Value *v1, BasicBlock *b2, Value *v2) {
    auto BB = bbstack.back();
    std::unique_ptr<IRBuilder<>> builder(new IRBuilder<>(BB));
    PHINode *PN = builder->CreatePHI(Type::getInt32Ty(ctx), 2);

    PN->addIncoming(v1, b1);
    PN->addIncoming(v2, b2);
    return PN;
  }
  void returnInst(Value *v) {
    auto BB = bbstack.back();
    ReturnInst::Create(ctx, v, BB);
  }

  LLVMContext &getCtx() { return ctx; }

  void dump(char const *filename) {
    std::error_code EC;
    raw_fd_ostream out(filename, EC, sys::fs::F_None);

    module->print(out, nullptr, true);
  }
};

SExpression *getAST(const char *expr) {
  SExpression *expression = NULL;
  yyscan_t scanner;
  YY_BUFFER_STATE state;

  if (yylex_init(&scanner)) {
    return NULL;
  }

  state = yy_scan_string(expr, scanner);

  if (yyparse(&expression, scanner)) {
    return NULL;
  }

  yy_delete_buffer(state, scanner);

  yylex_destroy(scanner);

  return expression;
}

struct Env {
  MyModule mm;
};

Value *evaluate(SExpression *e, Env &env) {
  if (e == NULL)
    return NULL;
  switch (e->type) {
  case eVALUE:
    return env.mm.createConst(e->value);
  case eSEM: {
    evaluate(e->left, env);
    evaluate(e->right, env);
    return NULL;
  }
  case eCOLON: {
    return env.mm.createMul(evaluate(e->left, env), evaluate(e->right, env));
  }
  case eEQEQ: {
    return env.mm.createEqeq(evaluate(e->left, env), evaluate(e->right, env));
  }
  case eLESS: {
    return env.mm.createLess(evaluate(e->left, env), evaluate(e->right, env));
  }
  case eMULTIPLY:
    return env.mm.createMul(evaluate(e->left, env), evaluate(e->right, env));
  case eMINUS: {
    if (e->left)
      return env.mm.createSub(evaluate(e->left, env), evaluate(e->right, env));
    else
      return env.mm.createMul(env.mm.createConst(-1), evaluate(e->right, env));
  }
  case eDIV:
    return env.mm.createDiv(evaluate(e->left, env), evaluate(e->right, env));
  case eADD:
    return env.mm.createAdd(evaluate(e->left, env), evaluate(e->right, env));
  case eCALL: {

    if (strcmp(e->name, "print") == 0) {
      auto *val = evaluate(e->left, env);
      env.mm.printVal("%i\n", val);
      return NULL;
    } else {
      auto *val = evaluate(e->left, env);
      return env.mm.createCall(e->name, {val});
    }
    return NULL;
  }
  case eIF: {
    auto *val = evaluate(e->cond, env);
    auto cond = env.mm.createCond(val);
    env.mm.popBB();
    env.mm.pushBB(cond.then);
    auto *ThenVal = evaluate(e->left, env);
    env.mm.createGoto(cond.merge);
    auto ThenBB = env.mm.popBB();
    env.mm.pushBB(cond.belse);
    auto *ElseVal = evaluate(e->right, env);
    env.mm.createGoto(cond.merge);
    auto ElseBB = env.mm.popBB();
    env.mm.pushBB(cond.merge);
    auto mergeVal = env.mm.merge(ThenBB, ThenVal, ElseBB, ElseVal);
    return mergeVal;
  }
  case eDEF: {
    ASS(false && "TODO");
    return NULL;
  }
  case eDEFUN: {
    auto &mm = env.mm;
    auto desc = mm.createFunction(e->name, Type::getInt32Ty(mm.getCtx()),
                                  {Type::getInt32Ty(mm.getCtx())});
    env.mm.pushFun(desc.first);
    env.mm.pushBB(desc.second);
    auto *val = evaluate(e->left, env);
    mm.returnInst(val);
    env.mm.popBB();
    env.mm.popFun();
    return NULL;
  }
  case eREF: {
    if (strcmp(e->name, "arg") == 0) {
      return env.mm.getFirstArgument();
    }
    ASS(false && "TODO");
    return NULL;
  }
  default:
    std::cout << "[ERROR] Unknown OP\n";
    abort();
    return 0;
  }
}

int main(int argc, char **argv) {
  std::ifstream file(argv[1]);
  std::vector<char> file_contents;
  {
    file.unsetf(std::ios::skipws);
    std::streampos fileSize;
    file.seekg(0, std::ios::end);
    fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    file_contents.reserve(fileSize);
    file_contents.insert(file_contents.begin(),
                         std::istream_iterator<char>(file),
                         std::istream_iterator<char>());
    file_contents.push_back('\0');
    // std::cout << "THE FILE :" << &file_contents[0] << "\n";
  }
  SExpression *e = getAST(&file_contents[0]);
  Env env;
  auto FuncDesc =
      env.mm.createFunction("main", Type::getInt32Ty(env.mm.getCtx()),
                            {Type::getInt32Ty(env.mm.getCtx())});
  env.mm.pushFun(FuncDesc.first);
  env.mm.pushBB(FuncDesc.second);
  evaluate(e, env);

  deleteExpression(e);
  auto v = ConstantInt::get(env.mm.getCtx(), APInt(32, 0));
  env.mm.returnInst(v);
  env.mm.dump("out.ll");

  llvm_shutdown();

  return 0;
}
