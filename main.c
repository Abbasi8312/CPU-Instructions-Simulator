#include <stdio.h>
#include <string.h>
#include <ctype.h>

int s[32];
unsigned __int8 status = 0;
char command[12];

int str_case(char *key);
void input(int *arg, int *is_imm);
int processor();
void parity_flag(int num);
void zero_flag(int num);
void sign_flag(int num);
void status_check(int num);
void print_bits (unsigned char num, int bits);

int main() {
    int is_exit = 0;
    while (!is_exit) {
        is_exit = processor();
    }
    return 0;
}

int str_case(char *key) {
    if (strcmp(command, key) == 0)
        return 1;
    else
        return 0;
}

int processor() {
    int arg[3] = {0};
    int is_imm[3] = {1, 1, 1};
    input(arg, is_imm);
    if (str_case("ADD")) {
        s[arg[0]] = s[arg[1]] + s[arg[2]];
        status_check(s[arg[0]]);
    }
    if (str_case("MOV")) {
        if (is_imm[1])
            s[arg[0]] = arg[1];
        else
            s[arg[0]] = s[arg[1]];
    }
    else if (str_case("OUTPUT")) {
        printf("%d\n", s[0]);
    }
    else if(str_case("DUMP_REGS")) {
        for (int i = 0; i < 32; ++i) {
            printf("   S%02d || Decimal: % -11d || Binary:", i, s[i]);
            print_bits(s[i], 32);
        }
        printf("Status || Decimal: % -11d || Binary:",status);
        print_bits(status, 8);
    }
    else if (str_case("EXIT"))
        return 1;
    return 0;
}

void input(int *arg, int *is_imm) {
    scanf("%s", command);
    unsigned char tmp;
    int sign = 1;
    while (1) {
        tmp = getchar();
        if (tmp == ',') {
            arg++;
            is_imm++;
            sign = 1;
        }
        else if (tmp == ' ') {
            continue;
        }
        else if (tmp == 'S') {
            *is_imm = 0;
        }
        else if (tmp == '-')
            sign = -1;
        else if (tmp != '\n') {
            *arg = *arg * 10 + sign * (tmp - '0');
        }
        else
            break;
    }
}

void status_check(int num) {
    status = 0;
    parity_flag(num);
    zero_flag(num);
    sign_flag(num);
}

void parity_flag(int num) {
    int odd = 0;
    for (int i = 0; i < 8; ++i) {
        if (num & 1)
            odd = 1 - odd;
        num >>= 1;
    }
    if (odd)
        status |= 1;
}

void zero_flag(int num) {
    if (!num)
        status |= 2;
}

void sign_flag(int num) {
    if (num < 0)
        status |= 4;
}

void print_bits(unsigned char num, int bits) {
    unsigned char mask = 1 << 7;
    for (int i = 0; i < bits; ++i) {
        if (i % 8 == 0)
            putchar(' ');
        putchar(num & mask ? '1' : '0');
        mask >>= 1;
    }
    putchar('\n');
}