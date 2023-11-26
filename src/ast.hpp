#include <cassert>
#include <algorithm>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <vector>


using namespace std;

struct variable {
  string name;
  int value;
};

class koopaIR {
private:
  string IR;
    
public:
  int n = 0;
  vector<variable> stack;
  // vector<string> identStack;
  // vector<int> valueStack;
  variable REG[100];

  koopaIR() {}

  void append(string str) {
    IR += str;
  }

  void append(int num) {
    IR += to_string(num);
  }

  string search(int _num) { // 查找数字所在的寄存器
    for(int i = 0; i < n; i++) 
      if(REG[i].value == _num)
        return "%" + to_string(i);
    //REG[n].value = _num;
    //n += 1;
    return to_string(_num);
  }

  // 指令操作
  void ins(string cmd, int reg1, string reg2, string reg3) {
    IR += "  %" + to_string(reg1) + " = " + cmd 
      + " " + reg2 + ", " 
      + reg3 + "\n";
  }

/*
  void reg_num(string cmd, int reg1, int reg2, int num2) {
    IR += "  %" + to_string(reg1) + " = " + cmd 
      + " %" + to_string(reg2) + ", " 
      + to_string(num2) + "\n";
  }

  void num_reg(string cmd, int reg1, int num1, int reg2) {
    IR += "  %" + to_string(reg1) + " = " + cmd 
      + " " + to_string(num1) + ", %" 
      + to_string(reg2) + "\n";
  }

  void reg_reg(string cmd, int reg1, int reg2, int reg3) {
    IR += "  %" + to_string(reg1) + " = " + cmd 
      + " %" + to_string(reg2) + ", %" 
      + to_string(reg3) + "\n";
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
// 函数声明
class FuncDefAST: public BaseAST {
public:
  unique_ptr<BaseAST> func_type;
  string ident;
  unique_ptr<BaseAST> block;

  void Dump() const override;
};


// 函数类型
class FuncTypeAST: public BaseAST {
public:
  string type;

  void Dump() const override;
};


// 基本块
class BlockAST: public BaseAST {
public:
  vector<unique_ptr<BaseAST>> blockitemnode;
  string entry = "\%entry";

  void Dump() const override;
};


class BlockItemAST: public BaseAST {
public:
  unique_ptr<BaseAST> decl;
  unique_ptr<BaseAST> stmt;

  void Dump() const override;
};



class StmtAST: public BaseAST {
public:
  unique_ptr<BaseAST> exp;
  string ret;

  void Dump() const override;
};


class PrimaryExpAST: public BaseAST {
public:
  unique_ptr<BaseAST> exp;
  unique_ptr<BaseAST> lval;
  unique_ptr<BaseAST> number;

  void Dump() const override;
};


// 一个表达式
class ConstExpAST: public BaseAST {
public:
  unique_ptr<BaseAST> exp;
  
  void Dump() const override;
};


// 一个表达式
class ExpAST: public BaseAST {
public:
  unique_ptr<BaseAST> lorexp;
  
  void Dump() const override;
};


class AddAST: public BaseAST {
public:
  unique_ptr<BaseAST> addexp;
  unique_ptr<BaseAST> mulexp;
  char op;

  void Dump() const override;

};


class MulAST: public BaseAST {
public:
  unique_ptr<BaseAST> mulexp;
  unique_ptr<BaseAST> unaryexp;
  char op;
  
  void Dump() const override;

};


class RelExpAST: public BaseAST {
public:
  unique_ptr<BaseAST> addexp;
  unique_ptr<BaseAST> relexp;
  string op;

  void Dump() const override;

};


class EqExpAST: public BaseAST {
public:
  unique_ptr<BaseAST> relexp;
  unique_ptr<BaseAST> eqexp;
  string op;

  void Dump() const override;

};


class LAndExpAST: public BaseAST {
public:
  unique_ptr<BaseAST> eqexp;
  unique_ptr<BaseAST> landexp;
  // string _and = "&&";

  void Dump() const override;

};


class LOrExpAST: public BaseAST {
public:
  unique_ptr<BaseAST> lorexp;
  unique_ptr<BaseAST> landexp;
  // string _or = "||";
  
  void Dump() const override;

};


// 一元表达式
class UnaryExpAST: public BaseAST {
public:
  unique_ptr<BaseAST> primaryexp;
  unique_ptr<BaseAST> unaryop;
  unique_ptr<BaseAST> unaryexp;

  void Dump() const override;

};


// 一元运算符
class UnaryOpAST: public BaseAST {
public:
  char op;

  void Dump() const override;

};


class DeclAST: public BaseAST {
public:
  unique_ptr<BaseAST> constdecl;
  
  void Dump() const override;

};


class ConstDeclAST: public BaseAST {
public:
  unique_ptr<BaseAST> btype;
  vector<unique_ptr<BaseAST>> constdef;
  
  void Dump() const override;

};


class BTypeAST: public BaseAST {
public:
  string type;
  
  void Dump() const override;
};


class ConstDefAST: public BaseAST {
public:
  string ident;
  unique_ptr<BaseAST> constinitval;

  void Dump() const override;
};


class ConstInitValAST: public BaseAST {
public:
  unique_ptr<BaseAST> constexp;

  void Dump() const override;
  
};


class LValAST: public BaseAST {
public:
  string ident;

  void Dump() const override;
  //void addVal(int )
};


// 一个整数
class NumberAST: public BaseAST {
public:
  int number;

  void Dump() const override;
  
};
