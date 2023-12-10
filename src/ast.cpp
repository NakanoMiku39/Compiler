#include "ast.hpp"
#include <string>
#include <vector>

koopaIR ir;

void CompUnitAST::Dump() const { func_def->Dump(); }

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
  if (tag == DECL)
    decl->Dump();
  else if (tag == STMT)
    stmt->Dump();
}

void StmtAST::Dump() const {
  cout << "Stmt called" << endl;
  if (tag == EXP) {
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
      vector<variable>::iterator t = ir.search(ir._ret.name);
      if(t->is_const)
        ir.append(t->intVal);
      else
        ir.append(t->reg);
    } 
    ir.append("\n");
  } else {
    // 赋值操作
    lval->Dump();
    
    exp->Dump();
    string _t = lval->ret_value(); // 获取变量名
    vector<variable>::iterator t = ir.search(_t); // 获取变量
    t->reg = "%" + to_string(ir.reg_len); // 更新新变量值所在的寄存器
    t->intVal = ir.valueStack.back(); // 栈顶数字（也就是要赋的值）给变量
    // ir.reg_len += 1;
    // 现在是赋值操作有问题，在完成加法运算后，需要对a进行复制
    // 但是调用完exp后会再把之前的变量值push到栈里进去
    
    ir.valueStack.pop_back();
    ir.store(t->reg, t->addr);
  }
}

void PrimaryExpAST::Dump() const {
  cout << "PrimaryExp called" << endl;
  if (tag == EXP)
    exp->Dump();
  else if (tag == LVAL)
    lval->Dump();
  else
    number->Dump();
}

// stringPrimaryExpAST::ret_ident() { return lval->ret_ident(); }

string PrimaryExpAST::ret_value() {
  switch (tag) {
  case EXP:
    return exp->ret_value();
  case LVAL:
    return lval->ret_value();
  case NUMBER:
    return number->ret_value();
  }
}

void ConstExpAST::Dump() const {
  cout << "ConstExp called" << endl;
  exp->Dump();
}

void ExpAST::Dump() const {
  cout << "Exp called" << endl;
  lorexp->Dump();
}

string ExpAST::ret_value() { return lorexp->ret_value(); }

void AddExpAST::Dump() const {
  cout << "Add called" << endl;
  if (tag == MULEXP) {
    mulexp->Dump();
    return;
  }
  addexp->Dump();
  string _1 = addexp->ret_value(); // 可能是寄存器也可能是立即数
  // int _t1 = _1[0] == '%' ? ir.REG[stoi(_1.erase(0, 1))] : stoi(_1);
  // ir.valueStack.push_back(_t1);
  mulexp->Dump();
  string _2 = mulexp->ret_value(); // 可能是寄存器也可能是立即数
  // int _t2 = _2[0] == '%' ? ir.REG[stoi(_2.erase(0, 1))] : stoi(_2);
  // ir.valueStack.push_back(_t2);
  // 计算并存储结果，结果保存到寄存器和栈中
  // 从栈中提取操作数
  int t, t1, t2;
  t1 = ir.valueStack.back();
  ir.valueStack.pop_back();
  t2 = ir.valueStack.back();
  ir.valueStack.pop_back();
  
  if (op == "add") {
    t = t2 + t1;
  } else {
    t = t2 - t1;
  }
  
  cout << t2 << " and " << t1 << " Add result: " << t << endl;
  ir.valueStack.push_back(t);


  // 输出指令
  if(_1[0] == '%' || _2[0] == '%') {
    ir.ins(op, _2, _1);
  }
  // ir.REG[ir.n1] = ir.valueStack.back();
  // ir.valueStack.pop_back();
}

string AddExpAST::ret_value() {
  switch (tag) {
  case MULEXP:
    return mulexp->ret_value();
  case ADDEXP:
    return addexp->ret_value();
  }
}

