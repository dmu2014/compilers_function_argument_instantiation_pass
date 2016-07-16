//===- funcInst.cpp Submitted by - Dishank Upadhyay and Zijie Lin 
// We have used the standard Makefile and have not done any modifications to it.
// This pass is to be followed by a -deadargelim pass.
//The output human readable IR files after -deadargelimpass are also attached.
//- Example code from "Writing an LLVM Pass" ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the function Argument Instantiation Pass and should be  
// later followed by dead arg elimination to eliminate dead arguments at the 
// call site.
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "hello"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/Support/CallSite.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/IR/Constants.h"


using namespace llvm;

namespace {
struct Hello :  public FunctionPass
{

        /** Constructor. */
	static char ID;                           
	Hello() : FunctionPass(ID) {}
        virtual bool runOnFunction(llvm::Function &F) {
		for(Function::iterator bb = F.begin(), e = F.end(); bb != e; ++bb) {
			for(BasicBlock::iterator I = bb->begin(), e = bb->end(); I != e;++I) {
				if (I->getOpcode()==Instruction::Call){
					Instruction& Inst = *I;
					CallInst *callInst = dyn_cast<CallInst>(&Inst);
					Function *fun = callInst ->getCalledFunction();
						//Skip processing if you encounter a printf call. Alternately, we could have used if (!F->isDeclaration())
						if (fun->getName() == "printf"){
							continue;
						}
					unsigned int num_arguments = callInst-> getNumArgOperands ();
					for (unsigned int j=0;j < num_arguments; j++){
						Value *arg_operand = callInst-> getArgOperand (j);
						if (isa<Constant>(arg_operand)){
							ValueToValueMapTy VMap;
							const Twine &Name = "";
							//Clone function with an empty VMap
							Function *clo_Function = CloneFunction(fun, VMap, true);
							//Insert into the function List
							F.getParent()->getFunctionList().push_back(clo_Function);
							//Replace old function with new one
							callInst->setCalledFunction(clo_Function);
							//set Linkage of the cloned Function to Internal. This helps in eliminating dead arguments in the deadargelim pass
							clo_Function->setLinkage(GlobalValue::InternalLinkage);
							
							//Replace all uses of formal argument with a ConstInt value of the constant argument
							unsigned int j=0;
							for (Function::arg_iterator fI = clo_Function->arg_begin(), fE = clo_Function->arg_end(); fI != fE; ++fI) {
								if (isa<Constant>(callInst-> getArgOperand (j))){
									Value * value = (callInst-> getArgOperand (j));
									ConstantInt* CI = dyn_cast<ConstantInt>(value);	
									uint64_t intval= CI->getValue().getLimitedValue();	
									Value *newvalue = ConstantInt::get(value->getType(), intval, false);						
									fI -> replaceAllUsesWith(newvalue);
								}
								j=j+1;
							}
							break;
						}
					}
				}
			}
		}
		return true;
	}
}; //Close Struct
	} //Close Namespace
char Hello::ID = 0;
static RegisterPass<Hello> X("hello", "Hello World Pass", false, false);



