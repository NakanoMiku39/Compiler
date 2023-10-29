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
  virtual void Dump() const = 0;

  virtual ~BaseAST() = default;
};

// CompUnit 是 BaseAST
class CompUnitAST: public BaseAST {
public:
  // 用智能指针管理对象
  unique_ptr<BaseAST> func_def;

  void Dump() const override {
    func_def->Dump();
  }
  
};

// FuncDef 也是 BaseAST
class FuncDefAST: public BaseAST {
public:
  unique_ptr<BaseAST> func_type;
  string ident;
  unique_ptr<BaseAST> block;

  void Dump() const override {
    cout << "fun ";
    cout << "@" << ident << "(): ";
    func_type->Dump();
    block->Dump();
  }
};

class FuncTypeAST: public BaseAST {
public:
  string type;

  void Dump() const override {
    cout << type << " {" << endl;
  }
};

class BlockAST: public BaseAST {
public:
  unique_ptr<BaseAST> stmt;

  void Dump() const override {
    cout << "%entry:" << endl << "  ";
    stmt->Dump();
  }
};

class StmtAST: public BaseAST {
public:
  unique_ptr<BaseAST> number;
  string ret;

  void Dump() const override {
    cout << ret << " ";
    number->Dump();
    cout << endl << "}";
  }
};

class NumberAST: public BaseAST {
public:
  int number;

  void Dump() const override {
  cout << number ;
  }
};
