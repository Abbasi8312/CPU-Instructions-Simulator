#include <stdio.h>
#include <string.h>
#include <ctype.h>

int s[32];
unsigned __int8 status = 0;
char command[12];

int strCase(char *key);
void input(int *arg, int *is_imm);
int processor();
void parityFlag(int num);
void zeroFlag(int num);
void signFlag(int num);
void statusCheck(int num);
void printBits (unsigned char num, int bits);

int main() {
    int is_exit = 0;
    while (!is_exit) {
        is_exit = processor();
    }
    return 0;
}

int strCase(char *key) {
    if (strcmp(command, key) == 0)
        return 1;
    else
        return 0;
}

int processor() {
    int arg[3] = {0};
    int is_imm[3] = {1, 1, 1};
    input(arg, is_imm);
    if (strCase("ADD")) {
        s[arg[0]] = s[arg[1]] + s[arg[2]];
        statusCheck(s[arg[0]]);
    }
    if (strCase("MOV")) {
        if (is_imm[1] == 0)
            s[arg[0]] = s[arg[1]];
        else
            s[arg[0]] = arg[1];
    }
    else if (strCase("OUTPUT")) {
        printf("%d\n", s[0]);
    }
    else if(strCase("DUMP_REGS")) {
        for (int i = 0; i < 32; ++i) {
            printf("S%02d    | Binary: ", i, s[i]);
            printBits(s[i], 32);
            printf(" | Decimal: %d\n", s[i]);
        }
        printf("Status | Binary: ",status);
        printBits(status, 8);
        printf("\t\t\t      | Decimal: %d\n", status);
    }
    else if (strCase("EXIT"))
        return 1;
    return 0;
}

void input(int *arg, int *is_imm) {
    scanf("%s", command);
    unsigned char temp;
    int sign = 1;
    while (1) {
        temp = getchar();
        if (temp == ',') {
            arg++;
            is_imm++;
            sign = 1;
        }
        else if (temp == ' ') {
            continue;
        }
        else if (temp == 'S') {
            *is_imm = 0;
        }
        else if (temp == '-')
            sign = -1;
        else if (temp != '\n') {
            *arg = *arg * 10 + sign * (temp - '0');
        }
        else
            break;
    }
}

void statusCheck(int num) {
    parityFlag(num);
    zeroFlag(num);
    signFlag(num);
}

void parityFlag(int num) {
    int odd = 0;
    for (int i = 0; i < 8; ++i) {
        if (num & 1)
            odd = 1 - odd;
        num >>= 1;
    }
    if (odd)
        status |= 1;
    else
        status &= 0xFE;
}

void zeroFlag(int num) {
    if (num)
        status &= 0xFD;
    else
        status |= 2;
}

void signFlag(int num) {
    if (num >= 0)
        status &= 0xFB;
    else
        status |= 4;
}

void printBits(unsigned char num, int bits) {
    unsigned char mask = 1 << 7;
    for (int i = 0; i < bits; ++i) {
        if (i % 8 == 0)
            putchar(' ');
        putchar(num & mask ? '1' : '0');
        mask >>= 1;
    }
}