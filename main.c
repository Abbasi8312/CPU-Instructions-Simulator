#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define INT_MAX 2147483647
#define INT_MIN (-2147483648)
#define ARG_MAX 3

int s[32];
unsigned __int8 status = 0;
char command[12];
int arg_type[ARG_MAX];
int arg_count;
int flag_invalid_input;

int str_case(char *key);

int get_input(int *arg, FILE *stream);

int processor(FILE *stream);

void status_check(int result, char operator, int num_1, int num_2);

void print_bits(unsigned int num, int bits);

int arg_error(int arg_need_count, const int arg_need_type[]);

void instruction_error();

int main() {
    FILE *stream = fopen("input.txt", "r");
    int is_exit = 0;
    while (!is_exit) {
        is_exit = processor(stream);
    }
    fclose(stream);
    return 0;
}

int str_case(char *key) {
    if (strcmp(command, key) == 0) {
        return 1;
    } else {
        return 0;
    }
}

int processor(FILE *stream) {
    flag_invalid_input = 0;
    int arg[ARG_MAX] = {0};
    for (int i = 0; i < ARG_MAX; ++i) {
        arg_type[i] = -1;
    }
    int is_eof = get_input(arg, stream);
    printf("%s %d %d %d count:%d \n", command, arg_type[0], arg_type[1], arg_type[2], arg_count);
    if (is_eof == 1) {
        return 1;
    }
    if (str_case("ADD")) {
        int imm_need[] = {0, 0, 0};
        if (arg_error(3, imm_need)) {
            return 0;
        }
        s[arg[0]] = s[arg[1]] + s[arg[2]];
        status_check(s[arg[0]], '+', s[arg[1]], s[arg[2]]);
    }
    else if (str_case("MOV")) {
        int imm_need[] = {0, -1};
        if (arg_error(2, imm_need)) {
            return 0;
        }
        if (arg_type[1]) {
            s[arg[0]] = arg[1];
        } else {
            s[arg[0]] = s[arg[1]];
        }
    } else if (str_case("OUTPUT")) {
        int imm_need[] = {-1};
        if (arg_error(0, imm_need)) {
            return 0;
        }
        printf("%d\n", s[0]);
    } else if (str_case("DUMP_REGS")) {
        int imm_need[] = {-1};
        if (arg_error(0, imm_need)) {
            return 0;
        }
        for (int i = 0; i < 32; ++i) {
            printf("   S%02d || Decimal: % -11d || Binary:", i, s[i]);
            print_bits(s[i], 32);
        }
        printf("Status || Decimal: % -11d || Binary:", status);
        print_bits(status, 8);
    } else if (str_case("EXIT")) {
        int imm_need[] = {-1};
        if (arg_error(0, imm_need)) {
            return 0;
        }
        return 1;
    } else {
        instruction_error();
    }
    if (is_eof == 2) {
        return 1;
    }
    return 0;
}

int get_input(int *arg, FILE *stream) {
    arg_count = 0;
    if (fscanf(stream, "%s", command) == EOF)
        return 1;
    for (int i = 0; command[i]; ++i) {
        command[i] = toupper(command[i]);
    }
    char tmp;
    int sign = 1;
    while (arg_count < ARG_MAX) {
        tmp = getc(stream);
        while (tmp == ' ')
            tmp = getc(stream);
        if (tmp == '\n') {
            if (arg_count) {
                arg_type[arg_count] = 2;
                ++arg_count;
            }
            return 0;
        }
        if (tmp == EOF) {
            if (arg_count) {
                arg_type[arg_count] = 2;
                ++arg_count;
            }
            return 2;
        }
        if (tmp == 'S' || tmp == 's') {
            arg_type[arg_count] = 0;
            tmp = getc(stream);
        } else if (tmp != '-' && !isdigit(tmp)) {
            arg_type[arg_count] = 2;
            ++arg_count;
            while (tmp != ',') {
                tmp = getc(stream);
                if (tmp == '\n')
                    return 0;
                else if (tmp == EOF)
                    return 2;
            }
            continue;
        }
        if (tmp == '-') {
            sign = -1;
            tmp = getc(stream);
        } else if (!isdigit(tmp)) {
            arg_type[arg_count] = 2;
            ++arg_count;
            while (tmp != ',') {
                if (tmp == '\n')
                    return 0;
                else if (tmp == EOF)
                    return 2;
                tmp = getc(stream);
            }
            continue;
        }
        if (isdigit(tmp)) {
            if (arg_type[arg_count] == -1) {
                arg_type[arg_count] = 1;
            }
            while (isdigit(tmp)) {
                arg[arg_count] = arg[arg_count] * 10 + sign * (tmp - '0');
                tmp = getc(stream);
            }
            ++arg_count;
            while (tmp == ' ')
                tmp = getc(stream);
            if (tmp == '\n')
                return 0;
            else if (tmp == EOF)
                return 2;
            else if (tmp != ',') {
                arg_type[arg_count - 1] = 2;
                while (tmp != ',') {
                    if (tmp == '\n')
                        return 0;
                    else if (tmp == EOF)
                        return 2;
                    tmp = getc(stream);
                }
            }
            continue;
        } else {
            arg_type[arg_count] = 2;
            ++arg_count;
            while (tmp != ',') {
                if (tmp == '\n')
                    return 0;
                else if (tmp == EOF)
                    return 2;
                tmp = getc(stream);
            }
            continue;
        }
        ++arg_count;
        while (tmp != ',') {
            if (tmp == '\n') {
                return 0;
            }
            else if (tmp == EOF) {
                return 2;
            }
            tmp = getc(stream);
        }
    }
    while (tmp != '\n' && tmp != EOF) {
        if (tmp == ',') {
            ++arg_count;
        }
        tmp = getc(stream);
    }
    return 0;
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

int arg_error(const int arg_need_count, const int arg_need_type[]) {
    int index;
    for (index = 0; index < arg_count && index < arg_need_count; ++index) {
        if ((arg_need_type[index] != arg_type[index] && arg_need_type[index] != -1) || arg_type[index] == 2) {
            if (flag_invalid_input == 0) {
                flag_invalid_input = 1;
                printf("Error! | Line: \n");
            }
            printf("\targument %d is invalid\n", index + 1);
        }
    }
    if (arg_need_count != arg_count) {
        if (flag_invalid_input == 0) {
            flag_invalid_input = 1;
            printf("Error! | Line: \n");
        }
        printf("\t%s instruction needs %d argument", command, arg_need_count);
        if (arg_need_count != 1) {
            putchar('s');
        }
        printf(" but you entered %d argument", arg_count);
        if (arg_count != 1) {
            putchar('s');
        }
        printf("\n");
    }
    if (flag_invalid_input) {
        printf("\tCorrect format: %s ", command);
        for (int i = 0; i < arg_need_count; ++i) {
            if (i) {
                printf(", ");
            }
            if (arg_need_type[i] == 0) {
                printf("S%c", 'a' + i);
            } else if (arg_need_type[i] == 1){
                printf("Imm");
            } else {
                printf("S%c/Imm", 'a' + i);
            }
        }
        putchar('\n');
        putchar('\n');
        return 1;
    }
    return 0;
}

void instruction_error() {
    printf("Error! | Line: \n\tUnknown instruction %s\n", command);
}