#ifndef CNFPARSER_H
#define CNFPARSER_H

#include "cnf.h"


DIMACS* readClauseSet(const char *filename);
void createFormula(DIMACS *data, int literals, int clauses);
void addClause(DIMACS *data, int n, int *clause);
void addClause4print(DIMACS *data,int len,int *literal);
void addLiteral(Clause *Cp, int literal);
void addLiteral4print(Clause *cp, int index);
void conjunctClause(DIMACS *data, Clause *Cp);
void removeClause(DIMACS *data, Clause *Cp);
Literal* removeLiteral(Clause *Cp, int literal);
void deleteClause(DIMACS *data, Clause *Cp);

void retrieveLiteral(Clause *Cp, Literal *Lp);
void removeLC(DIMACS *data, LearnClause *Lcp);
Clause* findUnitClause();
void ValueAssign(DIMACS *data, int index, int level);
int isUnitClause(Clause *Cp);
int evaluateClause(Clause *Cp, int index);
int containEmptyClause(DIMACS *data);
int containClause(DIMACS *data);

void writeSolution(const char *filename, int status, DIMACS *data, double runtime);
void printClause(DIMACS *data);

#endif
