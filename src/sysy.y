%code requires {
  #include <memory>
  #include <string>
}

%{

#include <iostream>
#include <memory>
#include <string>
#include "ast.hpp"

// 声明 lexer 函数和错误处理函数
int yylex();
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s);

using namespace std;

%}

// 定义 parser 函数和错误处理函数的附加参数
// 我们需要返回一个字符串作为 AST, 所以我们把附加参数定义成字符串的智能指针
// 解析完成后, 我们要手动修改这个参数, 把它设置成解析得到的字符串
%parse-param { std::unique_ptr<BaseAST> &ast }

// yylval 的定义, 我们把它定义成了一个联合体 (union)
// 因为 token 的值有的是字符串指针, 有的是整数
// 之前我们在 lexer 中用到的 str_val 和 int_val 就是在这里被定义的
// 至于为什么要用字符串指针而不直接用 string 或者 unique_ptr<string>?
// 请自行 STFW 在 union 里写一个带析构函数的类会出现什么情况
%union {
  std::string *str_val;
  int int_val;
  BaseAST *ast_val;
}

// lexer 返回的所有 token 种类的声明
// 注意 IDENT 和 INT_CONST 会返回 token 的值, 分别对应 str_val 和 int_val
%token INT CONST RETURN LESS_EQ GREAT_EQ EQUAL NOT_EQUAL AND OR IF ELSE WHILE BREAK CONTINUE
%token <str_val> IDENT
%token <int_val> INT_CONST

// 非终结符的类型定义
%type <ast_val> FuncDef FuncType Block BlockItemNode BlockItem Stmt Number
%type <ast_val> Exp PrimaryExp UnaryExp UnaryOp AddExp MulExp RelExp EqExp LAndExp LOrExp ConstExp 
%type <ast_val> Decl ConstDecl ConstDefNode BType ConstDef ConstInitVal LVal 


%%

// 开始符, CompUnit ::= FuncDef, 大括号后声明了解析完成后 parser 要做的事情
// 之前我们定义了 FuncDef 会返回一个 str_val, 也就是字符串指针
// 而 parser 一旦解析完 CompUnit, 就说明所有的 token 都被解析了, 即解析结束了
// 此时我们应该把 FuncDef 返回的结果收集起来, 作为 AST 传给调用 parser 的函数
// $1 指代规则里第一个符号的返回值, 也就是 FuncDef 的返回值
CompUnit
  : FuncDef {
    auto comp_unit = make_unique<CompUnitAST>();
    comp_unit->func_def = unique_ptr<BaseAST>($1);
    ast = move(comp_unit);
  }
  ;

