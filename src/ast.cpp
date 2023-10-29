#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using namespace std;

// 所有 AST 的基类
class BaseAST {
  public:
  virtual void Dump(FILE *yyout) const = 0;

  virtual ~BaseAST() = default;
};

// CompUnit 是 BaseAST
class CompUnitAST: public BaseAST {
public:
  // 用智能指针管理对象
  unique_ptr<BaseAST> func_def;

  void Dump(FILE *yyout) const override {
    func_def->Dump(yyout);
  }
  
};

// FuncDef 也是 BaseAST
class FuncDefAST: public BaseAST {
public:
  unique_ptr<BaseAST> func_type;
  string ident;
  unique_ptr<BaseAST> block;

  void Dump(FILE *yyout) const override {
    fprintf(yyout, "fun @%s(): ", ident.c_str());
    func_type->Dump(yyout);
    block->Dump(yyout);
  }
};

class FuncTypeAST: public BaseAST {
public:
  string type;

  void Dump(FILE *yyout) const override {
    fprintf(yyout, "%s {\n", type.c_str());
  }
};

class BlockAST: public BaseAST {
public:
  unique_ptr<BaseAST> stmt;
  string entry = "\%entry";

  void Dump(FILE *yyout) const override {
    fprintf(yyout, "\%s:\n  ", entry.c_str());
    stmt->Dump(yyout);
  }
};

class StmtAST: public BaseAST {
public:
  unique_ptr<BaseAST> number;
  string ret;

  void Dump(FILE *yyout) const override {
    fprintf(yyout,"%s ", ret.c_str());
    number->Dump(yyout);
    fprintf(yyout, "}");
  }
};

class NumberAST: public BaseAST {
public:
  int number;

  void Dump(FILE *yyout) const override {
    fprintf(yyout, "%d\n", number) ;
  }
};
