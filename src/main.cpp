//-------------------------------------------------------------------------------------------------------
// Some useful examples:
//    http://www.hanshq.net/clang-plugin-example.html
//
// A vast list of matchers here:
//     http://clang.llvm.org/doxygen/ASTMatchers_8h_source.html
//-------------------------------------------------------------------------------------------------------


#include <string>
#include <fstream>
#include <streambuf>
#include <iostream>

#include <clang/AST/ASTConsumer.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchers.h>

// #include <clang/AST/DeclVisitor.h>

#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Sema/Sema.h>
#include <clang/Parse/ParseAST.h>


using namespace clang::ast_matchers;
using namespace clang::tooling;
using namespace clang;
using namespace llvm;
using namespace std;


//TODO: Do we really need this?
class Dump : public MatchFinder::MatchCallback {
public:
  virtual void run(const MatchFinder::MatchResult &Result) {
    ASTContext *Context = Result.Context;
    if (const CallExpr *E =
        Result.Nodes.getNodeAs<CallExpr>("functions")) {
      FullSourceLoc FullLocation = Context->getFullLoc(E->getLocStart());
      if (FullLocation.isValid()) {
        llvm::outs() << "Found call at " << FullLocation.getSpellingLineNumber()
                     << ":" << FullLocation.getSpellingColumnNumber() << "\n";
      }
    }
  }
};


class MyASTVisitor : public RecursiveASTVisitor<MyASTVisitor> {
private:
  ASTContext *_context;
public:
  explicit MyASTVisitor(ASTContext *ctx) : _context(ctx) {}

  bool VisitFunctionDecl(FunctionDecl *funcDecl) {
    FullSourceLoc location = _context->getFullLoc(funcDecl->getLocStart());
    if(location.isValid() && location.isFileID()) {
      string fname = funcDecl->getNameInfo().getName().getAsString();
      string rtype = funcDecl->getReturnType().getAsString();
      printf("    %s: %s\n", fname.c_str(), rtype.c_str()); //TODO: CompilerInstance.getSourceManager()->getFilename(location).c_str());
    }
    return true;
  }

  bool VisitParmVarDecl(ParmVarDecl *parmDecl) {
    string pname = parmDecl->getName();
    string qtype = QualType::getAsString(parmDecl->getType().split());
    printf("        %s: [%s]\n", pname.c_str(), qtype.c_str());
    return true;
  }
};


class MyASTConsumer : public ASTConsumer {
public:
  explicit MyASTConsumer(ASTContext *ctx) : _visitor(ctx) {}

  virtual bool HandleTopLevelDecl(DeclGroupRef DR) {
    for (DeclGroupRef::iterator b = DR.begin(), e = DR.end(); b != e; ++b) {
      _visitor.TraverseDecl(*b);
    }
    return true;
  }
private:
  MyASTVisitor _visitor;
};


class MyFrontendAction : public ASTFrontendAction {
public:
  virtual unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &compiler, llvm::StringRef InFile) {
    return unique_ptr<ASTConsumer>(new MyASTConsumer(&compiler.getASTContext()));
  }
};



class MyFrontendActionFactory : public FrontendActionFactory {
public:
  virtual FrontendAction *create () {
    return new MyFrontendAction;
  }
};


int main(int argc, char **argv) {
  /*
    if (argc < 2) {
    cerr << "Use " << argv[0] << " source_file -- [clang_args]\n";
    return 0;
    }

    // build compilation database
    tooling::FixedCompilationDatabase *comp =
    tooling::FixedCompilationDatabase::loadFromCommandLine(argc, const_cast<const char **> (argv));
    if (!comp) {
    cerr << "Failed to load compilation database\n";
    return 1;
    }

    // process files
    const string file(argv[1]);
    vector<string> sources;
    sources.push_back(file);
    tooling::ClangTool tool(*comp, sources);
    FrontendActionFactory act;
    return (tool.run(&act) != 0);
   */



  Dump Dump;
  MatchFinder Finder;

  StatementMatcher functionMatcher =
    callExpr(callee(functionDecl(hasName("doSomething")))).bind("functions");

  Finder.addMatcher(functionMatcher, &Dump);

  for (int i=1; i<argc; i++) {
    puts(argv[i]);
    ifstream t(argv[i]);
    string str((istreambuf_iterator<char>(t)), istreambuf_iterator<char>());
    tooling::runToolOnCode(new MyFrontendAction, StringRef(str));
  }
}





/*
  class ASTConsumer : public ASTConsumer {
  private:
  MyVisitor _visitor;
  public:
  explicit ASTConsumer(ASTContext *Context) : MyVisitor(Context) {}

  virtual bool HandleTopLevelDecl(DeclGroupRef D) {
  DeclGroupRef::iterator it(D.begin ());
  const DeclGroupRef::iterator itEnd(D.end ());
  while (it != itEnd) {
  Decl *decl(*it);
  if (!_visitor.TraverseDecl(decl)) {
  return false; 
  }
  ++it;
  }
  return true;
  }
  };
*/
