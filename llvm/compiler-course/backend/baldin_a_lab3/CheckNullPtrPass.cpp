#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"

using namespace llvm;

namespace {
class NullCheckPass : public MachineFunctionPass {
public:
  static char ID;
  NullCheckPass() : MachineFunctionPass(ID) {}
  bool runOnMachineFunction(MachineFunction &MF) override;
};

char NullCheckPass::ID = 0;

bool NullCheckPass::runOnMachineFunction(MachineFunction &func) {
  bool changed = false;

  const TargetInstrInfo *TII = func.getSubtarget().getInstrInfo();

  for (MachineBasicBlock &MBB : func) {
    for (auto MI = MBB.begin(); MI != MBB.end(); ++MI) {
      MachineInstr &instr = *MI;

      if (instr.mayLoad() || instr.mayStore()) {
        Register baseReg = 0;
        for (unsigned i = 0; i < instr.getNumOperands(); ++i) {
          MachineOperand &MO = instr.getOperand(i);

          if (MO.isReg() && MO.isUse() && MO.getReg().isValid()) {
            if (MO.getReg() != X86::RSP && MO.getReg() != X86::RBP) {
              baseReg = MO.getReg();
              break;
            }
          }
        }

        if (!baseReg)
          continue;

        DebugLoc DL = instr.getDebugLoc();

        BuildMI(MBB, MI, DL, TII->get(TargetOpcode::COPY), X86::RDI)
            .addReg(baseReg);

        BuildMI(MBB, MI, DL, TII->get(X86::CALL64pcrel32))
            .addExternalSymbol("check_null");

        changed = true;
      }
    }
  }

  return changed;
}
} // namespace

static RegisterPass<NullCheckPass>
    X("null-check-x86", "Insert NULL checks before dereference", false, false);
