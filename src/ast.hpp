
#include "symbolTable.hpp"

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
  unique_ptr<vector<unique_ptr<BaseAST>>> compunitnode;
  void Dump() const override;
};

class CompUnitItemAST : public BaseAST {
public:
  // 用智能指针管理对象
  unique_ptr<FuncDefAST> funcdef;
  void Dump() const override;
};

// FuncDef 也是 BaseAST
// 函数声明
class FuncDefAST : public BaseAST {
public:
  unique_ptr<FuncTypeAST> functype;
  unique_ptr<FuncFParamsAST> funcfparams;
  string ident;
  unique_ptr<BlockAST> block;
  enum { FUNCFPARAMS, NOFUNCFPARAMS } tag;
  void Dump() const override;
};

// 函数类型
class FuncTypeAST : public BaseAST {
public:
  string type;
  void Dump() const override;
};

class FuncFParamsAST : public BaseAST {
public:
  vector<unique_ptr<FuncFParamAST>> funcfparamnode;
  void Dump() const override;
};

class FuncFParamAST : public BaseAST {
public:
  unique_ptr<BTypeAST> btype;
  string ident;
  void Dump() const override;
};

class FuncRParamsAST : public BaseAST {
public:
  vector<unique_ptr<ExpAST>> exp;
  // instack ret_value();
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
    WHILE,
    BREAK,
    CONTINUE
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
  unique_ptr<FuncRParamsAST> funcrparams;
  string ident;
  enum TAG { PRIMARYEXP, UNARYEXP, NOFUNCRPARAMS, FUNCRPARAMS } tag;

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
  string ret_ident;
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