void MulExpAST::Dump() const {
  cout << "Mul called" << endl;
  if (tag == UNARYEXP) {
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
  // ir.ins(op, ir.search(_2), ir.search(_1));
}

// string MulExpAST::ret_ident() { return unaryexp->ret_ident(); }
string MulExpAST::ret_value() {
  switch (tag) {
  case UNARYEXP:
    return unaryexp->ret_value();
  case MULEXP:
    return mulexp->ret_value();
  }
}

void RelExpAST::Dump() const {
  cout << "RelExp called" << endl;
  if (tag == ADDEXP) {
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
  // ir.ins(op, ir.search(_2), ir.search(_1));
}

// string RelExpAST::ret_ident() { return addexp->ret_ident(); }

string RelExpAST::ret_value() {
  switch (tag) {
  case ADDEXP:
    return addexp->ret_value();
  case RELEXP:
    return relexp->ret_value();
  }
}

void EqExpAST::Dump() const {
  cout << "EqExp called" << endl;
  if (tag == RELEXP) {
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

  // ir.ins(op, ir.search(_2), ir.search(_1));
}

string EqExpAST::ret_value() {
  switch (tag) {
  case RELEXP:
    return relexp->ret_value();
  case EQEXP:
    return eqexp->ret_value();
  }
}

void LAndExpAST::Dump() const {
  cout << "LAndExp called" << endl;
  if (tag == EQEXP) {
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
  // ir.ins("ne", ir.search(_1), "0");
  // ir.ins("ne", ir.search(_2), "0");
  _1 = (_1 != 0) ? 1 : 0;
  _2 = (_2 != 0) ? 1 : 0;

  // 再进行与操作
  int t = _1 && _2;
  ir.valueStack.push_back(t);
  // ir.ins("and", ir.search(_2), ir.search(_1));
}

string LAndExpAST::ret_value() {
  switch (tag) {
  case EQEXP:
    return eqexp->ret_value();
  case LANDEXP:
    return landexp->ret_value();
  }
}

void LOrExpAST::Dump() const {
  cout << "LOrExp called" << endl;
  if (tag == LANDEXP) {
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
  // ir.ins("ne", ir.search(_1), "0");
  // ir.ins("ne", ir.search(_2), "0");
  _1 = (_1 != 0) ? 1 : 0;
  _2 = (_2 != 0) ? 1 : 0;

  // 再进行或操作
  int t = _1 || _2;
  ir.valueStack.push_back(t);
  // ir.ins("or", ir.search(_2), ir.search(_1));
}

string LOrExpAST::ret_value() {
  switch (tag) {
  case LANDEXP:
    return landexp->ret_value();
  case LOREXP:
    return lorexp->ret_value();
  }
}

void UnaryExpAST::Dump() const {
  cout << "UnaryExp called" << endl;
  if (tag == PRIMARYEXP)
    primaryexp->Dump();
  else {
    // 一元运算需要先输出数字再弄运算
    unaryexp->Dump();
    unaryop->Dump();
  }
}

string UnaryExpAST::ret_value() {
  switch (tag) {
  case PRIMARYEXP:
    return primaryexp->ret_value();
  case UNARYEXP:
    return unaryexp->ret_value();
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
    // // ir.ins("sub", "0", "%" + to_string(ir.n1 - 1));

    // 进行计算并存储
    ir.valueStack.push_back(-_1);

    // ir.ins("sub", "0", ir.search(_1));
    break;

  case '!':
    // cout << "Running !" << endl;
    // // ir.ins("eq", ir.search(_1), "0");
    // 进行计算并存储
    ir.valueStack.push_back(!_1);

    // ir.ins("eq", ir.search(_1), "0");
    break;
  }
}

void DeclAST::Dump() const {
  cout << "Decl called" << endl;
  if (constdecl != nullptr)
    constdecl->Dump();
  else
    vardecl->Dump();
}

void ConstDeclAST::Dump() const {
  cout << "ConstDecl called" << endl;
  btype->Dump();
  string type = btype->ret_btype();

  int n = constdefnode.size();
  for (int i = 0; i < n; ++i) {

    // 把常量放进符号表
    struct variable t;
    t.type = type;
    t.reg = "%-1";
    t.addr = "@-1";
    t.is_const = true;
    ir.symbolTable.push_back(t);
    constdefnode[i]->Dump();
  }
}

void ConstDefAST::Dump() const {
  cout << "ConstDef called" << endl;
  ir.symbolTable.back().name = ident;
  constinitval->Dump();
}

// string ConstDefAST::ret_ident() { return ident; }

void ConstInitValAST::Dump() const {
  cout << "ConstInitVal called" << endl;

  constexp->Dump();
  ir.symbolTable.back().intVal = ir.valueStack.back();
  ir.valueStack.pop_back();
}

// int ConstInitValAST::ret_value() { return ir.valueStack.back(); }

void VarDeclAST::Dump() const {
  cout << "VarDecl called" << endl;
  btype->Dump();
  string type = btype->ret_btype();

  int n = vardefnode.size();
  for (int i = 0; i < n; ++i) {

    // 把变量放进符号表
    struct variable t;
    t.type = type;
    t.reg = "%-1";
    t.addr = "@" + to_string(ir.addr_len + 1);
    t.is_const = false;
    ir.symbolTable.push_back(t);
    vardefnode[i]->Dump();

    // 分配内存
    ir.alloc();
    // 然后把变量存到地址上
    ir.store(ir.symbolTable.back().intVal, t.addr);
  }
}

void VarDefAST::Dump() const {
  cout << "VarDef called" << endl;

  ir.symbolTable.back().name = ident;
  initval->Dump();
  // 往符号表中加入变量
}

void InitValAST::Dump() const {
  cout << "InitVal called" << endl;
  exp->Dump();
  ir.symbolTable.back().intVal = ir.valueStack.back();
  ir.valueStack.pop_back();
}

void BTypeAST::Dump() const { cout << "BType called" << endl; }

string BTypeAST::ret_btype() { return btype; }

void LValAST::Dump() const {
  cout << "LVal called" << endl;
  vector<variable>::iterator t =  ir.search(ident);
  if (ir.is_ret) {
    ir.is_ident_ret = true;
    ir._ret.name = ident;
    if(!t->is_const) {
      ir.load(t);
    }
    return;
  }
  
  
  if(t->reg == "%-1")
    ir.load(t);
}

string LValAST::ret_value() {
  vector<variable>::iterator t = ir.search(ident);  
  
  if(t->type == "int") {
    ir.valueStack.push_back(ir.REG[ir.reg_len]);
    cout << "Returning t: " << ir.valueStack.back() << endl;
  }
  return t->reg;
}

void NumberAST::Dump() const {
  cout << "Number called " << endl;
  // variable t{"", number};
  if (ir.is_ret) {
    ir.is_value_ret = true;
    ir._ret.intVal = number;
  }
  // ir.n1 += 1;
  ir.valueStack.push_back(number);
}

string NumberAST::ret_value() { 
  return to_string(number);
}