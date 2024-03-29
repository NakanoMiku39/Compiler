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
  vector<unique_ptr<BaseAST> > *vec_val;
}

// lexer 返回的所有 token 种类的声明
// 注意 IDENT 和 INT_CONST 会返回 token 的值, 分别对应 str_val 和 int_val
%token VOID INT CONST RETURN LESS_EQ GREAT_EQ EQUAL NOT_EQUAL AND OR IF ELSE WHILE BREAK CONTINUE
%token <str_val> IDENT
%token <int_val> INT_CONST

// 非终结符的类型定义
%type <ast_val> CompUnitItem FuncDef FuncType FuncFParam FuncFParams FuncRParams Block BlockItemNode BlockItem Decl Stmt 
%type <ast_val> Exp PrimaryExp UnaryExp UnaryOp AddExp MulExp RelExp EqExp LAndExp LOrExp ConstExp Number
%type <ast_val> ConstDecl ConstDefNode BType ConstDef ConstInitVal VarDecl VarDefNode VarDef InitVal LVal 
%type <vec_val> CompUnitNode 

%precedence  X
%precedence ELSE
%%

// 开始符, CompUnit ::= FuncDef, 大括号后声明了解析完成后 parser 要做的事情
// 之前我们定义了 FuncDef 会返回一个 str_val, 也就是字符串指针
// 而 parser 一旦解析完 CompUnit, 就说明所有的 token 都被解析了, 即解析结束了
// 此时我们应该把 FuncDef 返回的结果收集起来, 作为 AST 传给调用 parser 的函数
// $1 指代规则里第一个符号的返回值, 也就是 FuncDef 的返回值
CompUnit
  : CompUnitNode {
    auto comp_unit = make_unique<CompUnitAST>();
    // 一个指向存放了CompUnitItem的vector的指针
    comp_unit->compunitnode = unique_ptr<vector<unique_ptr<BaseAST>>>($1);
    ast = move(comp_unit);
  }
/*
CompUnitItem
  : Decl CompUnitItem {
    // 存放了CompUnitItem的vector
    auto ast = new CompUnitItemAST();
    ast->decl = unique_ptr<DeclAST>((DeclAST*)$1);
    ast->compunititem = unique_ptr<CompUnitItemAST>((CompUnitItemAST*)$2);
    ast->tag = CompUnitItemAST::DECL;
    $$ = ast;
  }
  | FuncDef CompUnitItem {
    auto ast = new CompUnitItemAST();
    ast->funcdef = unique_ptr<FuncDefAST>((FuncDefAST*)$1);
    ast->compunititem = unique_ptr<CompUnitItemAST>((CompUnitItemAST*)$2);
    ast->tag = CompUnitItemAST::FUNCDEF;
    $$ = ast;
  }
  |  {
    auto ast = new CompUnitItemAST();
    ast->tag = CompUnitItemAST::EMPTY;
    $$ = ast;
  }
  ;
  */
  CompUnitNode
  : CompUnitItem {
    // 存放了CompUnitItem的vector
    auto vec = new vector<unique_ptr<BaseAST>>();
    vec->push_back(unique_ptr<BaseAST>($1));
    $$ = vec;
  }
  | CompUnitNode CompUnitItem {
    auto vec = $1;
    // 向存放了CompUnitItem的vector放入一个CompUnitItem
    vec->push_back(unique_ptr<BaseAST>($2));
    $$ = vec;
  }
  ;

