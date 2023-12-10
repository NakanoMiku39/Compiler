# Tutorial site:
https://pku-minic.github.io/online-doc/#/lv1-main/lexer-parser

# Docker环境测试
make  
build/compiler 
build/compiler -riscv hello.c -o hello.s

# 使用lldb
## 运行的程序需要参数
lldb /path/to/your/executable -- arg1 arg2 arg3