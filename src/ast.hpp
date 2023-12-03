#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

using namespace std;

struct variable {
  string name, type;
  int intVal, constVal;
};

class koopaIR {
private:
  string IR;

public:
  int n = 0;
  bool is_ret = false, is_value_ret = false, is_ident_ret = false;
  vector<variable> symbolTable;
  variable temp;
  vector<int> valueStack;
  variable REG[100];

  koopaIR() {}

  void append(string str) { IR += str; }

  void append(int num) { IR += to_string(num); }

  // 查找数字所在的寄存器
  string search(int _num) {
    if (1)
      return "";
    for (int i = 0; i < n; i++)
      if (REG[i].constVal == _num)
        return "%" + to_string(i); // 如果找到了就返回数字所在的寄存器
    // REG[n].value = _num;
    // n += 1;
    return to_string(_num); // 否则直接返回数字
  }

  // 通过变量名查找变量值
  int search(string _ident) {
    for (vector<variable>::iterator i = symbolTable.begin();
         i < symbolTable.end(); i++) {
      if (i->name == _ident) {
        return i->constVal;
      }
    }
    return -1;
  }

  // 指令操作
  void ins(string cmd, string reg2, string reg3) {
    if (1)
      return;
    IR += "  %" + to_string(n) + " = " + cmd + " " + reg2 + ", " + reg3 + "\n";
    n += 1;
  }

  const char *show() {
    cout << IR << endl;
    return IR.c_str();
  }
};

// 所有类的声明
class BaseAST;
class CompUnitAST;
class FuncDefAST;
class FuncFParamsAST;
class FuncFParamAST;
class FuncTypeAST;

class BlockAST;
class BlockItemAST;

class DeclAST;
class StmtAST; // Stmt

// Decl
class ConstDeclAST;
class VarDeclAST;
class BTypeAST;
class ConstDefAST;
class VarDefAST;
class InitValAST;
class ConstInitValAST;
class LValAST;
class ConstExpAST;
class ArrayIndexConstExpList;
class ArrayIndexExpList;

// Expression
class ExpAST;
class PrimaryExpAST;
class UnaryExpAST;
class MulExpAST;
class AddExpAST;
class RelExpAST;
class EqExpAST;
class LAndExpAST;
class LOrExpAST;

class FuncRParamsAST;

// 所有 AST 的基类
class BaseAST {
public:
  virtual void Dump() const = 0;

  virtual ~BaseAST() = default;
};

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {
public:
  // 用智能指针管理对象
  unique_ptr<BaseAST> func_def;
  void Dump() const override;
};

// FuncDef 也是 BaseAST
// 函数声明
class FuncDefAST : public BaseAST {
public:
  unique_ptr<BaseAST> func_type;
  string ident;
  unique_ptr<BaseAST> block;
  void Dump() const override;
};

// 函数类型
class FuncTypeAST : public BaseAST {
public:
  string type;
  void Dump() const override;
};

// 基本块
class BlockAST : public BaseAST {
public:
  vector<unique_ptr<BaseAST>> blockitemnode;
  string entry = "\%entry";
  void Dump() const override;
};

class BlockItemAST : public BaseAST {
public:
  unique_ptr<BaseAST> decl;
  unique_ptr<BaseAST> stmt;
  void Dump() const override;
};

class StmtAST : public BaseAST {
public:
  unique_ptr<BaseAST> exp;

  void Dump() const override;
};

class PrimaryExpAST : public BaseAST {
public:
  unique_ptr<BaseAST> exp;
  unique_ptr<BaseAST> lval;
  unique_ptr<BaseAST> number;

  void Dump() const override;
};

// 一个表达式
class ConstExpAST : public BaseAST {
public:
  unique_ptr<BaseAST> exp;

  void Dump() const override;
};

// 一个表达式
class ExpAST : public BaseAST {
public:
  unique_ptr<BaseAST> lorexp;

  void Dump() const override;
};
class AddAST : public BaseAST {
public:
  unique_ptr<BaseAST> addexp;
  unique_ptr<BaseAST> mulexp;
  string op;

  void Dump() const override;
};

class MulAST : public BaseAST {
public:
  unique_ptr<BaseAST> mulexp;
  unique_ptr<BaseAST> unaryexp;
  string op;

  void Dump() const override;
};

class RelExpAST : public BaseAST {
public:
  unique_ptr<BaseAST> addexp;
  unique_ptr<BaseAST> relexp;
  string op;

  void Dump() const override;
};

class EqExpAST : public BaseAST {
public:
  unique_ptr<BaseAST> relexp;
  unique_ptr<BaseAST> eqexp;
  string op;

  void Dump() const override;
};

class LAndExpAST : public BaseAST {
public:
  unique_ptr<BaseAST> eqexp;
  unique_ptr<BaseAST> landexp;

  void Dump() const override;
};

class LOrExpAST : public BaseAST {
public:
  unique_ptr<BaseAST> lorexp;
  unique_ptr<BaseAST> landexp;

  void Dump() const override;
};

// 一元表达式
class UnaryExpAST : public BaseAST {
public:
  unique_ptr<BaseAST> primaryexp;
  unique_ptr<BaseAST> unaryop;
  unique_ptr<BaseAST> unaryexp;

  void Dump() const override;
};

// 一元运算符
class UnaryOpAST : public BaseAST {
public:
  char op;

  void Dump() const override;
};

class DeclAST : public BaseAST {
public:
  unique_ptr<BaseAST> constdecl;

  void Dump() const override;
};

class ConstDeclAST : public BaseAST {
public:
  unique_ptr<BaseAST> btype;
  vector<unique_ptr<BaseAST>> constdefnode;

  void Dump() const override;
};

class BTypeAST : public BaseAST {
public:
  string btype;

  void Dump() const override;
};

class ConstDefAST : public BaseAST {
public:
  string ident;
  unique_ptr<BaseAST> constinitval;

  string get_ident();
  void Dump() const override;
};

class ConstInitValAST : public BaseAST {
public:
  unique_ptr<BaseAST> constexp;

  void Dump() const override;
};

class LValAST : public BaseAST {
public:
  string ident;

  void Dump() const override;
  // void addVal(int )
};

// 一个整数
class NumberAST : public BaseAST {
public:
  int number;

  void Dump() const override;
};
