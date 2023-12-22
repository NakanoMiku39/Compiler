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

struct if_counter { // 记录if的嵌套深度等
  int max;
  vector<int> stack;
};

struct instack { // valueStack中的元素
  int val, reg;
};

struct variable { // 符号表中的元素
  string name, type;
  bool is_const;
  int depth;
  instack inner;
};

class koopaIR {
private:
  string IR;

public:
  int reg_len = 0, var_num = 0; // 临时寄存器长度，变量个数
  bool is_ret = false;
  vector<instack> valueStack;                  // 立即数栈
  vector<variable> _st;                        // 符号表
  vector<vector<variable>> symbolTableManager; // 全局符号表管理器
  if_counter ifManager = {0};                  // if嵌套管理
  vector<string> output;                       // 输出

  koopaIR() {}

  // 通过变量名查找变量
  vector<variable>::iterator search(string _ident) {
    vector<vector<variable>>::reverse_iterator i;
    // 遍历符号表栈
    for (i = symbolTableManager.rbegin(); i != symbolTableManager.rend(); i++) {
      // 从当前符号表中查找符号
      for (vector<variable>::iterator j = i->begin(); j != i->end(); j++) {
        if (j->name.find(_ident) == 0) {
          return j;
        }
      }
    }
    cout << "Failed to find variable through ident" << endl;
  }

  // 通过寄存器查找变量
  vector<variable>::iterator search(int reg) {
    vector<vector<variable>>::reverse_iterator i;
    for (i = symbolTableManager.rbegin(); i != symbolTableManager.rend(); i++) {
      for (vector<variable>::iterator j = i->begin(); j != i->end(); j++) {
        if (j->inner.reg == reg) {
          return j;
        }
      }
    }
    cout << "Failed to find variable through reg" << endl;
  }

  string symbolLabel(string label) { // 为新变量分配地址名
    var_num += 1;
    return label += "_" + to_string(var_num);
  }

  string blockLabel(string label) {
    return label += "_" + to_string(ifManager.stack.back());
  }

  instack get_instack() { // 返回valueStack顶部的元素并pop
    instack t = valueStack.back();
    valueStack.pop_back();
    return t;
  }

  // 指令操作
  void ins(string cmd, instack _1, instack _2) {
    string reg1, reg2, s;
    reg1 = _1.reg == -1 ? to_string(_1.val) : "%" + to_string(_1.reg);
    reg2 = _2.reg == -1 ? to_string(_2.val) : "%" + to_string(_2.reg);
    s = "  %" + to_string(reg_len) + " = " + cmd + " " + reg1 + ", " + reg2 +
        "\n";
    output.push_back(s);
    reg_len += 1;
  }

  void ret(instack _ret) {
    string s = "  ret ";
    if (_ret.reg == -1) {
      cout << "value" << endl;
      s += to_string(_ret.val) + "\n";
    } else {
      cout << "ident" << endl;
      s += "%" + to_string(_ret.reg) + "\n";
    }
    cout << _ret.val << endl;
    output.push_back(s);
  }

  // 从地址加载到寄存器
  void load(vector<variable>::iterator var) {
    string s;
    s = "  %" + to_string(reg_len) + " = load @" + var->name + "\n";
    output.push_back(s);
    var->inner.reg = reg_len;
    // var->depth = symbolTableManager.size();
    reg_len += 1;
  }

  void alloc(string var) {
    string s;
    s = "  @" + var + " = alloc i32\n";
    output.push_back(s);
  }

  // 把值存到地址
  void store(variable var) {
    string s;
    if (var.inner.reg == -1)
      s = "  store " + to_string(var.inner.val) + ", @" + var.name + "\n";
    else
      s = "  store %" + to_string(var.inner.reg) + ", @" + var.name + "\n";
    output.push_back(s);
  }

  void br(instack t, string block1, string block2) {
    string s;
    block1 = blockLabel(block1);
    block2 = blockLabel(block2);
    if (t.reg == -1)
      s = "  br " + to_string(t.val) + ", " + block1 + ", " + block2 + "\n";
    else
      s = "  br %" + to_string(t.reg) + ", " + block1 + ", " + block2 + "\n";
    output.push_back(s);
  }

  const char *show() {
    for (vector<string>::iterator i = output.begin(); i != output.end(); i++)
      IR += *i;
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
  unique_ptr<ExpAST> block;
  unique_ptr<StmtAST> stmt1;
  unique_ptr<StmtAST> stmt2;
  enum TAG {
    LVAL,
    EXP,
    EMPTY,
    BLOCK,
    RETURNEXP,
    RETURN,
    IF,
    ELSE,
    IFELSE,
    WHILE
  } tag;

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
  enum TAG { IDENT, INITVAL } tag;

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
