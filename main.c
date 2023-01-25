#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define ARG_MAX 3
#define INSTRUCTION_MAX_CHARACTERS 12

int s[32];
unsigned __int8 status = 0;
char command[INSTRUCTION_MAX_CHARACTERS];
int arg_type[ARG_MAX];
int arg_count;
long *line_place = NULL;
int current_line;
int max_line;
char is_overflow = 0;
FILE *output_stream = NULL;

int str_case(char *key);

int get_input_file(int *arg, FILE *input_stream);

int processor(FILE *input_stream);

void status_check(long long int result, char operator, int num_1, int num_2);

void print_bits(unsigned int num, int bits, FILE *stream);

int arg_error(int arg_need_count, const int arg_need_type[]);

int segmentation_error(const int arg[]);

void print_overflow_warning();

int main() {
    FILE *input_stream = NULL;
    int is_exit = 0;
    while (is_exit != 1) {
        if (is_exit == -1 || input_stream == NULL) {
            if (is_exit == -1) {
                printf("Do you want to exit? Y/N\n");
                char mode, tmp;
                while (1) {
                    do {
                        mode = getchar();
                    } while (mode == ' ' || mode == '\n');
                    do {
                        tmp = getchar();
                    } while (tmp == ' ');
                    if (tmp != '\n' || mode != 'n' && mode != 'y' && mode != 'N' && mode != 'Y') {
                        printf("Invalid input! Enter Y/N\n");
                        continue;
                    }
                    break;
                }
                if (mode == 'y' || mode == 'Y') {
                    break;
                }
                printf("Reset all registers to 0? Y/N\n");
                while (1) {
                    do {
                        mode = getchar();
                    } while (mode == ' ' || mode == '\n');
                    do {
                        tmp = getchar();
                    } while (tmp == ' ');
                    if (tmp != '\n' || mode != 'n' && mode != 'y' && mode != 'N' && mode != 'Y') {
                        printf("Invalid input! Enter Y/N\n");
                        continue;
                    }
                    break;
                }
                if (mode == 'y' || mode == 'Y') {
                    for (int i = 0; i < 32; ++i) {
                        s[i] = 0;
                    }
                    status = 0;
                }
                fclose(output_stream);
                output_stream = NULL;
            }
            while (1) {
                char input_name[100];
                printf("Enter input file name:\n");
                fgets(input_name, 99, stdin);
                input_name[strcspn(input_name, "\t\n")] = 0;
                input_stream = fopen(input_name, "r");
                if (input_stream == NULL) {
                    printf("Invalid file name!\n");
                    is_exit = -1;
                    continue;
                }
                break;
            }
            max_line = 1;
            char tmp;
            do {
                tmp = getc(input_stream);
                if (tmp == '\n') {
                    ++max_line;
                }
            } while (tmp != EOF);
            rewind(input_stream);
            if (line_place != NULL)
                free(line_place);
            line_place = (long *) malloc((max_line + 1) * sizeof(long));
            line_place[0] = 0;
            current_line = 1;
            do {
                tmp = getc(input_stream);
                if (tmp == '\n') {
                    line_place[current_line] = ftell(input_stream);
                    ++current_line;
                }
            } while (tmp != EOF);
            line_place[current_line] = ftell(input_stream);
            rewind(input_stream);
            current_line = 0;
        }
        is_exit = processor(input_stream);
    }
    if (fclose(input_stream) != 0) {
        fprintf(stdout, "Error closing input file!");
    }
    if (output_stream != NULL && fclose(output_stream) != 0) {
        fprintf(stdout, "Error closing output file!");
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

int processor(FILE *input_stream) {
    int arg[ARG_MAX] = {0};
    for (int i = 0; i < ARG_MAX; ++i) {
        arg_type[i] = -1;
    }
    is_overflow = 0;
    int is_eof = get_input_file(arg, input_stream);
    while (ftell(input_stream) != line_place[current_line]) {
        ++current_line;
    }
    for (int i = 1; i < INSTRUCTION_MAX_CHARACTERS && command[i] != 0; ++i)
        if (command[i - 1] == '/' && command[i] == '/') {
            command[i - 1] = 0;
            break;
        }
    if (is_eof == 1) {
        printf("Reached the end of file!\n");
        return -1;
    } else if (str_case("ADD")) {
        int imm_need[] = {0, 0, 0};
        if (arg_error(3, imm_need) || segmentation_error(arg))
            return 0;
        int tmp = s[arg[1]] + s[arg[2]];
        status_check(tmp, '+', s[arg[1]], s[arg[2]]);
        s[arg[0]] = tmp;
    } else if (str_case("SUB")) {
        int imm_need[] = {0, 0, 0};
        if (arg_error(3, imm_need) || segmentation_error(arg))
            return 0;
        int tmp = s[arg[1]] - s[arg[2]];
        status_check(tmp, '-', s[arg[1]], s[arg[2]]);
        s[arg[0]] = tmp;
    } else if (str_case("AND")) {
        int imm_need[] = {0, 0, 0};
        if (arg_error(3, imm_need) || segmentation_error(arg))
            return 0;
        s[arg[0]] = s[arg[1]] & s[arg[2]];
        status_check(s[arg[0]], '&', s[arg[1]], s[arg[2]]);
    } else if (str_case("XOR")) {
        int imm_need[] = {0, 0, 0};
        if (arg_error(3, imm_need) || segmentation_error(arg))
            return 0;
        s[arg[0]] = s[arg[1]] ^ s[arg[2]];
        status_check(s[arg[0]], '^', s[arg[1]], s[arg[2]]);
    } else if (str_case("OR")) {
        int imm_need[] = {0, 0, 0};
        if (arg_error(3, imm_need) || segmentation_error(arg))
            return 0;
        s[arg[0]] = s[arg[1]] | s[arg[2]];
        status_check(s[arg[0]], '|', s[arg[1]], s[arg[2]]);
    } else if (str_case("ADDI")) {
        int imm_need[] = {0, 0, 1};
        if (arg_error(3, imm_need) || segmentation_error(arg))
            return 0;
        int tmp = s[arg[1]] + arg[2];
        status_check(tmp, '+', s[arg[1]], arg[2]);
        s[arg[0]] = tmp;
    } else if (str_case("SUBI")) {
        int imm_need[] = {0, 0, 1};
        if (arg_error(3, imm_need) || segmentation_error(arg))
            return 0;
        int tmp = s[arg[1]] - arg[2];
        status_check(tmp, '-', s[arg[1]], arg[2]);
        s[arg[0]] = tmp;
    } else if (str_case("ANDI")) {
        int imm_need[] = {0, 0, 1};
        if (arg_error(3, imm_need) || segmentation_error(arg))
            return 0;
        s[arg[0]] = s[arg[1]] & arg[2];
        status_check(s[arg[0]], '&', s[arg[1]], arg[2]);
    } else if (str_case("XORI")) {
        int imm_need[] = {0, 0, 1};
        if (arg_error(3, imm_need) || segmentation_error(arg))
            return 0;
        s[arg[0]] = s[arg[1]] ^ arg[2];
        status_check(s[arg[0]], '^', s[arg[1]], arg[2]);
    } else if (str_case("ORI")) {
        int imm_need[] = {0, 0, 1};
        if (arg_error(3, imm_need) || segmentation_error(arg))
            return 0;
        s[arg[0]] = s[arg[1]] | arg[2];
        status_check(s[arg[0]], '|', s[arg[1]], arg[2]);
    } else if (str_case("MOV")) {
        int imm_need[] = {0, -1};
        if (arg_error(2, imm_need) || segmentation_error(arg))
            return 0;
        if (arg_type[1]) {
            s[arg[0]] = arg[1];
        } else {
            s[arg[0]] = s[arg[1]];
        }
    } else if (str_case("SWP")) {
        int imm_need[] = {0, 0};
        if (arg_error(2, imm_need) || segmentation_error(arg))
            return 0;
        int tmp = s[arg[0]];
        s[arg[0]] = s[arg[1]];
        s[arg[1]] = tmp;
    } else if (str_case("DUMP_REGS")) {
        int imm_need[] = {-1};
        if (arg_error(0, imm_need)) {
            return 0;
        }
        for (int i = 0; i < 32; ++i) {
            fprintf(stdout, "   S%02d || Decimal: % -11d || Binary:", i, s[i]);
            print_bits(s[i], 32, stdout);
        }
        fprintf(stdout, "Status || Decimal: % -11d || Binary:", status);
        for (int i = 0; i < 27; ++i)
            putc(' ', stdout);
        print_bits(status, 8, stdout);
    } else if (str_case("DUMP_REGS_F")) {
        int imm_need[] = {-1};
        if (arg_error(0, imm_need)) {
            return 0;
        }
        if (output_stream == NULL || ferror(output_stream)) {
            if (ferror(output_stream)) {
                printf("Can't access output file!\n");
                clearerr(output_stream);
            }
            while (1) {
                char output_name[100];
                char tmp;
                printf("Enter output file name:\n");
                fgets(output_name, 99, stdin);
                output_name[strcspn(output_name, "\t\n")] = 0;
                output_stream = fopen(output_name, "r");
                if (output_stream != NULL) {
                    printf("%s already exists! append, overwrite or enter a new name? a/w/c\n", output_name);
                    char mode;
                    while (1) {
                        do {
                            mode = getchar();
                        } while (mode == ' ' || mode == '\n');
                        do {
                            tmp = getchar();
                        } while (tmp == ' ');
                        if (tmp != '\n' || mode != 'a' && mode != 'w' && mode != 'c') {
                            printf("Invalid input! Enter a:append / w:overwrite / c:cancel\n");
                            continue;
                        }
                        break;
                    }
                    if (mode == 'c')
                        continue;
                    else if (mode == 'a') {
                        output_stream = fopen(output_name, "a");
                        fputc('\n', output_stream);
                    } else if (mode == 'w')
                        output_stream = fopen(output_name, "w");
                } else
                    output_stream = fopen(output_name, "w");
                if (output_stream == NULL) {
                    printf("Invalid file name!\n");
                    continue;
                }
                break;
            }
        }
        for (int i = 0; i < 32; ++i) {
            fprintf(output_stream, "   S%02d || Decimal: % -11d || Binary:", i, s[i]);
            print_bits(s[i], 32, output_stream);
        }
        fprintf(output_stream, "Status || Decimal: % -11d || Binary:", status);
        for (int i = 0; i < 27; ++i)
            putc(' ', output_stream);
        print_bits(status, 8, output_stream);
        fputc('\n', output_stream);
    } else if (str_case("INPUT")) {
        int imm_need[] = {-1};
        if (arg_error(0, imm_need)) {
            return 0;
        }
        while (1) {
            s[0] = 0;
            printf("Enter a number:\n");
            char tmp, sign = 1;
            do {
                tmp = getchar();
            } while (tmp == ' ' || tmp == '\t');
            if (tmp == '-') {
                sign = -1;
                tmp = getchar();
            } else if (tmp == '+') {
                tmp = getchar();
            }
            if (isdigit(tmp)) {
                int index = 0;
                char num[11] = {0};
                while (isdigit(tmp)) {
                    if (index < 11)
                        num[index] = tmp;
                    s[0] = s[0] * 10 + sign * (tmp - '0');
                    tmp = getchar();
                    ++index;
                }
                if (sign > 0 && strcmp(num, "2147483648") == 0 || index == 10 && strcmp(num, "2147483648") > 0 ||
                    index > 10)
                    print_overflow_warning();
                while (tmp == ' ' || tmp == '\t')
                    tmp = getchar();
                if (tmp == '\n')
                    break;
            }
            printf("Invalid input!\n");
            while (tmp != '\n')
                tmp = getchar();
        }
    } else if (str_case("OUTPUT")) {
        int imm_need[] = {-1};
        if (arg_error(0, imm_need)) {
            return 0;
        }
        printf("%d\n", s[0]);
    } else if (str_case("JMP")) {
        int imm_need[] = {1};
        if (arg_error(1, imm_need) || segmentation_error(arg))
            return 0;
        if (arg[0] > max_line || arg[0] <= 0) {
            printf("Line:%-3d| Out of range error!\n\tLine %d doesn't exist\n", current_line, arg[0]);
            return 0;
        }
        current_line = arg[0] - 1;
        fseek(input_stream, line_place[current_line], SEEK_SET);
        return 0;
    } else if (str_case("MULL")) {
        int imm_need[] = {0, 0};
        if (arg_error(2, imm_need) || segmentation_error(arg))
            return 0;
        long long int tmp = (long long int) s[arg[0]] * (long long int) s[arg[1]];
        status_check(tmp, '*', s[arg[0]], s[arg[1]]);
        s[arg[0]] = tmp >> 32;
        s[arg[1]] = tmp;
    } else if (str_case("DIV")) {
        int imm_need[] = {0, 0};
        if (arg_error(2, imm_need) || segmentation_error(arg))
            return 0;
        if (s[arg[1]] == 0) {
            printf("Error! Division by zero\n");
            return 0;
        } else if (s[arg[0]] == -2147483648 && s[arg[1]] == -1) {
            s[arg[1]] = 0;
            s[arg[0]] = -2147483648;
            print_overflow_warning();
        } else {
            int tmp = s[arg[0]] / s[arg[1]];
            status_check(tmp, '/', s[arg[0]], s[arg[1]]);
            s[arg[1]] = s[arg[0]] % s[arg[1]];
            s[arg[0]] = tmp;
        }
    } else if (str_case("SKIE")) {
        int imm_need[] = {0, 0};
        if (arg_error(2, imm_need) || segmentation_error(arg))
            return 0;
        else if (current_line == max_line) {
            printf("Line:%-3d| Out of range error!\n\tLine %d doesn't exist\n", current_line, max_line + 1);
        } else if (s[arg[0]] == s[arg[1]]) {
            ++current_line;
            fseek(input_stream, line_place[current_line], SEEK_SET);
        }
    } else if (str_case("EXIT")) {
        int imm_need[] = {-1};
        if (arg_error(0, imm_need)) {
            return 0;
        }
        return 1;
    } else if (command[0] == 0) {
        return 0;
    } else {
        printf("Line:%-3d| Syntax error!\n\tUnknown instruction %s\n", current_line, command);
        return 0;
    }
    if (is_overflow)
        print_overflow_warning();
    if (is_eof == 2) {
        printf("Reached the end of file!\n");
        return -1;
    }
    return 0;
}

int get_input_file(int *arg, FILE *input_stream) {
    arg_count = 0;
    if (fscanf(input_stream, "%s", command) == EOF)
        return 1;
    for (int i = 0; command[i]; ++i) {
        command[i] = toupper(command[i]);
    }
    int tmp;
    int sign = 1;
    while (arg_count < ARG_MAX) {
        tmp = getc(input_stream);
        while (tmp == ' ' || tmp == '\t')
            tmp = getc(input_stream);
        if (tmp == '/') {
            tmp = getc(input_stream);
            if (tmp == '/') {
                if (arg_count) {
                    arg_type[arg_count] = 2;
                    ++arg_count;
                }
                while (1) {
                    tmp = getc(input_stream);
                    if (tmp == '\n')
                        return 0;
                    else if (tmp == EOF)
                        return 2;
                }
            } else {
                arg_type[arg_count] = 2;
                while (tmp == ' ')
                    tmp = getc(input_stream);
            }
        }
        if (tmp == '\n') {
            if (arg_count || !arg_count && arg_type[0] == 2) {
                arg_type[arg_count] = 2;
                ++arg_count;
            }
            return 0;
        } else if (tmp == EOF) {
            if (arg_count || !arg_count && arg_type[0] == 2) {
                arg_type[arg_count] = 2;
                ++arg_count;
            }
            return 2;
        }
        if (tmp == 'S' || tmp == 's') {
            if (arg_type[arg_count] == -1)
                arg_type[arg_count] = 0;
            tmp = getc(input_stream);
        } else if (tmp != '-' && tmp != '+' && !isdigit(tmp)) {
            arg_type[arg_count] = 2;
            ++arg_count;
            while (tmp != ',') {
                tmp = getc(input_stream);
                if (tmp == '/') {
                    tmp = getc(input_stream);
                    if (tmp == '/') {
                        while (1) {
                            tmp = getc(input_stream);
                            if (tmp == '\n')
                                return 0;
                            else if (tmp == EOF)
                                return 2;
                        }
                    }
                }
                if (tmp == '\n')
                    return 0;
                else if (tmp == EOF)
                    return 2;
            }
            continue;
        }
        if (tmp == '-') {
            sign = -1;
            tmp = getc(input_stream);
        } else if (tmp == '+') {
            tmp = getc(input_stream);
        } else if (!isdigit(tmp)) {
            arg_type[arg_count] = 2;
            ++arg_count;
            while (tmp != ',') {
                if (tmp == '/') {
                    tmp = getc(input_stream);
                    if (tmp == '/') {
                        while (1) {
                            tmp = getc(input_stream);
                            if (tmp == '\n')
                                return 0;
                            else if (tmp == EOF)
                                return 2;
                        }
                    }
                }
                if (tmp == '\n')
                    return 0;
                else if (tmp == EOF)
                    return 2;
                tmp = getc(input_stream);
            }
            continue;
        }
        if (isdigit(tmp)) {
            int index = 0;
            char num[11] = {0};
            if (arg_type[arg_count] == -1) {
                arg_type[arg_count] = 1;
            }
            while (isdigit(tmp)) {
                if (index < 11)
                    num[index] = tmp;
                arg[arg_count] = arg[arg_count] * 10 + sign * (tmp - '0');
                tmp = getc(input_stream);
                ++index;
            }
            if (sign > 0 && strcmp(num, "2147483648") == 0 || index == 10 && strcmp(num, "2147483648") > 0 ||
                index > 10)
                is_overflow = 1;
            ++arg_count;
            while (tmp == ' ' || tmp == '\t')
                tmp = getc(input_stream);
            if (tmp == '\n')
                return 0;
            else if (tmp == EOF)
                return 2;
            else if (tmp != ',') {
                if (tmp == '/') {
                    tmp = getc(input_stream);
                    if (tmp == '/') {
                        while (1) {
                            tmp = getc(input_stream);
                            if (tmp == '\n')
                                return 0;
                            else if (tmp == EOF)
                                return 2;
                        }
                    }
                }
                arg_type[arg_count - 1] = 2;
                while (tmp != ',') {
                    if (tmp == '\n')
                        return 0;
                    else if (tmp == EOF)
                        return 2;
                    tmp = getc(input_stream);
                }
            }
            continue;
        } else {
            arg_type[arg_count] = 2;
            ++arg_count;
            while (tmp != ',') {
                if (tmp == '/') {
                    tmp = getc(input_stream);
                    if (tmp == '/') {
                        while (1) {
                            tmp = getc(input_stream);
                            if (tmp == '\n')
                                return 0;
                            else if (tmp == EOF)
                                return 2;
                        }
                    }
                }
                if (tmp == '\n')
                    return 0;
                else if (tmp == EOF)
                    return 2;
                tmp = getc(input_stream);
            }
            continue;
        }
    }
    while (tmp != '\n' && tmp != EOF) {
        if (tmp == ',') {
            ++arg_count;
        }
        tmp = getc(input_stream);
    }
    return 0;
}

void status_check(long long int result, char operator, int num_1, int num_2) {
    status = 0;
    int bit_count = operator == '*' ? 64 : 32;
    //Parity flag
    int odd = 0;
    for (int i = 0; i < bit_count; ++i) {
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
    }
    if (overflow_flag) {
        status |= 32;
        print_overflow_warning();
    }
}

void print_bits(unsigned int num, int bits, FILE *stream) {
    unsigned int mask = 1 << (bits - 1);
    for (int i = 0; i < bits; ++i) {
        if (i % 8 == 0) {
            putc(' ', stream);
        }
        putc(num & mask ? '1' : '0', stream);
        mask >>= 1;
    }
    putc('\n', stream);
}

int arg_error(const int arg_need_count, const int arg_need_type[]) {
    int index, flag_invalid_input = 0;
    for (index = 0; index < arg_count && index < arg_need_count; ++index) {
        if ((arg_need_type[index] != arg_type[index] && arg_need_type[index] != -1) || arg_type[index] == 2) {
            if (flag_invalid_input == 0) {
                flag_invalid_input = 1;
                printf("Line:%-3d| Syntax error!\n", current_line);
            }
            printf("\tArgument %d is invalid\n", index + 1);
        }
    }
    if (arg_need_count != arg_count) {
        if (flag_invalid_input == 0) {
            flag_invalid_input = 1;
            printf("Line:%-3d| Syntax error!\n", current_line);
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
            } else if (arg_need_type[i] == 1) {
                printf("Imm");
            } else {
                printf("S%c/Imm", 'a' + i);
            }
        }
        putchar('\n');
        return 1;
    }
    return 0;
}

int segmentation_error(const int *arg) {
    int index, flag_invalid_input = 0;
    for (index = 0; index < arg_count; ++index) {
        if (arg_type[index] == 0 && (arg[index] < 0 || arg[index] >= 32)) {
            if (flag_invalid_input == 0) {
                flag_invalid_input = 1;
                printf("Line:%-3d| Segmentation error!\n", current_line);
            }
            printf("\tArgument %d is invalid\n", index + 1);
        }
    }
    if (flag_invalid_input == 1)
        return 1;
    return 0;
}

void print_overflow_warning() {
    printf("Line:%-3d| Overflow warning!\n", current_line);
}
