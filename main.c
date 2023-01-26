#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define ARG_MAX_COUNT 3                          ///<Maximum number of arguments to store.
#define INSTRUCTION_MAX_CHARACTERS 12            ///<Maximum number of instruction characters to store.
#define STANDARD_ERROR_OUTPUT stdout //stderr

FILE *input_stream = NULL;                       ///<File to read instructions from.
FILE *output_stream = NULL;                      ///<File to print register values in.
FILE *LOG_STREAM;                                ///<File to save errors in.
long *line_place = NULL;                         ///<Place of each line in file.
int current_line;                                ///<Number of line that was read.
int max_line;                                    ///<File lines count.
int reg[32];                                     ///<4 byte registers.
unsigned __int8 status = 0;                      ///<1 byte status register.
char instruction[INSTRUCTION_MAX_CHARACTERS];    ///<Last instruction read from file.
int arg_type[ARG_MAX_COUNT];                     ///<Type of each argument in current line. 1:Imm / 0:Register number / -1:Empty
int arg_count;                                   ///<Number of arguments in current line.
int is_overflow = 0;                             ///<1 If overflowed.
int *stack;                                      ///<Stack allocated in memory.
int stack_index = -1;                            ///<Index of last element in stack.
/**
 * @brief Maximum number of elements to store.
 *
 * Increases if needed.
 */
int stack_max = 10;
int suggestion_status = 1;                       ///<0 If suggestion is disabled by user.

int get_input_file_name();

void open_input_file();

void reset_check();

int processor();

int get_input_from_file(int *arg);

int str_case(char *key);

void status_check(long long int result, char operator, int num_1, int num_2);

void open_output_file();

void dump_regs(FILE *stream);

void print_bits(unsigned int num, int bits, FILE *stream);

int arg_error(const int arg[], int arg_need_count, const int arg_need_type[], FILE *stream);

int segmentation_error(const int *arg, FILE *stream);

void print_overflow_warning();

int yes_no(int cancel);

int suggest(const int arg[], int arg_need_count, const int arg_need_type[]);

/**
 * @brief Main function of program.
 *
 * Checks for exit status.
 * Gets input file name if needed.
 * @note If there was no EXIT instruction in input file can open a new input file after EOF.
 */
int main() {
    stack = (int *) malloc(stack_max * sizeof(int));
    int is_exit = 0;
    LOG_STREAM = fopen("Errors.log", "w");
    while (is_exit != 1) {
        if (is_exit < 0 || input_stream == NULL) {
            if (is_exit != 0) {
                printf("Do you want to exit? Y/N\n");
                if (yes_no(0))
                    break;
                if (is_exit == -1)
                    reset_check();
            }
            if (get_input_file_name()) {
                is_exit = -2;
                continue;
            }
            open_input_file();
            fprintf(LOG_STREAM, "\n___________________________________________________________________\n");
        }
        is_exit = processor(input_stream);
    }
    fclose(input_stream);
    fclose(output_stream);
    fclose(LOG_STREAM);
    return 0;
}

/**
 * @brief Execute instructions.
 *
 * Execute instruction if there are no errors.
 * @return 0 Call next line.
 * @return 1 Exit program.
 * @return -1 End of file.
 * @note Changes #current_line variable.
 * @note Print detailed errors for Jmp, SKIE, POP, DIV (by zero) and unknown instruction.
 * @note Print input overflow warnings.
 * @note Print a message if EOF.
 */
