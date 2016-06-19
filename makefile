CXX := clang++
LLVMCOMPONENTS := cppbackend
RTTIFLAG := -fno-rtti
LLVMCONFIG := llvm-config

CXXFLAGS := -I$(shell $(LLVMCONFIG) --src-root)/tools/clang/include -I$(shell $(LLVMCONFIG) --obj-root)/tools/clang/include $(shell $(LLVMCONFIG) --cxxflags) $(RTTIFLAG)
LLVMLDFLAGS := $(shell $(LLVMCONFIG) --ldflags --libs $(LLVMCOMPONENTS))

SOURCES = src/main.cpp

OBJECTS = $(SOURCES:.cpp=.o)
EXES = $(OBJECTS:.o=)
CLANGLIBS = \
		-lclangTooling\
		-lclangFrontendTool\
		-lclangFrontend\
		-lclangDriver\
		-lclangSerialization\
		-lclangCodeGen\
		-lclangParse\
		-lclangSema\
		-lclangStaticAnalyzerFrontend\
		-lclangStaticAnalyzerCheckers\
		-lclangStaticAnalyzerCore\
		-lclangAnalysis\
		-lclangARCMigrate\
		-lclangRewrite\
		-lclangRewriteFrontend\
		-lclangEdit\
		-lclangAST\
                -lclangASTMatchers\
                -lclangDynamicASTMatchers\
		-lclangLex\
		-lclangBasic\
		$(shell $(LLVMCONFIG) --libs)\
		$(shell $(LLVMCONFIG) --system-libs)\
                -lcurses

all: $(OBJECTS) $(EXES)

%: %.o
	$(CXX) -o $@ $< $(CLANGLIBS) $(LLVMLDFLAGS)

clean:
	-rm -f $(EXES) $(OBJECTS) *~
