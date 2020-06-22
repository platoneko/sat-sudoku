#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "sudoku.h"
#include "cnfparser.h"
#include "solver.h"
#include "display.h"


extern int board[9][9];

void toCNF() {
    int temp1, temp2;
    int i, j, k, l, m;
	
	FILE *output = fopen("sudoku.cnf", "w");
	fprintf(output, "p cnf 729 3187\n");

    for (i=0; i<81; i++) {
        if (*(*board+i) != 0) {
            fprintf(output, "%d 0\n", 9*i + *(*board+i));
        }
    }
    /*归约过程*/
    /*每个方格只能填一个数字*/
    for (i=0; i<9; i++) {
        for (j=0; j<9; j++) {
            for (k=1; k<9; k++) {
                for (l=k+1; l<10; l++) {
                    temp1 = 81*i + 9*j + k;
                    temp2 = 81*i + 9*j + l;
                    fprintf(output,"-%d -%d 0\n", temp1, temp2);
                }
            }
        }
    }
    /*每排每个数字只能出现一遍*/
    for (i=1; i<10; i++) {
        for (j=1; j<10; j++) {
            for (k=1; k<10; k++) {
                temp1 = 81*(i-1)+9*(k-1)+j;
                fprintf(output, "%d ", temp1);
            }
            fprintf(output, "0\n");
        }
    }
    /*每列每个数字只能出现一遍*/
    for (i=1; i<10; i++) {
        for (j=1; j<10; j++) {
            for (k=1; k<10; k++) {
                temp1 = 81*(k-1) + 9*(i-1) + j;
                fprintf(output, "%d ", temp1);
            }
            fprintf(output, "0\n");
        }
    }
    /*每个3x3的方块里不能出现重复数字*/
    for (i=0; i<3; i++) {
        for (j=0; j<3; j++) {
            for (k=1; k<10; k++) {
                for (l=1; l<4; l++) {
                    for (m=1; m<4; m++) {
                        temp1 = 81*(3*i+l-1) + 9*(3*j+m-1) + k;
                        fprintf(output, "%d ", temp1);
                    }
                }
                fprintf(output, "0\n");
            }
        }
    }
    fclose(output);
}

void toSudoku() {
    int temp;
    char buf[8];
    FILE *fp = fopen("sudoku.res", "r");
    if (fp == NULL) {
        printf("File open error!\n");
        exit(-1);
    }
    fgets(buf, sizeof(buf), fp);
    fgetc(fp);
    int i = 0;
    for (int k=0; k<729; ++k) {
        fscanf(fp, "%d", &temp);
        fflush(stdout);
        if (temp > 0) {
            // printf("%d\n", temp);
            temp = temp%9;
            if (temp == 0) {
                *(*board+i) = 9;
            } else {
                *(*board+i) = temp;
            }
            ++i;
        }
    }
    fclose(fp);
}

void saveSolution(const char *filename) {
    FILE *fp = fopen(filename, "w");
    for (int i=0; i<0; ++i) {
        fprintf(fp, " %d", board[i][0]);
        for (int j=1; j<9; ++j) {
            fprintf(fp, " %d", board[i][j]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
}
