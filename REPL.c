#include <stdio.h>
#include <string.h>


int main() {
    char  input [100];
    while (1) {
        printf("branchdb > ");

        if (!fgets(input, sizeof(input), stdin)) {
            printf("\n");
            break;
        }

        if (strncmp(input, ".exit", 5) == 0) {
            break;
        }

        printf("You typed: %s", input);
    }
    return 0;
}