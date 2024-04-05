#include "codegen.h"
#include <llvm/Analysis/CGSCCPassManager.h>
#include <llvm/Analysis/LoopAnalysisManager.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/PassManager.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/Transforms/IPO/ConstantMerge.h>
#include <llvm/Transforms/IPO/DeadArgumentElimination.h>
#include <llvm/Transforms/IPO/GlobalOpt.h>
#include <llvm/Transforms/IPO/SCCP.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/DCE.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/Transforms/Scalar/IndVarSimplify.h>
#include <llvm/Transforms/Scalar/LICM.h>
#include <llvm/Transforms/Scalar/Reassociate.h>
#include <llvm/Transforms/Scalar/SCCP.h>
#include <llvm/Transforms/Scalar/SimplifyCFG.h>
#include <llvm/Transforms/Scalar/TailRecursionElimination.h>
#include <llvm/Transforms/Utils/Mem2Reg.h>
#include <filesystem>
#include <memory>

IRCodegenVisitor::IRCodegenVisitor(std::string const& filename, std::shared_ptr<Logger>& logger) : m_logger(logger) {
  m_context = std::make_unique<llvm::LLVMContext>();
  m_module = std::make_unique<llvm::Module>(filename, *m_context);
  m_builder = std::make_unique<llvm::IRBuilder<>>(*m_context);

  buildRTS();
}

// Why is almost every resource about optimization passes using the legacy pass
// manager?
void IRCodegenVisitor::optimize() {
  llvm::ModulePassManager mpm;
  llvm::FunctionPassManager fpm;

  // Func passes
  fpm.addPass(llvm::PromotePass());       // mem2reg Utils/mem2reg
  fpm.addPass(llvm::InstCombinePass());   // peephole optimization. there's the
                                          // aggreisve one as well
  fpm.addPass(llvm::ReassociatePass());   // exprs reassociate
  fpm.addPass(llvm::GVNPass());           // common expr
  fpm.addPass(llvm::SCCPPass());          // constant propogation
  fpm.addPass(llvm::DCEPass());           // dead code elimination
  fpm.addPass(llvm::SimplifyCFGPass());   // Simplify CFG
  fpm.addPass(llvm::TailCallElimPass());  // Recursion
                                          //
  mpm.addPass(llvm::createModuleToFunctionPassAdaptor(
      std::move(fpm)));  // Move to module manager

  // IPO passes
  mpm.addPass(llvm::ConstantMergePass());  // Merge duplicates
  mpm.addPass(llvm::IPSCCPPass());         // Global constant propogation
  mpm.addPass(
      llvm::DeadArgumentEliminationPass());  // Remove unused args and returns
  mpm.addPass(llvm::GlobalOptPass());        // Remove unused globals

  llvm::PassBuilder pb;

  llvm::LoopAnalysisManager lam;
  llvm::FunctionAnalysisManager fam;
  llvm::CGSCCAnalysisManager cgam;
  llvm::ModuleAnalysisManager mam;

  pb.registerModuleAnalyses(mam);
  pb.registerCGSCCAnalyses(cgam);
  pb.registerFunctionAnalyses(fam);
  pb.registerLoopAnalyses(lam);
  pb.crossRegisterProxies(lam, fam, cgam, mam);

  mpm.run(*m_module, mam);
}

void IRCodegenVisitor::generateObj() {
  std::filesystem::path outfile = m_module->getSourceFileName();
  outfile.replace_extension(".o");

  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmParser();
  llvm::InitializeNativeTargetAsmPrinter();
  std::string const target_triple{llvm::sys::getDefaultTargetTriple()};

  std::string error;
  llvm::Target const* target =
      llvm::TargetRegistry::lookupTarget(target_triple, error);
  if (!error.empty()) {
    m_logger->error(error);
  }

  llvm::TargetOptions const opt;
  auto reloc_model = std::optional<llvm::Reloc::Model>();
  auto* target_machine = target->createTargetMachine(target_triple, "generic",
                                                     "", opt, reloc_model);

  m_module->setDataLayout(target_machine->createDataLayout());
  m_module->setTargetTriple(target_triple);

  std::error_code err;
  llvm::raw_fd_ostream output{outfile.string(), err};
  if (err) {
    m_logger->error(err.message());
  }

  llvm::legacy::PassManager pass;
  target_machine->addPassesToEmitFile(pass, output, nullptr,
                                      llvm::CodeGenFileType::CGFT_ObjectFile);
  pass.run(*m_module);
  output.flush();
}
