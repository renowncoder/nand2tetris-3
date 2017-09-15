#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

#include "utils.h"
#include "exit.h"

#define MAX_LINE_LEN 200
/* Max chars of generated assembly output for a single line/command. */
#define MAX_ASM_OUT  1000
/* Number of tokens of the command with the most tokens (out of all cmds). */
#define MAX_TOKENS 3


typedef enum cmd_id {
    CMD_INVALID = 0,
    CMD_PUSH,
    CMD_POP,
    CMD_ADD,
    CMD_SUB,
    CMD_NEG,
    CMD_AND,
    CMD_OR,
    CMD_NOT,
    CMD_EQ,
    CMD_GT,
    CMD_LT,
    MAX_COMMANDS  /* their total count */
} cmd_id;

cmd_id str_to_cmdid(const char *s)
{
    cmd_id id = CMD_INVALID;

    if (s == NULL) {
    } else if (!strcmp(s, "push")) {
        id = CMD_PUSH;
    } else if (!strcmp(s, "pop")) {
        id = CMD_POP;
    } else if (!strcmp(s, "add")) {
        id = CMD_ADD;
    } else if (!strcmp(s, "sub")) {
        id = CMD_SUB;
    } else if (!strcmp(s, "neg")) {
        id = CMD_NEG;
    } else if (!strcmp(s, "and")) {
        id = CMD_AND;
    } else if (!strcmp(s, "or")) {
        id = CMD_OR;
    } else if (!strcmp(s, "not")) {
        id = CMD_NOT;
    } else if (!strcmp(s, "eq")) {
        id = CMD_EQ;
    } else if (!strcmp(s, "gt")) {
        id = CMD_GT;
    } else if (!strcmp(s, "lt")) {
        id = CMD_LT;
    }

    return id;
}

/*
 * Check whether the given path corresponds to a regular file and if that's
 * the case try to open the file using fopen.
 */
FILE *file_open_or_bail(const char *filename, const char *mode)
{
    struct stat path_stat;

    if (stat(filename, &path_stat) != 0) {
        exit_program(EXIT_FILE_DOES_NOT_EXIST, filename);
    }
    if(!S_ISREG(path_stat.st_mode)) {
        exit_program(EXIT_NOT_REGULAR_FILE, filename);
    }

    FILE *fp = fopen(filename, mode);

    if (fp == NULL) {
        exit_program(EXIT_CANNOT_OPEN_FILE, filename);
    }

    return fp;
}

/*
 * Strip a line from comments and remove trailing whitespace.
 */
char *strip_comments(char *s) {
    // sanity checks
    if (s == NULL) {
        return NULL;
    } else if (!*s) {
        return s;
    }

    char *s2 = s;
    char *last_char = s;

    for (; *s2; s2++) {
        if (*s2 == '/' && *(s2+1) == '/') {
            *s2 = '\0';
            break;
        } else if (!isspace(*s2)) {
            last_char = s2;
        }
    }

    if (*last_char) {
        *(last_char + 1) = '\0';
    }

    return s;
}

typedef bool (*parser_ptr)(int, const char **, char *);

bool parser_invalid(__attribute__((unused)) int nargs, __attribute__((unused)) const char *args[nargs], __attribute__((unused)) char *output) {
    puts("Invalid parser called!");
    return true;
}

bool parser_push(__attribute__((unused)) int nargs, __attribute__((unused)) const char *args[nargs], __attribute__((unused)) char *output) {
    puts("Push parser called!");
    return true;
}

bool parser_pop(__attribute__((unused)) int nargs, __attribute__((unused)) const char *args[nargs], __attribute__((unused)) char *output) {
    puts("Pop parser called!");
    return true;
}

bool parser_add(__attribute__((unused)) int nargs, __attribute__((unused)) const char *args[nargs], __attribute__((unused)) char *output) {
    puts("Add parser called!");
    return true;
}

bool parser_sub(__attribute__((unused)) int nargs, __attribute__((unused)) const char *args[nargs], __attribute__((unused)) char *output) {
    puts("Sub parser called!");
    return true;
}

bool parser_neg(__attribute__((unused)) int nargs, __attribute__((unused)) const char *args[nargs], __attribute__((unused)) char *output) {
    puts("Neg parser called!");
    return true;
}

bool parser_and(__attribute__((unused)) int nargs, __attribute__((unused)) const char *args[nargs], __attribute__((unused)) char *output) {
    puts("And parser called!");
    return true;
}

bool parser_or(__attribute__((unused)) int nargs, __attribute__((unused)) const char *args[nargs], __attribute__((unused)) char *output) {
    puts("Or parser called!");
    return true;
}

bool parser_not(__attribute__((unused)) int nargs, __attribute__((unused)) const char *args[nargs], __attribute__((unused)) char *output) {
    puts("Not parser called!");
    return true;
}

bool parser_eq(__attribute__((unused)) int nargs, __attribute__((unused)) const char *args[nargs], __attribute__((unused)) char *output) {
    puts("Eq parser called!");
    return true;
}

bool parser_gt(__attribute__((unused)) int nargs, __attribute__((unused)) const char *args[nargs], __attribute__((unused)) char *output) {
    puts("Gt parser called!");
    return true;
}

bool parser_lt(__attribute__((unused)) int nargs, __attribute__((unused)) const char *args[nargs], __attribute__((unused)) char *output) {
    puts("Lt parser called!");
    return true;
}

int main(int argc, const char *argv[])
{
    if (argc != 2) {
        exit_program(EXIT_MANY_FILES);
    }

    /*
     * Holds current line read.
     */
    char line[MAX_LINE_LEN + 1];
    char tmp_line[MAX_LINE_LEN + 1];
    /*
     * Holds the generated assembly code for the current line read.
     */
    char asm_output[MAX_ASM_OUT + 1];
    /*
     * To be filled with command tokens.
     */
    char *tokens[MAX_TOKENS + 1] = {NULL};
    /*
     * Number of tokens of current line / command.
     */
    int ntokens;

    parser_ptr parser_fn[MAX_COMMANDS] = {
        [CMD_INVALID] = parser_invalid, [CMD_PUSH] = parser_push,
        [CMD_POP] = parser_pop, [CMD_ADD] = parser_add, [CMD_SUB] = parser_sub,
        [CMD_NEG] = parser_neg, [CMD_AND] = parser_and, [CMD_OR] = parser_or,
        [CMD_NOT] = parser_not, [CMD_EQ] = parser_eq, [CMD_GT] = parser_gt,
        [CMD_LT] = parser_lt
    };

    FILE *fp = file_open_or_bail(argv[1], "r");

    while (fgets(line, sizeof(line), fp)) {
        strip_comments(line);
        if (is_empty(line)) {
            continue; // skip empty lines
        }

        strcpy(tmp_line, line);
        ntokens = s_tokenize(tmp_line, tokens, MAX_TOKENS+1, " ");
        // ntokens should be at least 1 because we have skipped empty lines
        cmd_id cmdid = str_to_cmdid(tokens[0]);
        puts(line);
        parser_fn[cmdid](ntokens, (const char **) tokens, asm_output);

        /*for (int i = 0; i < ntokens; i++) {*/
            /*puts(tokens[i]);*/
        /*}*/
        puts("---------");
    }

    fclose(fp);

    return 0;
}
