#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define INT_MAX 2147483647
#define INT_MIN -2147483648

int s[32];
unsigned __int8 status = 0;
char command[12];

int str_case(char *key);

void input(int *arg, int *is_imm);

int processor();

void status_check(int result, char operator, int num_1, int num_2);

void print_bits(unsigned int num, int bits);

int main() {
    int is_exit = 0;
    while (!is_exit) {
        is_exit = processor();
    }
    return 0;
}

int str_case(char *key) {
    if (strcmp(command, key) == 0) {
        return 1;
    } else {
        return 0;
    }
}

int processor() {
    int arg[3] = {0};
    int is_imm[3] = {1, 1, 1};
    input(arg, is_imm);
    if (str_case("ADD")) {
        s[arg[0]] = s[arg[1]] + s[arg[2]];
        status_check(s[arg[0]], '+', s[arg[1]], s[arg[2]]);
    }
    if (str_case("MOV")) {
        if (is_imm[1]) {
            s[arg[0]] = arg[1];
        } else {
            s[arg[0]] = s[arg[1]];
        }
    } else if (str_case("OUTPUT")) {
        printf("%d\n", s[0]);
    } else if (str_case("DUMP_REGS")) {
        for (int i = 0; i < 32; ++i) {
            printf("   S%02d || Decimal: % -11d || Binary:", i, s[i]);
            print_bits(s[i], 32);
        }
        printf("Status || Decimal: % -11d || Binary:", status);
        print_bits(status, 8);
    } else if (str_case("EXIT")) {
        return 1;
    }
    return 0;
}

void input(int *arg, int *is_imm) {
    scanf("%s", command);
    for (int i = 0; command[i]; ++i) {
        command[i] = toupper(command[i]);
    }
    unsigned char tmp;
    int sign = 1;
    while (1) {
        tmp = getchar();
        if (tmp == ',') {
            arg++;
            is_imm++;
            sign = 1;
        } else if (tmp == ' ') {
            continue;
        } else if (tmp == 'S' || tmp == 's') {
            *is_imm = 0;
        } else if (tmp == '-') {
            sign = -1;
        } else if (tmp != '\n') {
            *arg = *arg * 10 + sign * (tmp - '0');
        } else {
            break;
        }
    }
}

void status_check(int result, char operator, int num_1, int num_2) {
    status = 0;

    //Parity flag
    int odd = 0;
    for (int i = 0; i < 32; ++i) {
        if (result >> i & 1) {
            odd = 1 - odd;
        }
    }
    if (odd) {
        status |= 1;
    }

    //Zero flag
    if (!result) {
        status |= 2;
    }

    //Sign flag
    if (result < 0) {
        status |= 4;
    }

    //Overflow flag
    int overflow_flag = 0;
    switch (operator) {
        case '+':
            if (num_1 > 0 && num_2 > INT_MAX - num_1 || num_1 < 0 && num_2 < INT_MIN - num_1) {
                overflow_flag = 1;
            }
            break;
        case '-':
            if (num_1 < 0 && num_2 > INT_MAX + num_1 || num_1 > 0 && num_2 < INT_MIN + num_1) {
                overflow_flag = 1;
            }
            break;
        case '*':
            if (num_2 == -1 && num_1 == INT_MIN || num_1 == -1 && num_2 == INT_MIN ||
                num_1 != 0 && num_2 > INT_MAX / num_1 ||
                num_1 != 0 && num_2 < INT_MIN / num_1) {
                overflow_flag = 1;
            }
            break;
        case '/':
            if (num_2 == -1 && num_1 == INT_MIN || num_1 == -1 && num_2 == INT_MIN) {
                overflow_flag = 1;
            }
            break;
    }
    if (overflow_flag) {
        status |= 32;
    }
}

void print_bits(unsigned int num, int bits) {
    unsigned int mask = 1 << (bits - 1);
    for (int i = 0; i < bits; ++i) {
        if (i % 8 == 0) {
            putchar(' ');
        }
        putchar(num & mask ? '1' : '0');
        mask >>= 1;
    }
    putchar('\n');
}
