# UnremPG

UnremPG is a program for windows and Linux to generate passwords that you likely can not remember nor type conventionally on your keyboard.

# Compiling

I will maybe add a Makefile somewhere in the future, until then open the project in Visual Studio if you are on Windows or run one of these command if you are on Linux:  
```bash
gcc -m32 main.c -o main.out -I./include -L./include -l:BadUnicodeFilter.a  
gcc -m64 main.c -o main64.out -I./include -L./include -l:BadUnicodeFilter64.a  
```

