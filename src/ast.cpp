#include "ast.hpp"
#include <vector>

koopaIR ir;

void CompUnitAST::Dump() const {
  func_def->Dump();
  // cout << "ir.n:" << ir.n << endl;
}

void FuncDefAST::Dump() const {
  cout << "FuncDef called" << endl;
  ir.append("fun @");
  ir.append(ident.c_str());
  ir.append("(): ");
  func_type->Dump();
  ir.append(" {\n");
  block->Dump();
  ir.append("}");
}

void FuncTypeAST::Dump() const {
  cout << "FuncType called" << endl;
  ir.append(type.c_str());
}

void BlockAST::Dump() const {
  cout << "Block called" << endl;
  ir.append(entry.c_str());
  ir.append(":\n");

  int n = blockitemnode.size();
  for (int i = 0; i < n; ++i) {
    blockitemnode[i]->Dump();
  }
}

void BlockItemAST::Dump() const {
  cout << "BlockItem called" << endl;
  if (decl != nullptr)
    decl->Dump();
  else if (stmt != nullptr)
    stmt->Dump();
}

void StmtAST::Dump() const {
  cout << "Stmt called" << endl;
  ir.is_ret = true;
  exp->Dump();
  ir.append("  ret ");
  cout << "Returning ";
  if (ir.is_value_ret) {
    cout << "value: ";
    ir.append(ir.valueStack.back());
    cout << ir.valueStack.back() << endl;
  } else if (ir.is_ident_ret) {
    cout << "ident: ";
    int t = ir.search(ir.temp.name);
    ir.append(t);
    cout << t << endl;
  } else if (ir.n == 0) {
    cout << "imm: ";
    ir.append(ir.valueStack.back());
  }
  ir.append("\n");
}

void PrimaryExpAST::Dump() const {
  cout << "PrimaryExp called" << endl;
  if (exp != nullptr)
    exp->Dump();
  else if (lval != nullptr)
    lval->Dump();
  else
    number->Dump();
}

void ConstExpAST::Dump() const {
  cout << "ConstExp called" << endl;
  exp->Dump();
}

void ExpAST::Dump() const {
  cout << "Exp called" << endl;
  lorexp->Dump();
}

void AddAST::Dump() const {
  cout << "Add called" << endl;
  if (addexp == nullptr) {
    mulexp->Dump();
    return;
  }
  addexp->Dump();
  mulexp->Dump();

  // 先取出栈顶数字
  int _1 = ir.valueStack.back();
  ir.valueStack.pop_back();
  int _2 = ir.valueStack.back();
  ir.valueStack.pop_back();

  // 计算并存储结果，结果保存到寄存器和栈中
  int t;
  if (op == "add") {
    t = _2 + _1;
  } else if (op == "sub") {
    t = _2 - _1;
  }
  ir.valueStack.push_back(t);
  // ir.REG[ir.n] = {"", t};

  // 输出指令
  ir.ins(op, ir.search(_2), ir.search(_1));
}

void MulAST::Dump() const {
  cout << "Mul called" << endl;
  if (mulexp == nullptr) {
    unaryexp->Dump();
    return;
  }
  mulexp->Dump();
  unaryexp->Dump();

  int _1 = ir.valueStack.back();
  ir.valueStack.pop_back();
  int _2 = ir.valueStack.back();
  ir.valueStack.pop_back();

  // 计算并存储结果
  int t;
  if (op == "mul") {
    t = _2 * _1;
  } else if (op == "div") {
    t = _2 / _1;
  } else if (op == "mod") {
    t = _2 % _1;
  }
  ir.valueStack.push_back(t);
  // ir.REG[ir.n] = {"", t};
  ir.ins(op, ir.search(_2), ir.search(_1));
}

void RelExpAST::Dump() const {
  cout << "RelExp called" << endl;
  if (relexp == nullptr) {
    addexp->Dump();
    return;
  }

  relexp->Dump();
  addexp->Dump();

  int _1 = ir.valueStack.back();
  ir.valueStack.pop_back();
  int _2 = ir.valueStack.back();
  ir.valueStack.pop_back();

  // 计算并存储结果
  int t;
  if (op == "lt") {
    t = (_2 < _1) ? 1 : 0;
  } else if (op == "gt") {
    t = (_2 > _1) ? 1 : 0;
  } else if (op == "le") {
    t = (_2 <= _1) ? 1 : 0;
  } else if (op == "ge") {
    t = (_2 >= _1) ? 1 : 0;
  }
  ir.valueStack.push_back(t);
  // ir.REG[ir.n] = {"", t};
  ir.ins(op, ir.search(_2), ir.search(_1));
}

