#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include "display.h"
#include "cnfparser.h"
#include "solver.h" 


DIMACS * readClauseSet(const char *filename) {
    int i = 0,j = 0;
    char ch;
    int line[256];
    int varNum, claNum;
    FILE *input = fopen(filename, "r");
    if (input == NULL) {
        printf("\nFile open failed！\n");
        exit(-1);
    }
    
    DIMACS *data = (DIMACS *)malloc(sizeof(DIMACS));
    /*过滤掉注释*/
    while ((ch=fgetc(input)) == 'c') {
        while ((ch=fgetc(input)) != '\n');
    }
    /*过滤掉“p cnf”字样*/
    for (i=0; i<5; i++) {
        ch = fgetc(input);
    }

    fscanf(input, "%d", &varNum);
    fscanf(input, "%d", &claNum);
    
    createFormula(data, varNum, 0);

    for (i=0; i<claNum; i++) {
        j = 0;
        do {
            fscanf(input, "%d", &line[j]);
            j++;
        } while(line[j-1] != 0);
        addClause4print(data, j-1, line);
    }

    if (data->varNum != varNum) {
        printf("\nVar num error！\n");
        exit(-1);
    }
    fclose(input);
    return data;
} 

void createFormula(DIMACS *data, int literals, int clauses) {
    data->varNum = literals;
    data->claNum = clauses;
    data->root = NULL;
    data->learn_root = NULL;
    data->valuation = (int *)malloc((data->varNum+1)*sizeof(int));
    data->levalArray = (int *)malloc((data->varNum+1)*sizeof(int));
    data->countArray = (int *)malloc((data->varNum+1)*sizeof(int));
    data->levalArray[0] = -1;
 
    int i;
    for (i=1; i<=data->varNum; i++) {
        data->valuation[i] = UNCERTAIN;
        data->levalArray[i] = -1;
        data->countArray[i] = 0;
    }
}

void addClause(DIMACS *data, int n, int *clause) {
    int i;
    Clause *newClause = (Clause *)malloc(sizeof(Clause));
    if (n <= 0) {
        return;
    }
    newClause->head = NULL;
    newClause->rmv = NULL;
    newClause->length = 0;
    for (i=0; i<n; i++) {
        addLiteral(newClause, clause[i]);
    }
    if (data->root) {
        data->root->prev = newClause;
    }
    newClause->next = data->root;
    newClause->prev = NULL;

    data->root = newClause;
    data->claNum++;
}

void conjunctClause(DIMACS *data, Clause *Cp) {
    if (data->root) {
        data->root->prev = Cp;
    }
    Cp->next = data->root;
    Cp->prev = NULL;
    data->root = Cp;
    data->claNum++; 
}

void removeClause(DIMACS *data, Clause *Cp) {
    Clause *p = Cp->prev;
    if (Cp->next) {
        Cp->next->prev = p;
    }
    if (p) {
        p->next = Cp->next;
    } else {
        data->root = Cp->next;
    }
    data->claNum--;
}

void deleteClause(DIMACS *data, Clause *Cp) {
    Literal *lp;
    Clause *p = Cp->prev;
    if (Cp->next) {
        Cp->next->prev = p;
    }
    if (p) {
        p->next = Cp->next;
    } else {
        data->root = Cp->next;
    }
    for (lp=Cp->head; lp; lp=Cp->head) {
        Cp->head = lp->next;
        free(lp);
    }
    for (lp=Cp->rmv; lp; lp=Cp->rmv) {
        Cp->rmv = lp->next;
        free(lp);
    }
    free(Cp);
    data->claNum--;
}

void addLiteral(Clause *Cp, int literal) {
    Literal *p = (Literal *)malloc(sizeof(Literal));
    if (!p) {
        return;
    }
    p->index = literal;
    p->next = Cp->head;
    Cp->head = p;
    Cp->length++;
}

void removeLC(DIMACS *data, LearnClause *Lcp) {
    LearnClause *lcq = data->learn_root;
    if (Lcp == data->learn_root) {
        data->learn_root = Lcp->next;
    } else {
        for (lcq=data->learn_root; lcq; lcq=lcq->next) {
            if (lcq->next == Lcp) {
                lcq->next = Lcp->next;
                break;
            }
        }
    }
    deleteClause(data, Lcp->clause);
    free(Lcp);
}

