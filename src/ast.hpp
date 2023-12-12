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

struct instack {
  int val, reg;
};

struct variable {
  string name, type, addr;
  bool is_const;
  instack inner;
};

class koopaIR {
private:
  string IR;

public:
  int reg_len = 0, addr_len = 0;
  bool is_ret = false, is_value_ret = false,
       is_ident_ret = false;    // 记录是否返回
  variable _ret;                // 记录返回的变量
  vector<variable> symbolTable; // 符号表
  vector<instack> valueStack;   // 立即数栈
  variable addrs[100];          // 记录变量的地址
  int REG[100];

  koopaIR() {}

  void append(string str) { IR += str; }

  void append(int num) { IR += to_string(num); }

  vector<variable>::iterator search(string _ident) {
    vector<variable>::iterator i;

    for (i = symbolTable.begin(); i < symbolTable.end(); i++) {
      if (i->name == _ident) { // 通过变量名查找变量
        return i;
      }
    }
  }

  vector<variable>::iterator search(int reg) {
    vector<variable>::iterator i;
    for (i = symbolTable.begin(); i < symbolTable.end(); i++) {
      if (i->inner.reg == reg) { // 通过寄存器查找变量
        return i;
      }
    }
  }
  // 指令操作
  void ins(string cmd, instack _1, instack _2) {
    string reg1, reg2;
    reg1 = _1.reg == -1 ? to_string(_1.val) : "%" + to_string(_1.reg);
    reg2 = _2.reg == -1 ? to_string(_2.val) : "%" + to_string(_2.reg);
    IR += "  %" + to_string(reg_len) + " = " + cmd + " " + reg1 + ", " + reg2 +
          "\n";
    reg_len += 1;
  }

  // 从地址加载到寄存器
  void load(vector<variable>::iterator var) {

    IR += "  %" + to_string(reg_len) + " = load " + var->addr + "\n";
    var->inner.reg = reg_len;
    REG[reg_len] = var->inner.val;
    reg_len += 1;
  }

  void alloc() {
    addr_len += 1;
    IR += "  @" + to_string(addr_len) + " = alloc i32\n";
  }

  // 把立即数存到地址
  void store_num(int num, string addr) {
    IR += "  store " + to_string(num) + ", " + addr + "\n";
  }
  // 把寄存器存到地址
  void store_reg(int reg, string addr) {
    IR += "  store %" + to_string(reg) + ", " + addr + "\n";
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
class NumberAST;

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
class UnaryOpAST;

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
  unique_ptr<FuncDefAST> func_def;
  void Dump() const override;
};

// FuncDef 也是 BaseAST
// 函数声明
class FuncDefAST : public BaseAST {
public:
  unique_ptr<FuncTypeAST> func_type;
  string ident;
  unique_ptr<BlockAST> block;
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
  vector<unique_ptr<BlockItemAST>> blockitemnode;
  string entry = "\%entry";
  void Dump() const override;
};

class BlockItemAST : public BaseAST {
public:
  unique_ptr<DeclAST> decl;
  unique_ptr<StmtAST> stmt;
  enum TAG { DECL, STMT } tag;
  void Dump() const override;
};

class StmtAST : public BaseAST {
public:
  unique_ptr<LValAST> lval;
  unique_ptr<ExpAST> exp;
  enum TAG { LVAL, EXP } tag;

  void Dump() const override;
};

class PrimaryExpAST : public BaseAST {
public:
  unique_ptr<ExpAST> exp;
  unique_ptr<LValAST> lval;
  unique_ptr<NumberAST> number;
  enum TAG { EXP, LVAL, NUMBER } tag;

  instack ret_value();
  void Dump() const override;
};

// 一个表达式
class ConstExpAST : public BaseAST {
public:
  unique_ptr<ExpAST> exp;

  instack ret_value();
  void Dump() const override;
};

// 一个表达式
class ExpAST : public BaseAST {
public:
  unique_ptr<LOrExpAST> lorexp;

  instack ret_value();
  void Dump() const override;
};

class AddExpAST : public BaseAST {
public:
  unique_ptr<AddExpAST> addexp;
  unique_ptr<MulExpAST> mulexp;
  string op;
  enum TAG { ADDEXP, MULEXP } tag;

  instack ret_value();
  void Dump() const override;
};

class MulExpAST : public BaseAST {
public:
  unique_ptr<MulExpAST> mulexp;
  unique_ptr<UnaryExpAST> unaryexp;
  string op;
  enum TAG { MULEXP, UNARYEXP } tag;

  instack ret_value();
  void Dump() const override;
};

class RelExpAST : public BaseAST {
public:
  unique_ptr<AddExpAST> addexp;
  unique_ptr<RelExpAST> relexp;
  string op;
  enum TAG { ADDEXP, RELEXP } tag;

  instack ret_value();
  void Dump() const override;
};

class EqExpAST : public BaseAST {
public:
  unique_ptr<RelExpAST> relexp;
  unique_ptr<EqExpAST> eqexp;
  string op;
  enum TAG { RELEXP, EQEXP } tag;

  instack ret_value();
  void Dump() const override;
};

class LAndExpAST : public BaseAST {
public:
  unique_ptr<EqExpAST> eqexp;
  unique_ptr<LAndExpAST> landexp;
  enum TAG { EQEXP, LANDEXP } tag;

  instack ret_value();
  void Dump() const override;
};

class LOrExpAST : public BaseAST {
public:
  unique_ptr<LOrExpAST> lorexp;
  unique_ptr<LAndExpAST> landexp;
  enum TAG { LOREXP, LANDEXP } tag;

  instack ret_value();
  void Dump() const override;
};

// 一元表达式
class UnaryExpAST : public BaseAST {
public:
  unique_ptr<PrimaryExpAST> primaryexp;
  unique_ptr<UnaryOpAST> unaryop;
  unique_ptr<UnaryExpAST> unaryexp;
  enum TAG { PRIMARYEXP, UNARYEXP } tag;

  instack ret_value();
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
  unique_ptr<ConstDeclAST> constdecl;
  unique_ptr<VarDeclAST> vardecl;
  enum TAG { CONSTDECL, VARDECL } tag;

  void Dump() const override;
};

class ConstDeclAST : public BaseAST {
public:
  unique_ptr<BTypeAST> btype;
  vector<unique_ptr<ConstDefAST>> constdefnode;

  void Dump() const override;
};

class ConstDefAST : public BaseAST {
public:
  string ident;
  unique_ptr<ConstInitValAST> constinitval;

  void Dump() const override;
};

class ConstInitValAST : public BaseAST {
public:
  unique_ptr<ConstExpAST> constexp;

  instack ret_value();
  void Dump() const override;
};

class VarDeclAST : public BaseAST {
public:
  unique_ptr<BTypeAST> btype;
  vector<unique_ptr<VarDefAST>> vardefnode;

  void Dump() const override;
};

class VarDefAST : public BaseAST {
public:
  string ident;
  unique_ptr<InitValAST> initval;

  void Dump() const override;
};

class InitValAST : public BaseAST {
public:
  unique_ptr<ExpAST> exp;

  void Dump() const override;
};

class BTypeAST : public BaseAST {
public:
  string btype;

  string ret_btype();
  void Dump() const override;
};

class LValAST : public BaseAST {
public:
  string ident;

  instack ret_value();
  void Dump() const override;
};

// 一个整数
class NumberAST : public BaseAST {
public:
  int number;

  instack ret_value();
  void Dump() const override;
};
