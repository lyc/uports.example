#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef NOT_USED
#define NOT_USED(a) (void)(a)
#endif

int main(int argc, char *argv[])
{
	int rc = 0;

        NOT_USED(argc);
        printf("Welcome to %s...\n", argv[0]);

        return rc;
}
