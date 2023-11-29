#include "ast.hpp"


koopaIR ir;

void CompUnitAST::Dump() const  {
  func_def->Dump();
  cout << "ir.n:" << ir.n << endl;

}

void FuncDefAST::Dump() const {
  ir.append("fun @");
  ir.append(ident.c_str());
  ir.append("(): ");
  func_type->Dump();
  ir.append(" {\n");
  block->Dump();
  ir.append("}");
}

void FuncTypeAST::Dump() const {
  ir.append(type.c_str());
}

void BlockAST::Dump() const {
  ir.append(entry.c_str());
  ir.append(":\n");
  
  int n = blockitemnode.size();
  for(int i = 0; i < n; ++i){
        blockitemnode[i]->Dump();
    }
}

void BlockItemAST::Dump() const {
  if(decl != nullptr)
    decl->Dump();
  else if(stmt != nullptr)
    stmt->Dump();
}

void StmtAST::Dump() const {
  exp->Dump();
  ir.append("  ");
  ir.append(ret.c_str());
  ir.append(" ");
  if(ir.n == 0) {
    ir.append(ir.stack.back().value);
  } else {
    ir.append("%");
    ir.append(ir.n - 1);
  }
  ir.append("\n");
  cout << "Returning " << ir.stack.back().value << endl;
}

void PrimaryExpAST::Dump() const { 
  cout << "PrimaryExp called" << endl;
  if(exp != nullptr)
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
  if(addexp == nullptr) {
    mulexp->Dump();
    return;
  }
  addexp->Dump();
  mulexp->Dump();

  // 先取出栈顶数字
  int _1 = ir.stack.back().value;
  ir.stack.pop_back();
  int _2 = ir.stack.back().value;
  ir.stack.pop_back();

  // 计算并存储结果，结果保存到寄存器和栈中
  if(op == "add") {
    ir.stack.push_back({"", _1 + _2});
    ir.REG[ir.n] = {"", _1 + _2};
  } else if(op == "sub") {
    ir.stack.push_back({"", _1 - _2});
    ir.REG[ir.n] = {"", _1 - _2};
  }

  // 输出指令
  // ir.ins(op, ir.search(_2), ir.search(_1));
  
}


void MulAST::Dump() const {
  cout << "Mul called" << endl;
  if(mulexp == nullptr) {
    unaryexp->Dump();
    return;
  }
  mulexp->Dump();
  unaryexp->Dump();
 
  int _1 = ir.stack.back().value;
  ir.stack.pop_back();
  int _2 = ir.stack.back().value;
  ir.stack.pop_back();


  // 计算并存储结果
  if(op == "mul") {
      ir.stack.push_back({"", _1 * _2});
      ir.REG[ir.n] = {"", _1 * _2};
  } else if(op == "div") {
      ir.stack.push_back({"", _1 / _2});
      ir.REG[ir.n] = {"", _1 / _2};
  } else if(op == "mod") {
      ir.stack.push_back({"", _1 % _2});
      ir.REG[ir.n] = {"", _1 % _2};
  }
  // ir.ins(op, ir.search(_2), ir.search(_1));
  
}

void RelExpAST::Dump() const {
  cout << "RelExp called" << endl;
  if(relexp == nullptr) {
    addexp->Dump();
    return;
  }
    
  relexp->Dump();
  addexp->Dump();

  int _1 = ir.stack.back().value;
  ir.stack.pop_back();
  int _2 = ir.stack.back().value;
  ir.stack.pop_back();

    // 计算并存储结果
  if(op == "lt") {
      ir.stack.push_back({"", _1 < _2});
      ir.REG[ir.n] = {"", _1 < _2};
  } else if(op == "gt") {
      ir.stack.push_back({"", _1 > _2});
      ir.REG[ir.n] = {"", _1 > _2};
  } else if(op == "le") {
      ir.stack.push_back({"", _1 <= _2});
      ir.REG[ir.n] = {"", _1 <= _2};
  } else if(op == "ge") {
      ir.stack.push_back({"", _1 >= _2});
      ir.REG[ir.n] = {"", _1 >= _2};
  }
  
  // ir.ins(op, ir.search(_2), ir.search(_1));
}