CompUnitItem
  : FuncDef {
    // printf("compunit\n");
    auto ast = new CompUnitItemAST();
    ast->funcdef = unique_ptr<FuncDefAST>((FuncDefAST*)$1);
    ast->tag = CompUnitItemAST::FUNCDEF;
    $$ = ast;
  }
  | Decl {
    // printf("compunit\n");
    auto ast = new CompUnitItemAST();
    ast->decl = unique_ptr<DeclAST>((DeclAST*)$1);
    ast->tag = CompUnitItemAST::DECL;
    $$ = ast;
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
  : BType IDENT '(' ')' Block {
    // printf("funcdef\n");
    auto ast = new FuncDefAST();
    ast->btype = unique_ptr<BTypeAST>((BTypeAST*)$1);
    ast->ident = *unique_ptr<string>($2);
    ast->block = unique_ptr<BlockAST>((BlockAST*)$5);
    ast->tag = FuncDefAST::NOFUNCFPARAMS;
    $$ = ast;
  }
  | BType IDENT '(' FuncFParams ')' Block {
    // printf("funcdef\n");
    auto ast = new FuncDefAST();
    ast->btype = unique_ptr<BTypeAST>((BTypeAST*)$1);
    ast->ident = *unique_ptr<string>($2);
    ast->funcfparams = unique_ptr<FuncFParamsAST>((FuncFParamsAST *)$4);
    ast->block = unique_ptr<BlockAST>((BlockAST*)$6);
    ast->tag = FuncDefAST::FUNCFPARAMS;
    $$ = ast;
  }
  ;

FuncType
  : INT {
    // printf("functype\n");
    auto ast = new FuncTypeAST();
    ast->type = "i32";
    $$ = ast;
  }
  | VOID {
    // printf("functype\n");
    auto ast = new FuncTypeAST();
    ast->type = "";
    $$ = ast;
  }
  ;

FuncFParams
  : FuncFParam {
    auto ast = new FuncFParamsAST();
    ast->funcfparamnode = vector<unique_ptr<FuncFParamAST>>();
    ast->funcfparamnode.push_back(unique_ptr<FuncFParamAST>((FuncFParamAST*)$1));
    $$ = ast;
  }
  | FuncFParams ',' FuncFParam {
    auto ast = (FuncFParamsAST*)$1;
    ast->funcfparamnode.push_back(unique_ptr<FuncFParamAST>((FuncFParamAST*)$3));
    $$ = ast;
  }
  ;
/*
FuncFParamNode
  : FuncFParam {
    auto vec = new vector<unique_ptr<BaseAST>>();
    vec->push_back(unique_ptr<BaseAST>($1));
    $$ = vec;
  }
  | FuncFParamNode ',' FuncFParam {
    auto vec = $1;
    vec->push_back(unique_ptr<BaseAST>($3));
    $$ = vec;
  }
  ;
*/
FuncFParam
  : BType IDENT {
    // printf("funcfparam\n");
    auto ast = new FuncFParamAST();
    ast->btype = unique_ptr<BTypeAST>((BTypeAST*)$1);
    ast->ident = *unique_ptr<string>($2);
    $$ = ast;
  }
  ;

FuncRParams
  : Exp {
    auto ast = new FuncRParamsAST();
    ast->exp = vector<unique_ptr<ExpAST>>();
    ast->exp.push_back(unique_ptr<ExpAST>((ExpAST*)$1));
    $$ = ast;
  }
  | FuncRParams ',' Exp {
    auto ast = (FuncRParamsAST*)$1;
    ast->exp.push_back(unique_ptr<ExpAST>((ExpAST*)$3));
    $$ = ast;
  }
  ;

Block
  : '{' BlockItemNode '}' {
    // printf("block\n");
    $$ = $2;
  }
  ;

BlockItemNode
  : {
    // printf("blockitemnode\n");
    auto block = new BlockAST();
    $$ = block;
  } | BlockItem BlockItemNode {
    // printf("blockitemnode\n");
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
    // printf("blockitem\n");
    auto ast = new BlockItemAST();
    ast->decl = unique_ptr<DeclAST>((DeclAST*)$1);
    ast->tag = BlockItemAST::DECL;
    $$ = ast;
  }
  | Stmt {
    // printf("blockitem\n");
    auto ast = new BlockItemAST();
    ast->stmt = unique_ptr<StmtAST>((StmtAST*)$1);
    ast->tag = BlockItemAST::STMT;
    $$ = ast;
  }
  ;

Stmt
  : LVal '=' Exp ';' {
    // printf("stmt\n");
    auto ast = new StmtAST();
    ast->lval = unique_ptr<LValAST>((LValAST*)$1);
    ast->exp = unique_ptr<ExpAST>((ExpAST*)$3);
    ast->tag = StmtAST::LVAL;
    $$ = ast;
  }
  | Exp ';' {
    // printf("stmt\n");
    auto ast = new StmtAST();
    ast->exp = unique_ptr<ExpAST>((ExpAST*)$1);
    ast->tag = StmtAST::EXP;
    $$ = ast;
  }
  | ';' {
    // printf("stmt\n");
    auto ast = new StmtAST();
    ast->tag = StmtAST::EMPTY;
    $$ = ast;
  }
  | Block {
    // printf("stmt\n");
    auto ast = new StmtAST();
    ast->block = unique_ptr<ExpAST>((ExpAST*)$1);
    ast->tag = StmtAST::BLOCK;
    $$ = ast;
  }
  | RETURN Exp ';' {
    // printf("stmt\n");
    auto ast = new StmtAST();
    ast->exp = unique_ptr<ExpAST>((ExpAST*)$2);
    ast->tag = StmtAST::RETURNEXP;
    $$ = ast;
  }
  | RETURN ';' {
    // printf("return\n");
    auto ast = new StmtAST();
    ast->tag = StmtAST::RETURN;
    $$ = ast;
  }
  | IF '(' Exp')' Stmt %prec X {
    // printf("stmt\n");
    auto ast = new StmtAST();
    ast->exp = unique_ptr<ExpAST>((ExpAST*)$3);
    ast->stmt1 = unique_ptr<StmtAST>((StmtAST*)$5);
    ast->tag = StmtAST::IF;
    $$ = ast;
  }
  | IF '(' Exp')' Stmt ELSE Stmt {
    // printf("stmt\n");
    auto ast = new StmtAST();
    ast->exp = unique_ptr<ExpAST>((ExpAST*)$3);
    ast->stmt1 = unique_ptr<StmtAST>((StmtAST*)$5);
    ast->stmt2 = unique_ptr<StmtAST>((StmtAST*)$7);
    ast->tag = StmtAST::IFELSE;
    $$ = ast;
  } 
  | WHILE '(' Exp')' Stmt {
    // printf("while\n");
    auto ast = new StmtAST();
    ast->exp = unique_ptr<ExpAST>((ExpAST*)$3);
    ast->stmt1 = unique_ptr<StmtAST>((StmtAST*)$5);
    ast->tag = StmtAST::WHILE;
    $$ = ast;
  }
  | BREAK ';' {
    // printf("break\n");
    auto ast = new StmtAST();
    // ast->exp = unique_ptr<ExpAST>;
    ast->tag = StmtAST::BREAK;
    $$ = ast;
  }
  | CONTINUE ';' {
    // printf("continue\n");
    auto ast = new StmtAST();
    ast->tag = StmtAST::CONTINUE;
    $$ = ast;
  }
  ;

PrimaryExp  
  : '(' Exp ')' {
    // printf("primaryexp\n");
    auto ast = new PrimaryExpAST();
    ast->exp = unique_ptr<ExpAST>((ExpAST*)$2);
    ast->tag = PrimaryExpAST::EXP;
    $$ = ast;
  }
  | LVal {
    // printf("primaryexp\n");
    auto ast = new PrimaryExpAST();
    ast->lval = unique_ptr<LValAST>((LValAST*)$1);
    ast->tag = PrimaryExpAST::LVAL;
    $$ = ast;
  }
  | Number {
    // printf("primaryexp\n");
    auto ast = new PrimaryExpAST();
    ast->number = unique_ptr<NumberAST>((NumberAST*)$1);
    ast->tag = PrimaryExpAST::NUMBER;
    $$ = ast;
  }
  ;

ConstExp
  : Exp {
    // printf("constexp\n");
    auto ast = new ConstExpAST();
    ast->exp = unique_ptr<ExpAST>((ExpAST*)$1);
    $$ = ast;
  }
  ;

Exp
  : LOrExp {
    // printf("exp\n");
    auto ast = new ExpAST();
    ast->lorexp = unique_ptr<LOrExpAST>((LOrExpAST*)$1);
    $$ = ast;
  }
  ;

AddExp
  : MulExp {
    // printf("addexp\n");
    auto ast = new AddExpAST();
    ast->mulexp = unique_ptr<MulExpAST>((MulExpAST*)$1);
    ast->tag = AddExpAST::MULEXP;
    $$ = ast;
  }
  | AddExp '+' MulExp {
    // printf("addexp\n");
    auto ast = new AddExpAST();
    ast->addexp = unique_ptr<AddExpAST>((AddExpAST*)$1);
    ast->mulexp = unique_ptr<MulExpAST>((MulExpAST*)$3);
    ast->tag = AddExpAST::ADDEXP;
    ast->op = "add";
    $$ = ast;

  }
  | AddExp '-' MulExp {
    // printf("addexp\n");
    auto ast = new AddExpAST();
    ast->addexp = unique_ptr<AddExpAST>((AddExpAST*)$1);
    ast->mulexp = unique_ptr<MulExpAST>((MulExpAST*)$3);
    ast->tag = AddExpAST::ADDEXP;
    ast->op = "sub";
    $$ = ast;
  }
  ;

MulExp
  : UnaryExp  {
    // printf("mulexp\n");
    auto ast = new MulExpAST();
    ast->unaryexp = unique_ptr<UnaryExpAST>((UnaryExpAST*)$1);
    ast->tag = MulExpAST::UNARYEXP;
    $$ = ast;
  }
  | MulExp '*' UnaryExp {
    // printf("mulexp\n");
    auto ast = new MulExpAST();
    ast->mulexp = unique_ptr<MulExpAST>((MulExpAST*)$1);
    ast->unaryexp = unique_ptr<UnaryExpAST>((UnaryExpAST*)$3);
    ast->tag = MulExpAST::MULEXP;
    ast->op = "mul";
    $$ = ast;
  }
  | MulExp '/' UnaryExp {
    // printf("mulexp\n");
    auto ast = new MulExpAST();
    ast->mulexp = unique_ptr<MulExpAST>((MulExpAST*)$1);
    ast->unaryexp = unique_ptr<UnaryExpAST>((UnaryExpAST*)$3);
    ast->tag = MulExpAST::MULEXP;
    ast->op = "div";
    $$ = ast;
  }
  | MulExp '%' UnaryExp {
    // printf("mulexp\n");
    auto ast = new MulExpAST();
    ast->mulexp = unique_ptr<MulExpAST>((MulExpAST*)$1);
    ast->unaryexp = unique_ptr<UnaryExpAST>((UnaryExpAST*)$3);
    ast->tag = MulExpAST::MULEXP;
    ast->op = "mod";
    $$ = ast;
  }
  ;

RelExp
  : AddExp {
    // printf("relexp\n");
    auto ast = new RelExpAST();
    ast->addexp = unique_ptr<AddExpAST>((AddExpAST*)$1);
    ast->tag = RelExpAST::ADDEXP;
    $$ = ast;
  }
  | RelExp '<' AddExp {
    // printf("relexp\n");
    auto ast = new RelExpAST();
    ast->relexp = unique_ptr<RelExpAST>((RelExpAST*)$1);
    ast->addexp = unique_ptr<AddExpAST>((AddExpAST*)$3);
    ast->tag = RelExpAST::RELEXP;
    ast->op = "lt";
    $$ = ast;
  }
  | RelExp '>' AddExp {
    // printf("relexp\n");
    auto ast = new RelExpAST();
    ast->relexp = unique_ptr<RelExpAST>((RelExpAST*)$1);
    ast->addexp = unique_ptr<AddExpAST>((AddExpAST*)$3);
    ast->tag = RelExpAST::RELEXP;
    ast->op = "gt";
    $$ = ast;
  }
  | RelExp LESS_EQ AddExp {
    // printf("relexp\n");
    auto ast = new RelExpAST();
    ast->relexp = unique_ptr<RelExpAST>((RelExpAST*)$1);
    ast->addexp = unique_ptr<AddExpAST>((AddExpAST*)$3);
    ast->tag = RelExpAST::RELEXP;
    ast->op = "le";
    $$ = ast;
  }
  | RelExp GREAT_EQ AddExp {
    // printf("relexp\n");
    auto ast = new RelExpAST();
    ast->relexp = unique_ptr<RelExpAST>((RelExpAST*)$1);
    ast->addexp = unique_ptr<AddExpAST>((AddExpAST*)$3);
    ast->tag = RelExpAST::RELEXP;
    ast->op = "ge";
    $$ = ast;
  }
  ;


EqExp
  : RelExp {
    // printf("eqexp\n");
    auto ast = new EqExpAST();
    ast->relexp = unique_ptr<RelExpAST>((RelExpAST*)$1);
    ast->tag = EqExpAST::RELEXP;
    $$ = ast;
  }
  | EqExp EQUAL RelExp {
    // printf("eqexp\n");
    auto ast = new EqExpAST();
    ast->eqexp = unique_ptr<EqExpAST>((EqExpAST*)$1);
    ast->relexp = unique_ptr<RelExpAST>((RelExpAST*)$3);
    ast->tag = EqExpAST::EQEXP;
    ast->op = "eq";
    $$ = ast;
  }
  | EqExp NOT_EQUAL RelExp {
    // printf("eqexp\n");
    auto ast = new EqExpAST();
    ast->eqexp = unique_ptr<EqExpAST>((EqExpAST*)$1);
    ast->relexp = unique_ptr<RelExpAST>((RelExpAST*)$3);
    ast->tag = EqExpAST::EQEXP;
    ast->op = "ne";
    $$ = ast;
  }
  ;


LAndExp
  : EqExp {
    // printf("landexp\n");
    auto ast = new LAndExpAST();
    ast->eqexp = unique_ptr<EqExpAST>((EqExpAST*)$1);
    ast->tag = LAndExpAST::EQEXP;
    $$ = ast;
  }
  | LAndExp AND EqExp {
    // printf("landexp\n");
    auto ast = new LAndExpAST();
    ast->landexp = unique_ptr<LAndExpAST>((LAndExpAST*)$1);
    ast->eqexp = unique_ptr<EqExpAST>((EqExpAST*)$3);
    ast->tag = LAndExpAST::LANDEXP;
    $$ = ast;
  }
  ;


LOrExp
  : LAndExp {
    // printf("lorexp\n");
    auto ast = new LOrExpAST();
    ast->landexp = unique_ptr<LAndExpAST>((LAndExpAST*)$1);
    ast->tag = LOrExpAST::LANDEXP;
    $$ = ast;
  }
  | LOrExp OR LAndExp {
    // printf("lorexp\n");
    auto ast = new LOrExpAST();
    ast->lorexp = unique_ptr<LOrExpAST>((LOrExpAST*)$1);
    ast->landexp = unique_ptr<LAndExpAST>((LAndExpAST*)$3);
    ast->tag = LOrExpAST::LOREXP;
    $$ = ast;
  }
  ;

UnaryExp
  : PrimaryExp {
    // printf("unaryexp\n");
    auto ast = new UnaryExpAST();
    ast->primaryexp = unique_ptr<PrimaryExpAST>((PrimaryExpAST*)$1);
    ast->tag = UnaryExpAST::PRIMARYEXP;
    $$ = ast;
  }
  | UnaryOp UnaryExp {
    // printf("unaryexp\n");
    auto ast = new UnaryExpAST();
    ast->unaryop = unique_ptr<UnaryOpAST>((UnaryOpAST*)$1);
    ast->unaryexp = unique_ptr<UnaryExpAST>((UnaryExpAST*)$2);
    ast->tag = UnaryExpAST::UNARYEXP;
    $$ = ast;
  }
  | IDENT '(' ')' {
    auto ast = new UnaryExpAST();
    ast->ident = *unique_ptr<string>($1);
    ast->tag = UnaryExpAST::NOFUNCRPARAMS;
    $$ = ast;
  }
  | IDENT '(' FuncRParams ')' {
    auto ast = new UnaryExpAST();
    ast->ident = *unique_ptr<string>($1);
    ast->funcrparams = unique_ptr<FuncRParamsAST>((FuncRParamsAST*)$3);
    ast->tag = UnaryExpAST::FUNCRPARAMS;
    $$ = ast;
  }
  ;

UnaryOp
  : '+' {
    // printf("unaryop\n");
    auto ast = new UnaryOpAST();
    ast->op = '+';
    $$ = ast;
  }  
  | '-' {
    // printf("unaryop\n");
    auto ast = new UnaryOpAST();
    ast->op = '-';
    $$ = ast;
  }  
  | '!' {
    // printf("unaryop\n");
    auto ast = new UnaryOpAST();
    ast->op = '!';
    $$ = ast;
  }
  ;

Decl
  : ConstDecl {
    // printf("decl\n");
    auto ast = new DeclAST();
    ast->constdecl = unique_ptr<ConstDeclAST>((ConstDeclAST*)$1);
    ast->tag = DeclAST::CONSTDECL;
    $$ = ast;
  }
  | VarDecl {
    // printf("decl\n");
    auto ast = new DeclAST();
    ast->vardecl = unique_ptr<VarDeclAST>((VarDeclAST*)$1);
    ast->tag = DeclAST::VARDECL;
    $$ = ast;
  }
  ;

ConstDecl
  : CONST BType ConstDefNode ';' {
    // printf("constdecl\n");
    auto ast = (ConstDeclAST *)$3;
    ast->btype = unique_ptr<BTypeAST>((BTypeAST *)$2);
    $$ = ast;
  }
  ;

ConstDefNode
  : ConstDef {
    // // printf("constdefnode\n");
    auto const_decl = new ConstDeclAST();
    const_decl->constdefnode.emplace_back((ConstDefAST *)$1);
    $$ = const_decl;
  }
  | ConstDef ',' ConstDefNode {
    // // printf("constdefnode\n");
    auto const_decl = new ConstDeclAST();
    auto const_decl_2 = unique_ptr<ConstDeclAST>((ConstDeclAST *)$3);
    const_decl->constdefnode.emplace_back((ConstDefAST *)$1);
    int n = const_decl_2->constdefnode.size();
    // // // printf("%d", n);
    for(int i = 0; i < n; ++i){
      const_decl->constdefnode.emplace_back(const_decl_2->constdefnode[i].release());
    }
    $$ = const_decl;
  }
  ;
ConstDef  
  : IDENT '=' ConstInitVal {
    // // printf("constdef\n");
    auto ast = new ConstDefAST();
    ast->ident = *unique_ptr<string>($1);
    ast->constinitval = unique_ptr<ConstInitValAST>((ConstInitValAST*)$3);
    $$ = ast;
  }
  ;

ConstInitVal  
  : ConstExp {
    // printf("constinitval\n");
    auto ast = new ConstInitValAST();
    ast->constexp = unique_ptr<ConstExpAST>((ConstExpAST*)$1);
    $$ = ast;
  }
  ;

VarDecl 
  : BType VarDefNode ';' {
    // printf("vardecl\n");
    auto ast = (VarDeclAST *)$2;
    ast->btype = unique_ptr<BTypeAST>((BTypeAST *)$1);
    $$ = ast;
  }
  ;

VarDefNode
  : VarDef {
    // // printf("vardefnode\n");
    auto var_decl = new VarDeclAST();
    var_decl->vardefnode.emplace_back((VarDefAST *)$1);
    $$ = var_decl;
  }
  | VarDef ',' VarDefNode {
    // // printf("vardefnode\n");
    auto var_decl = new VarDeclAST();
    auto var_decl_2 = unique_ptr<VarDeclAST>((VarDeclAST *)$3);
    var_decl->vardefnode.emplace_back((VarDefAST *)$1);
    int n = var_decl_2->vardefnode.size();
    // // // printf("%d", n);
    for(int i = 0; i < n; ++i){
      var_decl->vardefnode.emplace_back(var_decl_2->vardefnode[i].release());
    }
    $$ = var_decl;
  }
  ;
  /*
VarDecl 
  : BType VarDefNode ';' {
    // printf("vardecl\n");
    auto ast = new VarDeclAST();
    ast->btype = unique_ptr<BTypeAST>((BTypeAST *)$1);
    ast->vardefnode = vector<unique_ptr<VarDefAST>>();
    $$ = ast;
  }
  ;

VarDefNode
  : VarDef {
    // printf("vardefnode\n");
    auto ast = new VarDeclAST();
    ast->vardefnode.push_back(unique_ptr<VarDefAST>((VarDefAST *)$1));
    $$ = ast;
  }
  | VarDef ',' VarDefNode {
    // printf("vardefnode\n");
    auto ast = (VarDeclAST*)$3;
    ast->vardefnode.push_back(unique_ptr<VarDefAST>((VarDefAST *)$1));
    $$ = ast;
  }
  ;
    */

VarDef
  : IDENT {
    // printf("vardef\n");
    auto ast = new VarDefAST();
    ast->ident = *unique_ptr<string>($1);
    ast->tag = VarDefAST::IDENT;
    $$ = ast;
  }
  | IDENT '=' InitVal {
    // printf("vardef\n");
    auto ast = new VarDefAST();
    ast->ident = *unique_ptr<string>($1);
    ast->initval = unique_ptr<InitValAST>((InitValAST*)$3);
    ast->tag = VarDefAST::INITVAL;
    $$ = ast;
  }
  ;

InitVal  
  : Exp {
    // printf("initval\n");
    auto ast = new InitValAST();
    ast->exp = unique_ptr<ExpAST>((ExpAST*)$1);
    $$ = ast;
  }
  ;

BType  
  : INT {
    // printf("btype\n");
    auto ast = new BTypeAST();
    ast->btype = "int";
    $$ = ast;
  }
  | VOID {
    // printf("btype\n");
    auto ast = new BTypeAST();
    ast->btype = "void";
    $$ = ast;
  }
  ;

LVal 
  : IDENT {
    // printf("lval\n");
    auto ast = new LValAST();
    ast->ident = *unique_ptr<string>($1);
    $$ = ast;
  }
  ;          

Number
  : INT_CONST {
    // printf("number\n");
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
