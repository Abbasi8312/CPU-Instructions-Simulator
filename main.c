#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define ARG_MAX_COUNT 3
#define INSTRUCTION_MAX_CHARACTERS 12

FILE *input_stream = NULL;
FILE *output_stream = NULL;
long *line_place = NULL;
int current_line, max_line;
int reg[32];
unsigned __int8 status = 0;
char instruction[INSTRUCTION_MAX_CHARACTERS];
int arg_type[ARG_MAX_COUNT];
int arg_count;
int is_overflow = 0;
int *stack;
int stack_index = -1, stack_max = 10;
int suggestion_status = 1;

int get_input_file_name();

void open_input_file();

void reset();

int processor();

int get_input_from_file(int *arg);

int str_case(char *key);

void status_check(long long int result, char operator, int num_1, int num_2);

void open_output_file();

void dump_regs(FILE *stream);

void print_bits(unsigned int num, int bits, FILE *stream);

int arg_error(const int arg[], int arg_need_count, const int arg_need_type[]);

int segmentation_error(const int arg[]);

void print_overflow_warning();

int yes_no(int cancel);

int suggest(const int arg[], int arg_need_count, const int arg_need_type[]);

int main() {
    stack = (int *) malloc(stack_max * sizeof(int));
    int is_exit = 0;
    while (is_exit != 1) {
        if (is_exit < 0 || input_stream == NULL) {
            if (is_exit != 0) {
                printf("Do you want to exit? Y/N\n");
                if (yes_no(0))
                    break;
                if (is_exit == -1)
                    reset();
            }
            if (get_input_file_name()) {
                is_exit = -2;
                continue;
            }
            open_input_file();
        }
        is_exit = processor(input_stream);
    }
    fclose(input_stream);
    fclose(output_stream);
    return 0;
}

