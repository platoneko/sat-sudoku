#include <stdlib.h>
#include "solver.h"


void unitPropagation(DIMACS *data, int index, Stack *head) {
    Literal *Lp;
    Clause *Cp = data->root;
    Clause *Cq;
    while (Cp)  {
        switch (evaluateClause(Cp,index))  {
        /*若子句可满足，将该子句删除*/
        case TRUE:
            removeClause(data, Cp);
            saveChange(head, CLAUSE, Cp, NULL); //保存被删除的子句
            Cp = Cp->next;
            break;
        case UNCERTAIN:
        /*若子句不可满足，将子句集中删除其负文字*/
            Lp = removeLiteral(Cp,-index);
            saveChange(head, LITERAL, Cp, Lp);  //保存被删除的文字
            if (Cp->length <= 2) {               //若当前子句长度为2，将该子句插入到子句集中作为第一个子句
                Cq = Cp->next;
                removeClause(data, Cp);
                conjunctClause(data, Cp);
                if (Cp->length == 0) {           //若当前子句为空子句，返回
                    return;
                } else {
                    Cp = Cq;                    //若当前子句为单子句，将该子句插入到子句集中作为第一个子句
                }
            } else {
                Cp = Cp->next;
            }
            break;
        default:
            Cp = Cp->next;
            break;
        }
    }
}

int chooseLiteral(DIMACS *data) {
    return data->root->head->index;
}

/*保存被删除的子句或者文字*/
void saveChange(Stack *head, int tag, Clause *Cp, Literal *Lp) {
    if (head == NULL) {
        return;
    }
    Stack *change = (Stack *)malloc(sizeof(Stack));

    change->tag = tag;
    switch (tag) {
    case CLAUSE:
        change->Cp = Cp;        //保存子句
        break;
    case LITERAL:
        change->Cp = Cp;        //保存所在子句地址
        change->Lp = Lp;        //保存变元
        break;
    case SPLIT:
        change->Cp = Cp;        //保存子句
        break;
    default:
        break;
    }
    change->next = head->next;
    head->next = change;
}

void unmakeChange(DIMACS *data, Stack *head, int time, int *learnarray) {
    /*
    learnarray组成:第一个元素为长度，第二个开始是变元
    学习子句计划
    加入a[]:变元x在当前层的赋值状态X=valuation[x]*x加入a[]
    第一次撤销肯定是冲突的子句C变空，所以第一次撤销后该子句C的DIVIDE后的所有变元都加入a[]

    1.将冲突的变元加入a[]_0

    2.第i次撤销后，先检查该子句是否有元素，若该子句的元素只有一个(tag为CLAUSE)，且元素在a[]_0内（元素的值取反后与a[]_0比较），该子句C的DIVIDE后的所有变元都加入a[]_0
    “若该子句的元素只有一个”：是不是蕴含关系
    “且元素在a[]_0内”：是否与冲突子句有关

    3.将所有撤销完成后，得到a[]_1，删去a[]_1中本层所有的变量赋值（决策以及推倒出）的变元，得到a[]_2
    本层：a[]_1中变元i的levalArray[i]的最大值F

    4.得到需要返回的决策层，即a[]_2中变元i的levalArray[i]的最大值toF

    5.加入本层的决策X得到a[]
    决策X：a[]_1中变元i的levalArray[i]的值为F且为最后一个(由后往前遍历)

    6.插入学习子句Cl => learnArray[]得到最后蕴含关系
    */
    int i = 1, j;
    Literal *Lp;
    Stack *Sp = head->next;
    while (Sp && time != 0) {
        switch (Sp->tag) {
        case CLAUSE:
            conjunctClause(data, Sp->Cp);
            break;
        case LITERAL:
            //子句变元遵循规律：DIVIDE后的变元决策层为降序
            //撤销本层的变元删除操作只需要将本决策层的变元放在DIVIDE前即可
            retrieveLiteral(Sp->Cp, Sp->Lp);
            if (learnarray) {
                //将冲突的变元加入a[]_0
                if(Sp->Cp->length ==1) {
                    learnarray[i++] = Sp->Cp->head->index;
                    learnarray[i++] = 0 - Sp->Cp->head->index;
                }
            }
            break;
        case SPLIT:
            deleteClause(data, Sp->Cp);
            break;
        default:
            break;
        }

        if (learnarray) {
            if (Sp->Cp->length == 1) {
                for (j=1; j<i; j++) {
                    if (Sp->Cp->head->index == -learnarray[j]) {
                        break;
                    }
                }
                //且元素在a[]_0内（元素的值取反后与a[]_0比较），该子句C的DIVIDE后的所有变元都加入a[]_0
                if (i != j) {
                    for(Lp=Sp->Cp->rmv; Lp; Lp=Lp->next) {
                        //删去重复的变元
                        for (j=1; j<i; j++) {
                            if (learnarray[j] == Lp->index) {
                                break;
                            }
                        }
                        if (j == i) {
                            learnarray[i] = Lp->index;
                            i++;
                        }
                    }
                }
            }
        }
        time--;
        head->next = head->next->next;
        free(Sp);
        Sp = head->next;
    }
    if (learnarray) {
        learnarray[0] = i-1;
    }

}

