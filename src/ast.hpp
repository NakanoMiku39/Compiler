#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using namespace std;


class koopaIR {
private:
  string IR;
public:
  koopaIR() {}

  void append(string str) {
    IR += str;
  }

  void append(int num) {
    IR += to_string(num);
  }
/*
  void ret(string str) {
    IR +="  ret " + str + "\n";
  }
*/
  const char* show() {
    cout << IR << endl;
    return IR.c_str();
  }
};




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

  void Dump() const override;
  
};

// FuncDef 也是 BaseAST
class FuncDefAST: public BaseAST {
public:
  unique_ptr<BaseAST> func_type;
  string ident;
  unique_ptr<BaseAST> block;

  void Dump() const override;
};

class FuncTypeAST: public BaseAST {
public:
  string type;

  void Dump() const override;
};

class BlockAST: public BaseAST {
public:
  unique_ptr<BaseAST> stmt;
  string entry = "\%entry";

  void Dump() const override;
};

class StmtAST: public BaseAST {
public:
  unique_ptr<BaseAST> number;
  string ret;

  void Dump() const override;
};

class NumberAST: public BaseAST {
public:
  int number;

  void Dump() const override;
  
};
