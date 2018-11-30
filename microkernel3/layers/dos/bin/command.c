#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CMD_MAX 64
#define PATH_MAX 32

char drive;
char cwd[PATH_MAX];

void exec_command(const char *cmd, const char *param) {
    printf("Executing cmd=%s param='%s'\n", cmd, param);

    // Internal commands.
    if (!strcmp(cmd, "ECHO")) {
        printf("%s\n", param);
    } else {
        // External commands.
        uint64_t rax, rbx, rcx, rdx;
        rax = SYSFUNC_EX_EXEC;
        rdx = (uint64_t) cmd;
        rbx = (uint64_t) param;
        rcx = strlen(param) << 16 | strlen(cmd);
        sysfunc(&rax, &rbx, &rcx, &rdx);
    }
}

void prompt(void) {
    printf("%c:%s> ", drive, (char *) &cwd);

    // Read command.
    char buf[CMD_MAX];
    int i = 0;
    int cmd_end = -1;
    int ch;
    while ((ch = getchar()) != '\n' && i < CMD_MAX - 1) {
        buf[i] = ch;
        printf("%c", ch);

        if (ch == ' ' && cmd_end == -1) {
            cmd_end = i;
            buf[i] = '\0';
        }

        i++;
    }

    buf[i] = '\0';
    printf("\n");

    char *cmd = &buf[0];
    char *param = (cmd_end == -1) ? "" : &buf[cmd_end + 1];
    exec_command(cmd, param);
}

int main(void) {
    drive = 'K';
    strcpy(cwd, "\\");

    for (;;) {
        prompt();
    }
}
