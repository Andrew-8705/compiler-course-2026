#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;

namespace {

class MutationFinder : public RecursiveASTVisitor<MutationFinder> {
public:
  explicit MutationFinder(const VarDecl *Target) 
      : m_target(Target), m_isPointerMutated(false), m_isDataMutated(false) {}

  void Analyze(Stmt *Body) {
    if (!Body) return;
    m_isPointerMutated = false;
    m_isDataMutated = false;
    TraverseStmt(Body); 
  }

  bool isPointerMutated() const { return m_isPointerMutated; }
  bool isDataMutated() const { return m_isDataMutated; }

  bool VisitBinaryOperator(BinaryOperator *BO) {
    if (BO->isAssignmentOp()) {
      checkExpression(BO->getLHS(), false);
    }
    return true;
  }

  bool VisitUnaryOperator(UnaryOperator *UO) {
    if (UO->isIncrementDecrementOp()) {
      checkExpression(UO->getSubExpr(), false);
    }
    return true;
  }

private:
  const VarDecl *m_target;
  bool m_isPointerMutated;
  bool m_isDataMutated;

  void checkExpression(Expr *E, bool isDeref) {
    if (!E) return;
    E = E->IgnoreParenCasts();

    if (auto *DRE = dyn_cast<DeclRefExpr>(E)) {
      if (DRE->getDecl() == m_target) {
        if (isDeref) m_isDataMutated = true;
        else m_isPointerMutated = true;
      }
      return;
    }

    if (auto *UO = dyn_cast<UnaryOperator>(E)) {
      if (UO->getOpcode() == UO_Deref) {
        checkExpression(UO->getSubExpr(), true);
      }
      return;
    }

    if (auto *ASE = dyn_cast<ArraySubscriptExpr>(E)) {
        checkExpression(ASE->getBase(), true);
        return;
    }

    if (auto *BO = dyn_cast<BinaryOperator>(E)) {
        checkExpression(BO->getLHS(), isDeref);
        checkExpression(BO->getRHS(), isDeref);
        return;
    }
  }
};

class ConstFixVisitor final : public RecursiveASTVisitor<ConstFixVisitor> {
public:
  explicit ConstFixVisitor(ASTContext *context, Rewriter &R) 
      : m_context(context), m_rewriter(R) {}

  bool VisitVarDecl(VarDecl *v) {
    QualType qt = v->getType();

    bool isPtr = qt->isPointerType();
    bool isRef = qt->isReferenceType();

    if (!isPtr && !isRef) return true;

    auto *func = dyn_cast<FunctionDecl>(v->getDeclContext());
    if (!func || !func->hasBody()) return true;

    MutationFinder finder(v);
    finder.Analyze(func->getBody());

    bool ptrMutated = finder.isPointerMutated();
    bool dataMutated = finder.isDataMutated();

    bool dataIsConst = qt->getPointeeType().isConstQualified();
    bool ptrIsConst = qt.isLocalConstQualified(); 

    if (isRef) {
        if (!dataMutated && !qt.getNonReferenceType().isConstQualified()) {
            m_rewriter.InsertText(v->getBeginLoc(), "const ");
        }
    } 
    else if (isPtr) {
        if (!ptrMutated && !dataMutated) {
            if (!dataIsConst) m_rewriter.InsertText(v->getBeginLoc(), "const ");
            if (!ptrIsConst) m_rewriter.InsertText(v->getLocation(), "const ");
        } 
        else if (!dataMutated && ptrMutated) {
            if (!dataIsConst) m_rewriter.InsertText(v->getBeginLoc(), "const ");
        } 
        else if (dataMutated && !ptrMutated) {
            if (!ptrIsConst) m_rewriter.InsertText(v->getLocation(), "const ");
        }
    }
    
    return true;
  }

private:
  ASTContext *m_context;
  Rewriter &m_rewriter;
};

class ConstFixConsumer final : public ASTConsumer {
public:
  explicit ConstFixConsumer(CompilerInstance &CI) {}

  void HandleTranslationUnit(ASTContext &context) override {
    
    Rewriter rewriter;
    rewriter.setSourceMgr(context.getSourceManager(), context.getLangOpts());

    ConstFixVisitor visitor(&context, rewriter);
    
    visitor.TraverseDecl(context.getTranslationUnitDecl());
    
    FileID mainFileID = context.getSourceManager().getMainFileID();
    const llvm::RewriteBuffer *RewriteBuf = rewriter.getRewriteBufferFor(mainFileID);

    if (RewriteBuf) {
        llvm::outs() << std::string(RewriteBuf->begin(), RewriteBuf->end());
    } else {
        bool invalid = false;
        StringRef buf = context.getSourceManager().getBufferData(mainFileID, &invalid);
        if (!invalid) {
            llvm::outs() << buf;
        }
    }
    
    llvm::outs().flush();
  }
};

class ConstFixaAction final : public PluginASTAction {
public:
  std::unique_ptr<ASTConsumer>
  CreateASTConsumer(CompilerInstance &ci, llvm::StringRef) override {
    return std::make_unique<ConstFixConsumer>(ci);
  }

  bool ParseArgs(const CompilerInstance &ci, const std::vector<std::string> &args) override {
    return true;
  }

};

} // namespace

static FrontendPluginRegistry::Add<ConstFixaAction>
    X("const_plugin", "Adds const to pointers/refs");