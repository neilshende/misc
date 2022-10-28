#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main( int argc, char *argv[] )
{
    int pid = atoi(argv[1]);
    char cmd[512] = {0}; 
    snprintf(cmd, 512, "awk '/State:/ {print $3}' /proc/%d/status", (int)pid);
    FILE *cmd_out = popen(cmd, "r");
    char result[80] = {0}; 
    fgets(result, 80, cmd_out);
    if (!strncmp(result, "(stopped)", 9)) {
      printf("The pid %d IS stopped [%s]\n", (int)pid, result);
    } else {
      printf("The pid %d NOT stopped [%s]\n", (int)pid, result);
    }
    return 0;
}
