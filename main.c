#include <stdio.h>
#include <string.h>

int s[32];
char command[12];

int strCase(char *key);
void input(int *arg, int *is_imm);
int processor();

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