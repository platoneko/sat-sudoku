#ifndef SOLVER_H
#define SOLVER_H

#include "cnf.h"
#include "cnfparser.h"


void unitPropagation(DIMACS *data, int index, Stack *head);
int chooseLiteral(DIMACS *data);
void saveChange(Stack *head, int tag, Clause *Cp, Literal *Lp);
void unmakeChange(DIMACS *data, Stack *head, int time, int *learnarray);
int createLearnClause(DIMACS *data, int *learnArray, int i);
void backtrackLC(DIMACS *data, int level);
void deleteLC(DIMACS *data, LearnClause *Lcp);
void backAssign(DIMACS *data, int level);
int DPLL(DIMACS *data, const int level);

#endif
