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

#define IS_FPARAM true
#define NOT_FPARAM false

#define IS_CONST true
#define NOT_CONST false

#define IS_GLOBAL true
#define NOT_GLOBAL false

#define INITIALIZED true
#define UNINITIALIZED false

#define DEBUG 0
#define DEPRECATED 0

struct if_counter { // 记录if的嵌套深度等
  int max;
  vector<int> if_stack, while_stack;
};

struct instack { // valueStack中的元素
  int val, reg;
};

struct variable { // 符号表中的元素
  string name, type;
  bool _is_const, _is_fparam, _is_initialized, _is_global;
  instack inner;
};

// 所有类的声明
class BaseAST;
class CompUnitAST;
class CompUnitItemAST;
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

// -----------------------------------------------------------

class koopaIR {
private:
  string IR;

public:
  int reg_len = 0, var_num = 0; // 临时寄存器长度，变量个数
  // bool is_ret = false;
  vector<instack> valueStack;                  // 立即数栈
  vector<variable> _st;                        // 符号表
  vector<vector<variable>> symbolTableManager; // 符号表管理器
  vector<variable> globalTable;
  if_counter labelManager = {0}; // if-while嵌套管理
  vector<string> output;         // 输出

  koopaIR() {}

  // 通过变量名查找变量
  vector<variable>::iterator search(string _ident);

  // 通过寄存器查找变量
  vector<variable>::iterator search(int reg);

  vector<variable>::iterator search_global(string _ident);

  void new_symbol(string ident, string type, bool _is_const, bool _is_fparam,
                  bool _is_initialized);

  string symbolLabel(string label);

  string blockLabel(string label, int token);

  instack get_instack();
  // 指令操作
  void ins(string cmd, instack _1, instack _2);

  void ret(instack _ret);

  // 从地址加载到寄存器
  void load(vector<variable>::iterator var);

  void alloc(variable var);

  // 把值存到地址
  void store(variable var);

  // 用于给形参分配地址空间的store
  void store(string _ident);

  void br(instack t, string block1, string block2, int token);

  void call(string _ident, vector<unique_ptr<ExpAST>> *exp);

  void decl(string _ident, string type);

  const char *show();
};