int createLearnClause(DIMACS *data, int *learnArray, int i) {
    int j, k;
    int index = data->root->head->index;
    int level = 0, backto = 0;

    /*由后往前减少赋值次数*/
    for (j=i-1; j>=0; j--) {
        if (level < data->levalArray[abs(learnArray[j])]) {
            level = data->levalArray[abs(learnArray[j])];
        }
    }
    /*删去levelArray[]中本层所有的变量赋值（决策以及推倒出）的变元*/
    for (k=0,j=0; j<i; j++) {
        if (data->levalArray[abs(learnArray[j])]!=level && data->levalArray[abs(learnArray[j])]!=-1) {
            learnArray[k] = learnArray[j];
            k++;
        }
    }
    /*找出levelArray[]中最大的数，即为要回溯到的决策层*/
    for (j=0; j<k; j++) {
        if (backto<data->levalArray[abs(learnArray[j])]) {
            backto=data->levalArray[abs(learnArray[j])];
        }
    }

    int len = k+1;//长学习子句的原长

    /*排序变元，使得变元决策层为降序*/
    for (i=1; i<k; i++) {
        for (j=i-1; j>=0; j--) {
            if (data->levalArray[abs(learnArray[j+1])] > data->levalArray[abs(learnArray[j])]) {
                int tmp = learnArray[j+1];
                learnArray[j+1] = learnArray[j];
                learnArray[j] = tmp;
            }
        }
    }
    /*加入本层的决策X得到levelArray[]*/
    learnArray[k++] = -index;
    for (j=k-1; j>0; j--) {
        int tmp = learnArray[j-1];
        learnArray[j-1] = learnArray[j];
        learnArray[j] = tmp;
    }

    /*插入学习子句到learnArray[]中得到最后蕴含关系*/
    addClause(data, k, learnArray);

    LearnClause *lcp = (LearnClause *)malloc(sizeof(LearnClause));
    lcp->isInStack = FALSE;
    lcp->level = level;
    lcp->clause = data->root;
    lcp->count = len;
    lcp->next = data->learn_root;
    data->learn_root = lcp;
    
    /*学习子句的变元有赋值*/
    while (data->root->head) {
        removeLiteral(data->root, data->root->head->index);
    }

    return backto;
}