// FuncDef ::= FuncType IDENT '(' ')' Block;
// 我们这里可以直接写 '(' 和 ')', 因为之前在 lexer 里已经处理了单个字符的情况
// 解析完成后, 把这些符号的结果收集起来, 然后拼成一个新的字符串, 作为结果返回
// $$ 表示非终结符的返回值, 我们可以通过给这个符号赋值的方法来返回结果
// 你可能会问, FuncType, IDENT 之类的结果已经是字符串指针了
// 为什么还要用 unique_ptr 接住它们, 然后再解引用, 把它们拼成另一个字符串指针呢
// 因为所有的字符串指针都是我们 new 出来的, new 出来的内存一定要 delete
// 否则会发生内存泄漏, 而 unique_ptr 这种智能指针可以自动帮我们 delete
// 虽然此处你看不出用 unique_ptr 和手动 delete 的区别, 但当我们定义了 AST 之后
// 这种写法会省下很多内存管理的负担
FuncDef
  : FuncType IDENT '(' ')' Block {
    auto ast = new FuncDefAST();
    ast->func_type = unique_ptr<BaseAST>($1);
    ast->ident = *unique_ptr<string>($2);
    ast->block = unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  ;

FuncType
  : INT {
    printf("functype\n");
    auto ast = new FuncTypeAST();
    ast->type = "i32";
    $$ = ast;
  }
  ;

Block
  : '{' BlockItemNode '}' {
    printf("block\n");
    $$ = $2;
  }
  ;

BlockItemNode
  : {
    printf("blockitemnode\n");
    auto block = new BlockAST();
    $$ = block;
  } | BlockItem BlockItemNode {
    printf("blockitemnode\n");
    auto block = new BlockAST();
    auto block_lst = unique_ptr<BlockAST>((BlockAST *)$2);
    block->blockitemnode.emplace_back((BlockItemAST *)$1);
    int n = block_lst->blockitemnode.size();
    for(int i = 0; i < n; ++i){
        block->blockitemnode.emplace_back(block_lst->blockitemnode[i].release());
    }
    $$ = block;
  }
  ;

BlockItem
  : Decl {
    printf("blockitem\n");
    auto ast = new BlockItemAST();
    ast->decl = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | Stmt {
    printf("blockitem\n");
    auto ast = new BlockItemAST();
    ast->stmt = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

Stmt
  : RETURN Exp ';' {
    printf("stmt\n");
    auto ast = new StmtAST();
    ast->exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  ;

PrimaryExp  
  : '(' Exp ')' {
    printf("primaryexp\n");
    auto ast = new PrimaryExpAST();
    ast->exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  | LVal {
    printf("primaryexp\n");
    auto ast = new PrimaryExpAST();
    ast->lval = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | Number {
    printf("primaryexp\n");
    auto ast = new PrimaryExpAST();
    ast->number = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

ConstExp
  : Exp {
    printf("constexp\n");
    auto ast = new ConstExpAST();
    ast->exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

Exp
  : LOrExp {
    printf("exp\n");
    auto ast = new ExpAST();
    ast->lorexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

AddExp
  : MulExp {
    printf("addexp\n");
    auto ast = new AddAST();
    ast->mulexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | AddExp '+' MulExp {
    printf("addexp\n");
    auto ast = new AddAST();
    ast->addexp = unique_ptr<BaseAST>($1);
    ast->mulexp = unique_ptr<BaseAST>($3);
    ast->op = "add";
    $$ = ast;

  }
  | AddExp '-' MulExp {
    printf("addexp\n");
    auto ast = new AddAST();
    ast->addexp = unique_ptr<BaseAST>($1);
    ast->mulexp = unique_ptr<BaseAST>($3);
    ast->op = "sub";
    $$ = ast;
  }
  ;

MulExp
  : UnaryExp  {
    printf("mulexp\n");
    auto ast = new MulAST();
    ast->unaryexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | MulExp '*' UnaryExp {
    printf("mulexp\n");
    auto ast = new MulAST();
    ast->mulexp = unique_ptr<BaseAST>($1);
    ast->unaryexp = unique_ptr<BaseAST>($3);
    ast->op = "mul";
    $$ = ast;
  }
  | MulExp '/' UnaryExp {
    printf("mulexp\n");
    auto ast = new MulAST();
    ast->mulexp = unique_ptr<BaseAST>($1);
    ast->unaryexp = unique_ptr<BaseAST>($3);
    ast->op = "div";
    $$ = ast;
  }
  | MulExp '%' UnaryExp {
    printf("mulexp\n");
    auto ast = new MulAST();
    ast->mulexp = unique_ptr<BaseAST>($1);
    ast->unaryexp = unique_ptr<BaseAST>($3);
    ast->op = "mod";
    $$ = ast;
  }
  ;

RelExp
  : AddExp {
    printf("relexp\n");
    auto ast = new RelExpAST();
    ast->addexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | RelExp '<' AddExp {
    printf("relexp\n");
    auto ast = new RelExpAST();
    ast->relexp = unique_ptr<BaseAST>($1);
    ast->addexp = unique_ptr<BaseAST>($3);
    ast->op = "lt";
    $$ = ast;
  }
  | RelExp '>' AddExp {
    printf("relexp\n");
    auto ast = new RelExpAST();
    ast->relexp = unique_ptr<BaseAST>($1);
    ast->addexp = unique_ptr<BaseAST>($3);
    ast->op = "gt";
    $$ = ast;
  }
  | RelExp LESS_EQ AddExp {
    printf("relexp\n");
    auto ast = new RelExpAST();
    ast->relexp = unique_ptr<BaseAST>($1);
    ast->addexp = unique_ptr<BaseAST>($3);
    ast->op = "le";
    $$ = ast;
  }
  | RelExp GREAT_EQ AddExp {
    printf("relexp\n");
    auto ast = new RelExpAST();
    ast->relexp = unique_ptr<BaseAST>($1);
    ast->addexp = unique_ptr<BaseAST>($3);
    ast->op = "ge";
    $$ = ast;
  }
  ;


EqExp
  : RelExp {
    printf("eqexp\n");
    auto ast = new EqExpAST();
    ast->relexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | EqExp EQUAL RelExp {
    printf("eqexp\n");
    auto ast = new EqExpAST();
    ast->eqexp = unique_ptr<BaseAST>($1);
    ast->relexp = unique_ptr<BaseAST>($3);
    ast->op = "eq";
    $$ = ast;
  }
  | EqExp NOT_EQUAL RelExp {
    printf("eqexp\n");
    auto ast = new EqExpAST();
    ast->eqexp = unique_ptr<BaseAST>($1);
    ast->relexp = unique_ptr<BaseAST>($3);
    ast->op = "ne";
    $$ = ast;
  }
  ;


LAndExp
  : EqExp {
    printf("landexp\n");
    auto ast = new LAndExpAST();
    ast->eqexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | LAndExp AND EqExp {
    printf("landexp\n");
    auto ast = new LAndExpAST();
    ast->landexp = unique_ptr<BaseAST>($1);
    ast->eqexp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;


LOrExp
  : LAndExp {
    printf("lorexp\n");
    auto ast = new LOrExpAST();
    ast->landexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | LOrExp OR LAndExp {
    printf("lorexp\n");
    auto ast = new LOrExpAST();
    ast->lorexp = unique_ptr<BaseAST>($1);
    ast->landexp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

UnaryExp
  : PrimaryExp {
    printf("unaryexp\n");
    auto ast = new UnaryExpAST();
    ast->primaryexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | UnaryOp UnaryExp {
    printf("unaryexp\n");
    auto ast = new UnaryExpAST();
    ast->unaryop = unique_ptr<BaseAST>($1);
    ast->unaryexp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  ;

UnaryOp
  : '+' {
    printf("unaryop\n");
    auto ast = new UnaryOpAST();
    ast->op = '+';
    $$ = ast;
  }  
  | '-' {
    printf("unaryop\n");
    auto ast = new UnaryOpAST();
    ast->op = '-';
    $$ = ast;
  }  
  | '!' {
    printf("unaryop\n");
    auto ast = new UnaryOpAST();
    ast->op = '!';
    $$ = ast;
  }
  ;

Decl
  : ConstDecl {
    printf("decl\n");
    auto ast = new DeclAST();
    ast->constdecl = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

ConstDecl
  : CONST BType ConstDefNode ';' {
    printf("constdecl\n");
    auto ast = (ConstDeclAST *)$3;
    ast->btype = unique_ptr<BTypeAST>((BTypeAST *)$2);
    $$ = ast;
  }
  ;


ConstDefNode
  : ConstDef {
    printf("constdefnode\n");
    auto const_decl = new ConstDeclAST();
    const_decl->constdefnode.emplace_back((ConstDefAST *)$1);
    $$ = const_decl;
  }
  | ConstDef ',' ConstDefNode {
    printf("constdefnode\n");
    auto const_decl = new ConstDeclAST();
    auto const_decl_2 = unique_ptr<ConstDeclAST>((ConstDeclAST *)$3);
    const_decl->constdefnode.emplace_back((ConstDefAST *)$1);
    int n = const_decl_2->constdefnode.size();
    // printf("%d", n);
    for(int i = 0; i < n; ++i){
      const_decl->constdefnode.emplace_back(const_decl_2->constdefnode[i].release());
    }
    $$ = const_decl;
  }
  ;



BType  
  : INT {
    printf("btype\n");
    auto ast = new BTypeAST();
    ast->btype = "int";
    $$ = ast;
  }
  ;

ConstDef  
  : IDENT '=' ConstInitVal {
    printf("constdef\n");
    auto ast = new ConstDefAST();
    ast->ident = *unique_ptr<string>($1);
    ast->constinitval = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

ConstInitVal  
  : ConstExp {
    printf("constinitval\n");
    auto ast = new ConstInitValAST();
    ast->constexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

LVal 
  : IDENT {
    printf("lval\n");
    auto ast = new LValAST();
    ast->ident = *unique_ptr<string>($1);
    $$ = ast;
  }
  ;          

Number
  : INT_CONST {
    printf("number\n");
    auto ast = new NumberAST();
    ast->number = $1;
    $$ = ast;
  }
  ;


%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(unique_ptr<BaseAST> &ast, const char *s) {
  cerr << "error: " << s << endl;
}
