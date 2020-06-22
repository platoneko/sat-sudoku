#ifndef CNF_H
#define CNF_H

#define LENGTH_MAX 50
#define NOT_EXISTS 2
#define CLAUSE 0
#define LITERAL 1
#define SPLIT 2

#define TRUE 1
#define FALSE -1
#define UNCERTAIN 0


typedef struct Literal {
    int index;//变元下标
    struct Literal *next;//指向下一个变元
} Literal;

typedef struct Clause {
    int length;//子句长度
    struct Literal *head;//指向第一个变元
    struct Literal *rmv;//指向第一个被删除的变元
    struct Clause *prev;//指向上一个子句
    struct Clause *next;//指向下一个子句
} Clause;

typedef struct LearnClause {
    int isInStack;              //是否在回溯栈中
    int level;                  //决策级数
    int count;                  //调用次数
    struct Clause *clause;      //指向学习子句
    struct LearnClause *next;   //指向下一个学习子句
} LearnClause;

typedef struct Stack{
    int tag;
    int level;
    struct Literal *Lp;
    struct Clause *Cp;
    struct Stack *next;
} Stack;

typedef struct DIMACS {
    int varNum;//变元个数
    int claNum;//子句个数
    int *levalArray;//记录决策层的数组
    int *valuation;//记录变元赋值的数组
    int *countArray;//记录变元出现次数的数组
    struct Clause *root;//指向第一个子句
    struct LearnClause *learn_root;//指向第一个学习子句
} DIMACS;

#endif