Literal *removeLiteral(Clause *Cp, int literal) {
    Literal *p, *q = NULL;
    for (p=Cp->head; p; p=p->next) {
        if (p->index == literal) {
            Cp->length--;
            if (!q) {
                Cp->head = p->next;
            } else {
                q->next = p->next;
            }
            p->next = Cp->rmv;
            Cp->rmv = p;
            return p;
        }
        q = p;
    }
    return NULL;
}

void retrieveLiteral(Clause *Cp, Literal *Lp) {
    Literal *Lq = NULL;
    if(Cp->rmv == Lp) {
        Cp->rmv = Lp->next;
    } else {
        for (Lq=Cp->rmv; Lq && Lq->next!=Lp; Lq=Lq->next) ;
        if (Lq == NULL) {
            return;
        }
        Lq->next = Lp->next;
    }
    Lp->next = Cp->head;
    Cp->head = Lp;
    Cp->length++;
}

void ValueAssign(DIMACS *data, int index, int level) {
    data->valuation[abs(index)] = (index>0?1:-1);//保存选取文字的赋值
    data->levalArray[abs(index)] = level;//保存选取文字的决策层
}

//判断子句是否为空
int isClauseEmpty(DIMACS *data) {
    return (data->claNum)?FALSE:TRUE;
}

//判断子句是否为单子句
int isUnitClause(Clause *Cp) {
    return (Cp->length==1)?TRUE:FALSE;
}

//寻扎单子句
Clause* findUnitClause(DIMACS *data) {
    Clause *p;
    for (p=data->root; p; p=p->next) {
        if (isUnitClause(p) == TRUE) {
            return p;
        }
    }
    return NULL;
}

//判断子句集是否包含空子句
int containEmptyClause(DIMACS *data) {
    Clause *p;
    for (p=data->root; p; p=p->next) {
        if (!(p->length)) {
            return TRUE;
        }
    }
    return FALSE;
}

//判断子句集是否为空
int containClause(DIMACS *data) {
    return (data->claNum)?TRUE:FALSE;
}

//子句判真
int evaluateClause(Clause *Cp, int index) {
    int flag = NOT_EXISTS;
    Literal *p;
    for (p=Cp->head; p; p=p->next) {
        if (index == p->index) {
            return TRUE;
        } else if (-index == p->index) {
            flag = UNCERTAIN;
        }
    }
    return flag;
}

//打印子句
void printClause(DIMACS *data) {
    Literal *lp;
    Clause *cp;
    for (cp=data->root; cp; cp=cp->next) {
        for (lp=cp->head; lp; lp=lp->next) {
            printf("%d ", lp->index);
        }
        printf("\n");
    }
}

//将求解结果保存到文件
void writeSolution(const char *filename, int status, DIMACS *data, double runtime) {
    FILE *output = fopen(filename, "w");
    int i;
    if (status == TRUE) {
        for (i=1; i<=data->claNum; i++) {
            if (data->valuation[i] == 0) {
                break;
            }
        }
        fprintf(output, "s 1\n");
    	fprintf(output, "v ");
    	for (i=1; i<=data->varNum; i++) {
            fprintf(output, "%d ", data->valuation[i]*i);
    	}
    } else {
    	fprintf(output,"s 0");
	}
    fprintf(output, "\nt %.2lfms", runtime);
    fclose(output);
}

//用于打印子句的子句插入函数
void addClause4print(DIMACS *data,int len,int *literal) {
    if (len<0) {
        return;
    }
    Clause *cp= (Clause *)malloc(sizeof(Clause));
    cp->length = 0;
    cp->head = NULL;
    cp->next = NULL;
    cp->prev = NULL;
    cp->rmv = NULL;
    
    int i;
    for (i=0; i<len; i++) {
        if (literal[i] > data->varNum) {
            printf("\nVar num error！\n");
            exit(-1);
        }
        addLiteral4print(cp, literal[i]);
    }
    Clause *itr = data->root;
    if (itr) {
        for (; itr->next; itr=itr->next) ;
        itr->next = cp;
        cp->prev = itr;
    } else {
        data->root = cp;
    }
    data->claNum++;
}

//用于打印子句的变元插入函数
void addLiteral4print(Clause *cp, int index) {
    Literal *lp = (Literal *)malloc(sizeof(Literal)), *itr = cp->head;
    lp->index = index;
    if (itr) {
        for (; itr->next; itr=itr->next);
        itr->next = lp;
        lp->next = NULL;
    } else {
        cp->head = lp;
        lp->next = NULL;
    }
    cp->length++;
}