void EqExpAST::Dump() const {
  cout << "EqExp called" << endl;
  if(eqexp == nullptr) {
    relexp->Dump();
    return;
  }
  
  eqexp->Dump();
  relexp->Dump();

  int _1 = ir.stack.back().value;
  ir.stack.pop_back();
  int _2 = ir.stack.back().value;
  ir.stack.pop_back();
  
  if(op == "eq") {
    ir.stack.push_back({"", _1 == _2});
    ir.REG[ir.n] = {"", _1 == _2};
  } else if(op == "ne") {
    ir.stack.push_back({"", _1 != _2});
    ir.REG[ir.n] = {"", _1 != _2};
  }
  // ir.ins(op, ir.search(_2), ir.search(_1));
  
}

void LAndExpAST::Dump() const {
  cout << "LAndExp called" << endl;
  if(landexp == nullptr) {
    eqexp->Dump();
    return;
  }

  landexp->Dump();
  eqexp->Dump();
  
  int _1 = ir.stack.back().value;
  ir.stack.pop_back();
  int _2 = ir.stack.back().value;
  ir.stack.pop_back();

  // 首先要判断操作数是不是0
  // ir.ins("ne", ir.search(_1), "0");
  // ir.ins("ne", ir.search(_2), "0");
  _1 = (_1 != 0) ? 1 : 0;
  _2 = (_2 != 0) ? 1 : 0;

  // 再进行与操作
  ir.stack.push_back({"", _1 && _2});
  ir.REG[ir.n] = {"", _1 && _2};
  // ir.ins("and", ir.search(_2), ir.search(_1));
}

void LOrExpAST::Dump() const {
  cout << "LOrExp called" << endl;
  if(lorexp == nullptr) {
    landexp->Dump();
    return;
  }

  lorexp->Dump();
  landexp->Dump();

  int _1 = ir.stack.back().value;
  ir.stack.pop_back();
  int _2 = ir.stack.back().value;
  ir.stack.pop_back();

  // 首先要判断操作数是不是0
  // ir.ins("ne", ir.search(_1), "0");
  // ir.ins("ne", ir.search(_2), "0");
  _1 = (_1 != 0) ? 1 : 0;
  _2 = (_2 != 0) ? 1 : 0;

  // 再进行或操作
  ir.stack.push_back({"", _1 || _2});
  ir.REG[ir.n] = {"", _1 || _2};
  // ir.ins("or", ir.search(_2), ir.search(_1));
  
}


void UnaryExpAST::Dump() const {
  cout << "UnaryExp called" << endl;
  if(primaryexp != nullptr)
    primaryexp->Dump();
  else {
    // 一元运算需要先输出数字再弄运算
    unaryexp->Dump();
    unaryop->Dump();
  }
}

void UnaryOpAST::Dump() const {
  cout << "UnaryOp called" << endl;

  int _1 = ir.stack.back().value;
  ir.stack.pop_back();
  // cout << "_1: " << _1 << endl;
  
  switch(op) {
    case '+':
      // 进行计算并存储
      // cout << "Running +" << endl;
      ir.stack.push_back({"", _1});
      break;
    
    case '-':
      // cout << "Running -" << endl;
      // // ir.ins("sub", "0", "%" + to_string(ir.n - 1));

      // 进行计算并存储
      ir.stack.push_back({"", -_1});
      ir.REG[ir.n] = {"", -_1};

      // ir.ins("sub", "0", ir.search(_1));
      break;
    
    case '!':
      // cout << "Running !" << endl;
      // // ir.ins("eq", ir.search(_1), "0");
      // 进行计算并存储
      ir.stack.push_back({"",!_1});
      ir.REG[ir.n] = {"", !_1};

      // ir.ins("eq", ir.search(_1), "0");
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
  for(int i = 0; i < n; ++i){
      constdefnode[i]->Dump();
  }
  //constdefnode->Dump();
}

void BTypeAST::Dump() const {
  cout << "BType called" << endl;
  //ir.append(type);
}

void ConstDefAST::Dump() const {
  cout << "ConstDef called" << endl;
  // string name = ir.identStack.back();
  // int value = ir.valueStack.back();
  // ir.identStack.pop_back();
  // ir.valueStack.pop_back();
  
  // variable temp{name, value};
  // ir.REG[ir.n] = temp;
  // ir.stack.push_back(temp);
  constinitval->Dump();
}

void ConstInitValAST::Dump() const {
  cout << "ConstInitVal called" << endl;

  constexp->Dump();
}

void LValAST::Dump() const {
  cout << "LVal called" << endl;
  // ir.identStack.push_back(ident);
  // ir.append(ident);
}

void NumberAST::Dump() const {
  cout << "Number called " << endl;
  variable t{"", number};
  ir.stack.push_back(t);
}