int processor() {
    int arg[ARG_MAX_COUNT] = {0};
    for (int i = 0; i < ARG_MAX_COUNT; ++i)
        arg_type[i] = -1;
    is_overflow = 0;
    int is_eof = get_input_from_file(arg);
    while (ftell(input_stream) != line_place[current_line])
        ++current_line;
    for (int i = 1; i < INSTRUCTION_MAX_CHARACTERS && instruction[i] != 0; ++i) {
        if (instruction[i - 1] == '/' && instruction[i] == '/') {
            instruction[i - 1] = 0;
            break;
        }
    }
    if (is_eof == 1) {
        printf("Reached the end of file!\n");
        return -1;
    } else if (str_case("ADD")) {
        int arg_need_type[] = {0, 0, 0};
        if (arg_error(arg, 3, arg_need_type) || segmentation_error(arg))
            return 0;
        int tmp = reg[arg[1]] + reg[arg[2]];
        status_check(tmp, '+', reg[arg[1]], reg[arg[2]]);
        reg[arg[0]] = tmp;
    } else if (str_case("SUB")) {
        int arg_need_type[] = {0, 0, 0};
        if (arg_error(arg, 3, arg_need_type) || segmentation_error(arg))
            return 0;
        int tmp = reg[arg[1]] - reg[arg[2]];
        status_check(tmp, '-', reg[arg[1]], reg[arg[2]]);
        reg[arg[0]] = tmp;
    } else if (str_case("AND")) {
        int arg_need_type[] = {0, 0, 0};
        if (arg_error(arg, 3, arg_need_type) || segmentation_error(arg))
            return 0;
        reg[arg[0]] = reg[arg[1]] & reg[arg[2]];
        status_check(reg[arg[0]], '&', reg[arg[1]], reg[arg[2]]);
    } else if (str_case("XOR")) {
        int arg_need_type[] = {0, 0, 0};
        if (arg_error(arg, 3, arg_need_type) || segmentation_error(arg))
            return 0;
        reg[arg[0]] = reg[arg[1]] ^ reg[arg[2]];
        status_check(reg[arg[0]], '^', reg[arg[1]], reg[arg[2]]);
    } else if (str_case("OR")) {
        int arg_need_type[] = {0, 0, 0};
        if (arg_error(arg, 3, arg_need_type) || segmentation_error(arg))
            return 0;
        reg[arg[0]] = reg[arg[1]] | reg[arg[2]];
        status_check(reg[arg[0]], '|', reg[arg[1]], reg[arg[2]]);
    } else if (str_case("ADDI")) {
        int arg_need_type[] = {0, 0, 1};
        if (arg_error(arg, 3, arg_need_type) || segmentation_error(arg))
            return 0;
        int tmp = reg[arg[1]] + arg[2];
        status_check(tmp, '+', reg[arg[1]], arg[2]);
        reg[arg[0]] = tmp;
    } else if (str_case("SUBI")) {
        int arg_need_type[] = {0, 0, 1};
        if (arg_error(arg, 3, arg_need_type) || segmentation_error(arg))
            return 0;
        int tmp = reg[arg[1]] - arg[2];
        status_check(tmp, '-', reg[arg[1]], arg[2]);
        reg[arg[0]] = tmp;
    } else if (str_case("ANDI")) {
        int arg_need_type[] = {0, 0, 1};
        if (arg_error(arg, 3, arg_need_type) || segmentation_error(arg))
            return 0;
        reg[arg[0]] = reg[arg[1]] & arg[2];
        status_check(reg[arg[0]], '&', reg[arg[1]], arg[2]);
    } else if (str_case("XORI")) {
        int arg_need_type[] = {0, 0, 1};
        if (arg_error(arg, 3, arg_need_type) || segmentation_error(arg))
            return 0;
        reg[arg[0]] = reg[arg[1]] ^ arg[2];
        status_check(reg[arg[0]], '^', reg[arg[1]], arg[2]);
    } else if (str_case("ORI")) {
        int arg_need_type[] = {0, 0, 1};
        if (arg_error(arg, 3, arg_need_type) || segmentation_error(arg))
            return 0;
        reg[arg[0]] = reg[arg[1]] | arg[2];
        status_check(reg[arg[0]], '|', reg[arg[1]], arg[2]);
    } else if (str_case("MOV")) {
        int arg_need_type[] = {0, -1};
        if (arg_error(arg, 2, arg_need_type) || segmentation_error(arg))
            return 0;
        if (arg_type[1]) {
            reg[arg[0]] = arg[1];
        } else {
            reg[arg[0]] = reg[arg[1]];
        }
    } else if (str_case("SWP")) {
        int arg_need_type[] = {0, 0};
        if (arg_error(arg, 2, arg_need_type) || segmentation_error(arg))
            return 0;
        int tmp = reg[arg[0]];
        reg[arg[0]] = reg[arg[1]];
        reg[arg[1]] = tmp;
    } else if (str_case("DUMP_REGS")) {
        int arg_need_type[] = {-1};
        if (arg_error(arg, 0, arg_need_type))
            return 0;
        dump_regs(stdout);
    } else if (str_case("DUMP_REGS_F")) {
        int arg_need_type[] = {-1};
        if (arg_error(arg, 0, arg_need_type))
            return 0;
        open_output_file();
        dump_regs(output_stream);
    } else if (str_case("INPUT")) {
        int arg_need_type[] = {-1};
        if (arg_error(arg, 0, arg_need_type))
            return 0;
        while (1) {
            reg[0] = 0;
            printf("Enter a number:\n");
            int tmp, sign = 1;
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
                    reg[0] = reg[0] * 10 + sign * (tmp - '0');
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
        int arg_need_type[] = {-1};
        if (arg_error(arg, 0, arg_need_type))
            return 0;
        printf("%d\n", reg[0]);
    } else if (str_case("JMP")) {
        int arg_need_type[] = {1};
        if (arg_error(arg, 1, arg_need_type) || segmentation_error(arg))
            return 0;
        if (arg[0] > max_line || arg[0] <= 0) {
            printf("Line:%-3d| Out of range error!\n\tLine %d doesn't exist\n", current_line, arg[0]);
            return 0;
        }
        current_line = arg[0] - 1;
        fseek(input_stream, line_place[current_line], SEEK_SET);
        return 0;
    } else if (str_case("MULL")) {
        int arg_need_type[] = {0, 0};
        if (arg_error(arg, 2, arg_need_type) || segmentation_error(arg))
            return 0;
        long long int tmp = (long long int) reg[arg[0]] * (long long int) reg[arg[1]];
        status_check(tmp, '*', reg[arg[0]], reg[arg[1]]);
        reg[arg[0]] = (int) (tmp >> 32);
        reg[arg[1]] = (int) tmp;
    } else if (str_case("DIV")) {
        int arg_need_type[] = {0, 0};
        if (arg_error(arg, 2, arg_need_type) || segmentation_error(arg))
            return 0;
        if (reg[arg[1]] == 0) {
            printf("Error! Division by zero\n");
        } else if (reg[arg[0]] == -2147483648 && reg[arg[1]] == -1) {
            reg[arg[1]] = 0;
            reg[arg[0]] = -2147483648;
            print_overflow_warning();
        } else {
            int tmp = reg[arg[0]] / reg[arg[1]];
            status_check(tmp, '/', reg[arg[0]], reg[arg[1]]);
            reg[arg[1]] = reg[arg[0]] % reg[arg[1]];
            reg[arg[0]] = tmp;
        }
    } else if (str_case("SKIE")) {
        int arg_need_type[] = {0, 0};
        if (arg_error(arg, 2, arg_need_type) || segmentation_error(arg))
            return 0;
        else if (current_line == max_line && reg[arg[0]] == reg[arg[1]]) {
            printf("Line:%-3d| Out of range error!\n\tLine %d doesn't exist\n", current_line, max_line + 1);
        } else if (reg[arg[0]] == reg[arg[1]]) {
            ++current_line;
            fseek(input_stream, line_place[current_line], SEEK_SET);
        }
    } else if (str_case("PUSH")) {
        int arg_need_type[] = {0};
        if (arg_error(arg, 1, arg_need_type) || segmentation_error(arg))
            return 0;
        ++stack_index;
        if (stack_index == stack_max) {
            stack_max += 10;
            stack = realloc(stack, stack_max * sizeof(int));
        }
        stack[stack_index] = reg[arg[0]];
    } else if (str_case("POP")) {
        int arg_need_type[] = {0};
        if (arg_error(arg, 1, arg_need_type) || segmentation_error(arg))
            return 0;
        if (stack_index == -1) {
            printf("Line:%-3d| Stack error!\n\tStack is empty\n", current_line);
            return 0;
        }
        reg[arg[0]] = stack[stack_index];
        --stack_index;
    } else if (str_case("EXIT")) {
        int arg_need_type[] = {-1};
        if (arg_error(arg, 0, arg_need_type))
            return 0;
        return 1;
    } else if (instruction[0] == 0) {
        return 0;
    } else {
        printf("Line:%-3d| Syntax error!\n\tUnknown instruction %s\n", current_line, instruction);
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

int get_input_from_file(int *arg) {
    arg_count = 0;
    if (fscanf(input_stream, "%s", instruction) == EOF)
        return 1;
    for (int i = 0; instruction[i]; ++i)
        instruction[i] = toupper(instruction[i]);
    int tmp, sign = 1;
    while (arg_count < ARG_MAX_COUNT) {
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

int str_case(char *key) {
    if (strcmp(instruction, key) == 0) {
        return 1;
    } else {
        return 0;
    }
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
        if (i % 8 == 0)
            putc(' ', stream);
        putc(num & mask ? '1' : '0', stream);
        mask >>= 1;
    }
    putc('\n', stream);
}

int arg_error(const int arg[], int arg_need_count, const int arg_need_type[]) {
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
        printf("\t%s instruction needs %d argument", instruction, arg_need_count);
        if (arg_need_count != 1)
            putchar('s');
        printf(" but you entered %d argument", arg_count);
        if (arg_count != 1)
            putchar('s');
        printf("\n");
    }
    if (flag_invalid_input) {
        printf("\tCorrect format: %s ", instruction);
        for (int i = 0; i < arg_need_count; ++i) {
            if (i)
                printf(", ");
            if (arg_need_type[i] == 0) {
                printf("S%c", 'a' + i);
            } else if (arg_need_type[i] == 1) {
                printf("Imm");
            } else {
                printf("S%c/Imm", 'a' + i);
            }
        }
        putchar('\n');
        if (suggest(arg, arg_need_count, arg_need_type))
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
            printf("\tArgument %d is invalid (Correct argument range: S0-S31)\n", index + 1);
        }
    }
    if (flag_invalid_input == 1)
        return 1;
    return 0;
}

void print_overflow_warning() {
    printf("Line:%-3d| Overflow warning!\n", current_line);
}

void reset() {
    printf("Reset stack and registers? Y/N\n");
    if (yes_no(0)) {
        for (int i = 0; i < 32; ++i) {
            reg[i] = 0;
        }
        status = 0;
        free(stack);
        stack_max = 10;
        stack_index = -1;
        stack = (int *) malloc(stack_max * sizeof(int));
    }
    fclose(output_stream);
    output_stream = NULL;
    suggestion_status = 1;
}

int get_input_file_name() {
    char *input_name = (char *) malloc(100 * sizeof(char));
    printf("Enter input file name:\n");
    fgets(input_name, 99, stdin);
    while (input_name != 0 && (*input_name == ' ' || *input_name == '\t'))
        ++input_name;
    input_name[strcspn(input_name, "\t\n")] = 0;
    input_stream = fopen(input_name, "r");
    if (input_stream == NULL) {
        printf("Invalid file name!\n");
        return 1;
    }
    return 0;
}

void open_input_file() {
    max_line = 1;
    int tmp;
    do {
        tmp = getc(input_stream);
        if (tmp == '\n')
            ++max_line;
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

void open_output_file() {
    if (output_stream == NULL || ferror(output_stream)) {
        if (ferror(output_stream)) {
            printf("Can't access output file!\n");
            clearerr(output_stream);
        }
        while (1) {
            char *output_name = (char *) malloc(100 * sizeof(char));
            printf("Enter output file name:\n");
            fgets(output_name, 99, stdin);
            while (output_name != 0 && (*output_name == ' ' || *output_name == '\t'))
                ++output_name;
            output_name[strcspn(output_name, "\t\n")] = 0;
            output_stream = fopen(output_name, "r");
            if (output_stream != NULL) {
                printf("%s already exists! Overwrite? Y:Overwrite / N:Append / C:Cancel\n", output_name);
                int tmp = yes_no(1);
                if (tmp == -1) {
                    continue;
                } else if (tmp == 0) {
                    output_stream = fopen(output_name, "a");
                    fputc('\n', output_stream);
                } else {
                    output_stream = fopen(output_name, "w");
                }
            } else {
                output_stream = fopen(output_name, "w");
            }
            if (output_stream == NULL) {
                printf("Invalid file name!\n");
                continue;
            }
            break;
        }
    }
}

void dump_regs(FILE *stream) {
    for (int i = 0; i < 32; ++i) {
        fprintf(stream, "   S%02d || Decimal: % -11d || Binary:", i, reg[i]);
        print_bits(reg[i], 32, stream);
    }
    fprintf(stream, "Status || Decimal: % -11d || Binary:", status);
    for (int i = 0; i < 27; ++i)
        fputc(' ', stream);
    print_bits(status, 8, stream);
    fputc('\n', stream);
}

int yes_no(int cancel) {
    int tmp, mode;
    while (1) {
        do {
            mode = getchar();
        } while (mode == ' ' || mode == '\n' || mode == '\t');
        do {
            tmp = getchar();
        } while (tmp == ' ' || tmp == '\t');
        if (tmp != '\n' || mode != 'n' && mode != 'y' && mode != 'N' && mode != 'Y') {
            if (cancel == 1 && (mode == 'c' || mode == 'C'))
                break;
            printf("Invalid input! Enter Y/N%s\n", cancel == 1 ? "/C" : "");
            while (getchar() != '\n');
            continue;
        }
        break;
    }
    if (mode == 'Y' || mode == 'y')
        return 1;
    else if (mode == 'C' || mode == 'c')
        return -1;
    return 0;
}

int suggest(const int arg[], int arg_need_count, const int arg_need_type[]) {
    if (suggestion_status == 0)
        return 1;
    for (int i = 0; i < arg_need_count; ++i) {
        if (arg_need_type[i] == 0 && (arg[i] > 31 || arg[i] < 0))
            return 1;
    }
    printf("Did you mean %s", instruction);
    for (int i = 0; i < arg_need_count; ++i) {
        if (i != 0)
            putchar(',');
        putchar(' ');
        if (arg_need_type[i] == 0)
            putchar('S');
        printf("%d", arg[i]);
    }
    printf(" ? Y:Yes / N:No / C:Cancel all suggestions\n");
    int tmp = yes_no(1);
    if (tmp == 1) {
        for (int i = 0; i < arg_count; ++i)
            arg_type[i] = arg_need_type[i];
        return 0;
    } else if (tmp == -1)
        suggestion_status = 0;
    return 1;
}