void EqExpAST::Dump() const {
  cout << "EqExp called" << endl;
  if (eqexp == nullptr) {
    relexp->Dump();
    return;
  }

  eqexp->Dump();
  relexp->Dump();

  int _1 = ir.valueStack.back();
  ir.valueStack.pop_back();
  int _2 = ir.valueStack.back();
  ir.valueStack.pop_back();

  int t;
  if (op == "eq") {
    t = (_1 == _2) ? 1 : 0;
  } else if (op == "ne") {
    t = (_1 != _2) ? 1 : 0;
  }
  ir.valueStack.push_back(t);
  // ir.REG[ir.n] = {"", t};

  ir.ins(op, ir.search(_2), ir.search(_1));
}

void LAndExpAST::Dump() const {
  cout << "LAndExp called" << endl;
  if (landexp == nullptr) {
    eqexp->Dump();
    return;
  }

  landexp->Dump();
  eqexp->Dump();

  int _1 = ir.valueStack.back();
  ir.valueStack.pop_back();
  int _2 = ir.valueStack.back();
  ir.valueStack.pop_back();

  // 首先要判断操作数是不是0
  ir.ins("ne", ir.search(_1), "0");
  ir.ins("ne", ir.search(_2), "0");
  _1 = (_1 != 0) ? 1 : 0;
  _2 = (_2 != 0) ? 1 : 0;

  // 再进行与操作
  int t = _1 && _2;
  ir.valueStack.push_back(t);
  // ir.REG[ir.n] = {"", t};
  ir.ins("and", ir.search(_2), ir.search(_1));
}

void LOrExpAST::Dump() const {
  cout << "LOrExp called" << endl;
  if (lorexp == nullptr) {
    landexp->Dump();
    return;
  }

  lorexp->Dump();
  landexp->Dump();

  int _1 = ir.valueStack.back();
  ir.valueStack.pop_back();
  int _2 = ir.valueStack.back();
  ir.valueStack.pop_back();

  // 首先要判断操作数是不是0
  ir.ins("ne", ir.search(_1), "0");
  ir.ins("ne", ir.search(_2), "0");
  _1 = (_1 != 0) ? 1 : 0;
  _2 = (_2 != 0) ? 1 : 0;

  // 再进行或操作
  int t = _1 || _2;
  ir.valueStack.push_back(t);
  // ir.REG[ir.n] = {"", t};
  ir.ins("or", ir.search(_2), ir.search(_1));
}

void UnaryExpAST::Dump() const {
  cout << "UnaryExp called" << endl;
  if (primaryexp != nullptr)
    primaryexp->Dump();
  else {
    // 一元运算需要先输出数字再弄运算
    unaryexp->Dump();
    unaryop->Dump();
  }
}

void UnaryOpAST::Dump() const {
  cout << "UnaryOp called" << endl;

  int _1 = ir.valueStack.back();
  ir.valueStack.pop_back();
  // cout << "_1: " << _1 << endl;

  switch (op) {
  case '+':
    // 进行计算并存储
    // cout << "Running +" << endl;
    ir.valueStack.push_back(_1);
    break;

  case '-':
    // cout << "Running -" << endl;
    // ir.ins("sub", "0", "%" + to_string(ir.n - 1));

    // 进行计算并存储
    ir.valueStack.push_back(-_1);
    // ir.REG[ir.n] = {"", -_1};

    ir.ins("sub", "0", ir.search(_1));
    break;

  case '!':
    // cout << "Running !" << endl;
    // ir.ins("eq", ir.search(_1), "0");
    // 进行计算并存储
    ir.valueStack.push_back(!_1);
    // ir.REG[ir.n] = {"", !_1};

    ir.ins("eq", ir.search(_1), "0");
    break;
  }
}

void DeclAST::Dump() const {
  cout << "Decl called" << endl;
  constdecl->Dump();
}

void ConstDeclAST::Dump() const {
  cout << "ConstDecl called" << endl;
  btype->Dump();
  int n = constdefnode.size();
  for (int i = 0; i < n; ++i) {
    constdefnode[i]->Dump();
  }
}

void BTypeAST::Dump() const {
  cout << "BType called" << endl;
  // 存放类型
  ir.temp.type = btype;
  cout << ir.temp.type << endl;
}

void ConstDefAST::Dump() const {
  cout << "ConstDef called" << endl;
  constinitval->Dump();

  // 往符号表中加入变量
  ir.temp.name = ident;
  ir.symbolTable.push_back(ir.temp);
}

void ConstInitValAST::Dump() const {
  cout << "ConstInitVal called" << endl;

  constexp->Dump();
  ir.temp.constVal = ir.valueStack.back();
  ir.valueStack.pop_back();
}

void LValAST::Dump() const {
  cout << "LVal called" << endl;
  if (ir.is_ret) {
    ir.is_ident_ret = true;
  }
  ir.temp.name = ident;
}

void NumberAST::Dump() const {
  cout << "Number called " << endl;
  // variable t{"", number};
  if (ir.is_ret) {
    ir.is_value_ret = true;
  }
  ir.valueStack.push_back(number);
}