int processor() {
    int arg[ARG_MAX_COUNT] = {0};
    for (int i = 0; i < ARG_MAX_COUNT; ++i)
        arg_type[i] = -1;
    is_overflow = 0;
    int is_eof = get_input_from_file(arg);
    while (ftell(input_stream) != line_place[current_line])
        ++current_line;
    if (is_eof == 1) {
        printf("Reached the end of file!\n");
        return -1;
    } else if (str_case("ADD")) {
        int arg_need_type[] = {0, 0, 0};
        if (arg_error(arg, 3, arg_need_type, STANDARD_ERROR_OUTPUT) || segmentation_error(arg, STANDARD_ERROR_OUTPUT))
            return 0;
        int tmp = reg[arg[1]] + reg[arg[2]];
        status_check(tmp, '+', reg[arg[1]], reg[arg[2]]);
        reg[arg[0]] = tmp;
    } else if (str_case("SUB")) {
        int arg_need_type[] = {0, 0, 0};
        if (arg_error(arg, 3, arg_need_type, STANDARD_ERROR_OUTPUT) || segmentation_error(arg, STANDARD_ERROR_OUTPUT))
            return 0;
        int tmp = reg[arg[1]] - reg[arg[2]];
        status_check(tmp, '-', reg[arg[1]], reg[arg[2]]);
        reg[arg[0]] = tmp;
    } else if (str_case("AND")) {
        int arg_need_type[] = {0, 0, 0};
        if (arg_error(arg, 3, arg_need_type, STANDARD_ERROR_OUTPUT) || segmentation_error(arg, STANDARD_ERROR_OUTPUT))
            return 0;
        reg[arg[0]] = reg[arg[1]] & reg[arg[2]];
        status_check(reg[arg[0]], '&', reg[arg[1]], reg[arg[2]]);
    } else if (str_case("XOR")) {
        int arg_need_type[] = {0, 0, 0};
        if (arg_error(arg, 3, arg_need_type, STANDARD_ERROR_OUTPUT) || segmentation_error(arg, STANDARD_ERROR_OUTPUT))
            return 0;
        reg[arg[0]] = reg[arg[1]] ^ reg[arg[2]];
        status_check(reg[arg[0]], '^', reg[arg[1]], reg[arg[2]]);
    } else if (str_case("OR")) {
        int arg_need_type[] = {0, 0, 0};
        if (arg_error(arg, 3, arg_need_type, STANDARD_ERROR_OUTPUT) || segmentation_error(arg, STANDARD_ERROR_OUTPUT))
            return 0;
        reg[arg[0]] = reg[arg[1]] | reg[arg[2]];
        status_check(reg[arg[0]], '|', reg[arg[1]], reg[arg[2]]);
    } else if (str_case("ADDI")) {
        int arg_need_type[] = {0, 0, 1};
        if (arg_error(arg, 3, arg_need_type, STANDARD_ERROR_OUTPUT) || segmentation_error(arg, STANDARD_ERROR_OUTPUT))
            return 0;
        int tmp = reg[arg[1]] + arg[2];
        status_check(tmp, '+', reg[arg[1]], arg[2]);
        reg[arg[0]] = tmp;
    } else if (str_case("SUBI")) {
        int arg_need_type[] = {0, 0, 1};
        if (arg_error(arg, 3, arg_need_type, STANDARD_ERROR_OUTPUT) || segmentation_error(arg, STANDARD_ERROR_OUTPUT))
            return 0;
        int tmp = reg[arg[1]] - arg[2];
        status_check(tmp, '-', reg[arg[1]], arg[2]);
        reg[arg[0]] = tmp;
    } else if (str_case("ANDI")) {
        int arg_need_type[] = {0, 0, 1};
        if (arg_error(arg, 3, arg_need_type, STANDARD_ERROR_OUTPUT) || segmentation_error(arg, STANDARD_ERROR_OUTPUT))
            return 0;
        reg[arg[0]] = reg[arg[1]] & arg[2];
        status_check(reg[arg[0]], '&', reg[arg[1]], arg[2]);
    } else if (str_case("XORI")) {
        int arg_need_type[] = {0, 0, 1};
        if (arg_error(arg, 3, arg_need_type, STANDARD_ERROR_OUTPUT) || segmentation_error(arg, STANDARD_ERROR_OUTPUT))
            return 0;
        reg[arg[0]] = reg[arg[1]] ^ arg[2];
        status_check(reg[arg[0]], '^', reg[arg[1]], arg[2]);
    } else if (str_case("ORI")) {
        int arg_need_type[] = {0, 0, 1};
        if (arg_error(arg, 3, arg_need_type, STANDARD_ERROR_OUTPUT) || segmentation_error(arg, STANDARD_ERROR_OUTPUT))
            return 0;
        reg[arg[0]] = reg[arg[1]] | arg[2];
        status_check(reg[arg[0]], '|', reg[arg[1]], arg[2]);
    } else if (str_case("MOV")) {
        int arg_need_type[] = {0, -1};
        if (arg_error(arg, 2, arg_need_type, STANDARD_ERROR_OUTPUT) || segmentation_error(arg, STANDARD_ERROR_OUTPUT))
            return 0;
        if (arg_type[1]) {
            reg[arg[0]] = arg[1];
        } else {
            reg[arg[0]] = reg[arg[1]];
        }
    } else if (str_case("SWP")) {
        int arg_need_type[] = {0, 0};
        if (arg_error(arg, 2, arg_need_type, STANDARD_ERROR_OUTPUT) || segmentation_error(arg, STANDARD_ERROR_OUTPUT))
            return 0;
        int tmp = reg[arg[0]];
        reg[arg[0]] = reg[arg[1]];
        reg[arg[1]] = tmp;
    } else if (str_case("DUMP_REGS")) {
        int arg_need_type[] = {-1};
        if (arg_error(arg, 0, arg_need_type, STANDARD_ERROR_OUTPUT))
            return 0;
        dump_regs(stdout);
    } else if (str_case("DUMP_REGS_F")) {
        int arg_need_type[] = {-1};
        if (arg_error(arg, 0, arg_need_type, STANDARD_ERROR_OUTPUT))
            return 0;
        open_output_file();
        dump_regs(output_stream);
    } else if (str_case("INPUT")) {
        int arg_need_type[] = {-1};
        if (arg_error(arg, 0, arg_need_type, STANDARD_ERROR_OUTPUT))
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
        if (arg_error(arg, 0, arg_need_type, STANDARD_ERROR_OUTPUT))
            return 0;
        printf("%d\n", reg[0]);
    } else if (str_case("JMP")) {
        int arg_need_type[] = {1};
        if (arg_error(arg, 1, arg_need_type, STANDARD_ERROR_OUTPUT) || segmentation_error(arg, STANDARD_ERROR_OUTPUT))
            return 0;
        if (arg[0] > max_line || arg[0] <= 0) {
            fprintf(STANDARD_ERROR_OUTPUT, "Line:%-3d| Out of range error!\n\tLine %d doesn't exist\n", current_line,
                    arg[0]);
            fprintf(LOG_STREAM, "Line:%-3d| Out of range error!\n\tLine %d doesn't exist\n", current_line, arg[0]);
            return 0;
        }
        current_line = arg[0] - 1;
        fseek(input_stream, line_place[current_line], SEEK_SET);
        return 0;
    } else if (str_case("MULL")) {
        int arg_need_type[] = {0, 0};
        if (arg_error(arg, 2, arg_need_type, STANDARD_ERROR_OUTPUT) || segmentation_error(arg, STANDARD_ERROR_OUTPUT))
            return 0;
        long long int tmp = (long long int) reg[arg[0]] * (long long int) reg[arg[1]];
        status_check(tmp, '*', reg[arg[0]], reg[arg[1]]);
        reg[arg[0]] = (int) (tmp >> 32);
        reg[arg[1]] = (int) tmp;
    } else if (str_case("DIV")) {
        int arg_need_type[] = {0, 0};
        if (arg_error(arg, 2, arg_need_type, STANDARD_ERROR_OUTPUT) || segmentation_error(arg, STANDARD_ERROR_OUTPUT))
            return 0;
        if (reg[arg[1]] == 0) {
            fprintf(STANDARD_ERROR_OUTPUT, "Line:%-3d| Error! Division by zero\n", current_line);
            fprintf(LOG_STREAM, "Line:%-3d| Error! Division by zero\n", current_line);
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
        if (arg_error(arg, 2, arg_need_type, STANDARD_ERROR_OUTPUT) || segmentation_error(arg, STANDARD_ERROR_OUTPUT))
            return 0;
        else if (current_line == max_line && reg[arg[0]] == reg[arg[1]]) {
            fprintf(STANDARD_ERROR_OUTPUT, "Line:%-3d| Out of range error!\n\tLine %d doesn't exist\n", current_line,
                    max_line + 1);
            fprintf(LOG_STREAM, "Line:%-3d| Out of range error!\n\tLine %d doesn't exist\n", current_line,
                    max_line + 1);
        } else if (reg[arg[0]] == reg[arg[1]]) {
            ++current_line;
            fseek(input_stream, line_place[current_line], SEEK_SET);
        }
    } else if (str_case("PUSH")) {
        int arg_need_type[] = {0};
        if (arg_error(arg, 1, arg_need_type, STANDARD_ERROR_OUTPUT) || segmentation_error(arg, STANDARD_ERROR_OUTPUT))
            return 0;
        ++stack_index;
        if (stack_index == stack_max) {
            stack_max += 10;
            stack = realloc(stack, stack_max * sizeof(int));
        }
        stack[stack_index] = reg[arg[0]];
    } else if (str_case("POP")) {
        int arg_need_type[] = {0};
        if (arg_error(arg, 1, arg_need_type, STANDARD_ERROR_OUTPUT) || segmentation_error(arg, STANDARD_ERROR_OUTPUT))
            return 0;
        if (stack_index == -1) {
            fprintf(STANDARD_ERROR_OUTPUT, "Line:%-3d| Stack error!\n\tStack is empty\n", current_line);
            fprintf(LOG_STREAM, "Line:%-3d| Stack error!\n\tStack is empty\n", current_line);
            return 0;
        }
        reg[arg[0]] = stack[stack_index];
        --stack_index;
    } else if (str_case("EXIT")) {
        int arg_need_type[] = {-1};
        if (arg_error(arg, 0, arg_need_type, STANDARD_ERROR_OUTPUT))
            return 0;
        return 1;
    } else if (instruction[0] == 0) {
        return 0;
    } else {
        fprintf(STANDARD_ERROR_OUTPUT, "Line:%-3d| Syntax error!\n\tUnknown instruction %s\n", current_line,
                instruction);
        fprintf(LOG_STREAM, "Line:%-3d| Syntax error!\n\tUnknown instruction %s\n", current_line, instruction);
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

/**
 * @brief Read one line from file.
 * Change #instruction and #arg based on input.
 * @param arg An array of arguments to fill.
 * @return 1 If program reaches EOF before getting instruction from current line.
 * @return 2 If program reaches EOF after getting instruction from current line.
 * @return 0 If next line is available.
 * @note Function is whitespace-insensitive.
 * @note If "//" is reached the rest of the line will be skipped as comment.
 * @note Counts arguments of current line and stores it in #arg_count.
 * @warning Function recognizes multiple arguments by comma(,).
 */
int get_input_from_file(int *arg) {
    arg_count = 0;
    int tmp, sign = 1;
    if (fscanf(input_stream, "%s", instruction) == EOF)
        return 1;
    for (int i = 0; instruction[i]; ++i)
        instruction[i] = toupper(instruction[i]);
    for (int i = 1; i < INSTRUCTION_MAX_CHARACTERS && instruction[i] != 0; ++i) {
        if (instruction[i - 1] == '/' && instruction[i] == '/') {
            instruction[i - 1] = 0;
            while (1) {
                tmp = getc(input_stream);
                if (tmp == '\n')
                    return 0;
                else if (tmp == EOF)
                    return 2;
            }
        }
    }
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

/**
 * @brief Recognize instruction
 * @param key String to compare to the instruction from input file.
 * @return 1 If strings are equivalent.
 */
int str_case(char *key) {
    if (strcmp(instruction, key) == 0) {
        return 1;
    } else {
        return 0;
    }
}

/**
 * @brief Change status register.
 * @param result Argument 1 (Status register change based on this number).
 * @param operator '+'/'-'/...
 * @param num_1 Argument 2.
 * @param num_2 Argument 3.
 * @note Function changes overflow flag only for '+' and '-' operands.
 */
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

/**
 * @brief Print numbers in binary.
 *
 * @param num Number to be printed.
 * @param bits Bit count.
 * @param stream Stream to print in.
 * @note Places one space between bytes.
 */
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

/**
 * @brief Check for typos in arguments.
 *
 * Print detailed errors.
 * After printing errors suggests correct form if available.
 * @param arg Array of arguments read from current line.
 * @param arg_need_count How many arguments are needed.
 * @param arg_need_type Type of each argument. 1:Imm / 0:Register number / 2:Any / -1:Empty
 * @param stream Stream to print in.
 * @return 1 Error.
 * @return 0 No error.
 * @note Print in both console and Errors.log .
 */
int arg_error(const int arg[], int arg_need_count, const int arg_need_type[], FILE *stream) {
    int index, flag_invalid_input = 0;
    for (index = 0; index < arg_count && index < arg_need_count; ++index) {
        if ((arg_need_type[index] != arg_type[index] && arg_need_type[index] != -1) || arg_type[index] == 2) {
            if (flag_invalid_input == 0) {
                flag_invalid_input = 1;
                fprintf(stream, "Line:%-3d| Syntax error!\n", current_line);
            }
            fprintf(stream, "\tArgument %d is invalid\n", index + 1);
        }
    }
    if (arg_need_count != arg_count) {
        if (flag_invalid_input == 0) {
            flag_invalid_input = 1;
            fprintf(stream, "Line:%-3d| Syntax error!\n", current_line);
        }
        fprintf(stream, "\t%s instruction needs %d argument", instruction, arg_need_count);
        if (arg_need_count != 1)
            putc('s', stream);
        fprintf(stream, " but you entered %d argument", arg_count);
        if (arg_count != 1)
            putc('s', stream);
        putc('\n', stream);
    }
    if (flag_invalid_input) {
        fprintf(stream, "\tCorrect format: %s ", instruction);
        for (int i = 0; i < arg_need_count; ++i) {
            if (i)
                fprintf(stream, ", ");
            if (arg_need_type[i] == 0) {
                fprintf(stream, "S%c", 'a' + i);
            } else if (arg_need_type[i] == 1) {
                fprintf(stream, "Imm");
            } else {
                fprintf(stream, "S%c/Imm", 'a' + i);
            }
        }
        putc('\n', stream);
        if (stream == STANDARD_ERROR_OUTPUT)
            arg_error(arg, arg_need_count, arg_need_type, LOG_STREAM);
        else if (stream == LOG_STREAM)
            return 0;
        if (suggest(arg, arg_need_count, arg_need_type))
            return 1;
    }
    return 0;
}

/**
 * @brief Check for segmentation errors in using registers.
 *
 * Print detailed error;
 * @param arg Array of arguments read from current line.
 * @param stream Stream to print in.
 * @return 1 Error.
 * @return 0 No error.
 * @note Print in both console and Errors.log .
 */
int segmentation_error(const int *arg, FILE *stream) {
    int index, flag_invalid_input = 0;
    for (index = 0; index < arg_count; ++index) {
        if (arg_type[index] == 0 && (arg[index] < 0 || arg[index] >= 32)) {
            if (flag_invalid_input == 0) {
                flag_invalid_input = 1;
                fprintf(stream, "Line:%-3d| Segmentation error!\n", current_line);
            }
            fprintf(stream, "\tArgument %d is invalid (Correct argument range: S0-S31)\n", index + 1);
        }
    }
    if (stream == STANDARD_ERROR_OUTPUT)
        segmentation_error(arg, LOG_STREAM);
    if (flag_invalid_input == 1)
        return 1;
    return 0;
}

/**
 * @brief Print overflow warning.
 */
void print_overflow_warning() {
    printf("Line:%-3d| Overflow warning!\n", current_line);
}

/**
 * @brief Reset registers and stack to 0.
 *
 * Ask user first.
 */
void reset_check() {
    printf("Reset stack and registers? Y/N\n");
    if (yes_no(0)) {
        for (int i = 0; i < 32; ++i)
            reg[i] = 0;
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

/**
 * @brief Get input file's name from user.
 *
 * @return 1 If can't find file.
 * @return 0 If successful.
 */
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

/**
 * @brief Open input file.
 *
 * Stores place of each line in #line_place.
 */
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

/**
 * @brief Open output file.
 *
 *
 * Ask from user if file exists (Y:Overwrite / N:Append / C:Cancel).
 */
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

/**
 * @brief Print all registers.
 *
 * Print in both decimal and binary.
 * @param stream Stream to print in.
 */
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

/**
 * @brief Ask user Yes/No(/Cancel) question.
 *
 * @param cancel 1 If cancel choice is available.
 * @return 1 If yes.
 * @return 0 If no.
 * @return -1 If cancel.
 */
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

/**
 * @brief Suggest correct form.
 *
 * May suggest correct form of instruction to user if error occurs.
 * @param arg Array of arguments read from current line.
 * @param arg_need_count How many arguments are needed.
 * @param arg_need_type Type of each argument. 1:Imm / 0:Register number / 2:Any / -1:Empty
 * @return 0 If user accepts suggestion.
 * @note Can be turned off by user using #suggestion_status.
 */
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
