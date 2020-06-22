#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "display.h"
#include "solver.h"
#include "cnfparser.h"
#include "sudoku.h"


extern int board[9][9];

void menu() {
	printf("\n\n\n\n\t-------------------------\n"
	       "\t|                       |\n"
           "\t|     Play or Solve ?   |\n"
           "\t|                       |\n"
           "\t|       1->PLAY         |\n"
           "\t|                       |\n"
           "\t|       2->SOLVE        |\n"
           "\t|                       |\n"
           "\t|       0->EXIT         |\n"
           "\t|                       |\n"
           "\t-------------------------\n"
           "\nYour option:\n");
    int option;
    while (1) {
        scanf("%d", &option);
        getchar();
        if (option == 1) {
            play();
        } else if (option == 2) {
            loadBoard();
            printBoard();
            solve();
        } else if (option == 0) {
            exit(0);
        }
    }
}

void loadBoard() {
    char filepath[1024];
    int temp, row[9][10] = {0}, column[9][10] = {0}, block[3][3][10] = {0};
    printf("Please input sudoku file path:\n");
    scanf("%s", filepath);
    FILE *fp = fopen(filepath, "r");
    if (fp == NULL) {
        printf("File open failed!\n");
        exit(-1);
    }
    for (int i=0; i<9; ++i) {
        for (int j=0; j<9; ++j) {
            fscanf(fp, "%d", &temp);
            if (temp < 0 || temp > 9) {
                printf("Invalid sudoku file!\n");
                exit(-1);
            }
            board[i][j] = temp;
            if (temp > 0) {
                if (row[i][temp]) {
                    printf("Row conflict detected at (%d, %d)!\n", i, j);
                    exit(-1);
                }
                row[i][temp] = 1;
                if (column[j][temp]) {
                    printf("Column conflict detected at (%d, %d)!\n", i, j);
                    exit(-1);
                }
                column[j][temp] = 1;
                if (block[i/3][j/3][temp]) {
                    printf("Block conflict detected at (%d, %d)!\n", i, j);
                    exit(-1);
                }
                block[i/3][j/3][temp] = 1;
            }
        }
    }
}

void printBoard() {
    printf("\n-------------------------\n");
    for (int i=0; i<9; ++i) {
        if (i%3==0 && i>0) {
            printf("|-----------------------|\n");
        }
        for (int j=0; j<9; ++j) {
            if (j%3 == 0) {
                printf("| ");
            }
            printf("%d ", board[i][j]);
        }
        printf("|\n");
    }
    printf("-------------------------\n");
}

void play() {
    // newSudoku();
    printBoard();
    printf("\nPress Any Key to Get Solution.\n");
    if (getchar()) {
        solve();
    }
}

void solve() {
    time_t start, end;
    float runtime;
    int status;
    printf("Transform to CNF...\n");
    toCNF();
    printf("Solving...\n");
    DIMACS *data = readClauseSet("sudoku.cnf");
    start = clock();
    status = DPLL(data, 0);
    end = clock();
    runtime = (float)(end-start)/CLOCKS_PER_SEC;
    writeSolution("sudoku.res", status, data, runtime);
    if (status != TRUE) {
        printf("No solution!\n");
        exit(0);
    }
    toSudoku();
    printf("Here is the solution: (%f seconds)\n", runtime);
    printBoard();
    saveSolution("solution.txt");
    exit(0);
}