void backtrackLC(DIMACS *data, int level)
 {
    Literal *Lp;
    LearnClause *lcp = data->learn_root, *lcq = NULL;
    while (lcp) {
        if (lcp->isInStack == FALSE) {
            for (Lp=lcp->clause->rmv; Lp && data->levalArray[abs(Lp->index)]==level; Lp=lcp->clause->rmv) {
                retrieveLiteral(lcp->clause,Lp);
            }
            if (Lp == NULL) {
                if (lcp->clause->length > LENGTH_MAX) {
                    if (lcq == NULL) {
                        data->learn_root = lcp->next;
                        deleteClause(data, lcp->clause);
                        free(lcp);
                        lcp = data->learn_root;
                        continue;
                    } else {
                        lcq->next = lcp->next;
                        deleteClause(data, lcp->clause);
                        free(lcp);
                        lcp = lcq;
                    }
                } else {
                    lcp->isInStack=TRUE;
                }
            }
        }
        lcq = lcp;
        lcp = lcp->next;
    }
}

void deleteLC(DIMACS *data, LearnClause *Lcp) {
    Literal *Lp1, *Lp2;
    LearnClause *lcp = data->learn_root;
    lcp = Lcp->next;
    if (lcp) {
        Lp1 = lcp->clause->head;
        if (Lp1 && Lp1->index==Lcp->clause->head->index) {
            for (Lp1=lcp->clause->rmv,Lp2=data->learn_root->clause->rmv; Lp1 && Lp2;) {
                if (Lp1->index != Lp2->index) {
                    break;
                }
                Lp1 = Lp1->next;
                Lp2 = Lp2->next;
            }
            if(Lp1==NULL && Lp2==NULL) {
                LearnClause *tmp = lcp->next;
                removeLC(data, lcp);
                lcp = tmp;
            } else {
                lcp = lcp->next;
            }
        }
    }
}

void backAssign(DIMACS *data, int level) {
    int i;
    for (i=1; i<=data->varNum; i++) {
        if (data->levalArray[i] == level) {
            data->valuation[i] = UNCERTAIN;
            data->levalArray[i] = -1;
        }
    }
}

int DPLL(DIMACS *data, const int level)
 {
    int status, picked;//求解状态、选取的变元
    int backtrackLevel = level;//回溯层数
    Clause *Cp;
    Stack stack;
    stack.level = level;
    stack.next = NULL;
    do {
        /*利用选取出的文字，结合单子句传播规则，化简子句集*/
        while ((Cp = findUnitClause(data)) != NULL) {
            /*若存在单子句*/
            ValueAssign(data, Cp->head->index, level);//记录到数组
            unitPropagation(data, Cp->head->index, &stack);//单子句传播规则
            if (containEmptyClause(data) == TRUE) {
                /*若化简过后的子句集存在空子句，说明发生了冲突，需要回溯*/
                int *learnArray= (int *)malloc(sizeof(int) * data->varNum);
                unmakeChange(data, &stack, -1, learnArray);//还原子句集
                /*如果取值不成立，可能要添加学习子句并非时序回溯*/
                backtrackLevel = createLearnClause(data, learnArray+1, learnArray[0]);
                free(learnArray);
                backtrackLC(data, level);
                deleteLC(data, data->learn_root);
                backAssign(data, level);//回溯赋值
                return backtrackLevel-level;
            } else if(containClause(data) == FALSE) {//如果没有子句
                unmakeChange(data, &stack, -1, NULL);//还原到初始集
                backtrackLC(data, level);//回溯学习子句
                return TRUE;
            }
        }//while
        picked = chooseLiteral(data);//选取下一个变元进行递归
        addClause(data, 1, &picked);
        saveChange(&stack, SPLIT, data->root, NULL);//保存插入操作
        status = DPLL(data, level+1);
        unmakeChange(data, &stack, 1, NULL);
    } while (status == FALSE);
    
    if(status == TRUE) {
        unmakeChange(data, &stack, -1, NULL);
        backtrackLC(data, level);//学习子句回溯
        return TRUE;
    }
    //按返回值多层回溯
    else {
        unmakeChange(data, &stack, -1, NULL);
        backtrackLC(data, level);//学习子句回溯
        backAssign(data, level);//回溯赋值
        return status+1;
    }
}