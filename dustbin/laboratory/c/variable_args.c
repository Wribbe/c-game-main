#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

void print_all_the_strings(int num, ...)
{
    va_list valist;
    va_start(valist, num);

    for (int i=0; i<num; i++) {
        printf("%s", va_arg(valist, const char *));
    }

    va_end(valist);
}

int main(void)
{
    const char * a = "This is message a.\n";
    const char * b = "This is message b.\n";
    const char * c = "This is message c.\n";

    print_all_the_strings(3, a, b, c);
}
