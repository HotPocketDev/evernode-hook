#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

void trace(const char *trace)
{
    printf("%s", trace);
    printf("\n");
    fflush(stdout);
}

int main()
{
    char str[2048];
    int i;

    trace("**TRACE**Enter a string: ");

    fgets(str, 2048, stdin);

    /* remove newline, if present */
    i = strlen(str) - 1;
    if (str[i] == '\n')
        str[i] = '\0';

    char *format = "**TRACE**Transaction: %s";
    const int len = 23 + i;
    char out[len];
    sprintf(out, format, str);
    trace(out);

    sleep(1);

    trace("**EMIT**Test");

    exit(0);
}