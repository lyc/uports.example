/*****************************************************************
 * outline.c
 *
 * Copyright 1999, Clark Cooper
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the same terms as Perl.
 *
 * Read an XML document from standard input and print an element
 * outline on standard output.
 */


#include <stdio.h>
#include <string.h>
#include <expat.h>

#define BUFFSIZE	8192

char Buff[BUFFSIZE];

int Depth;

/* End of start handler */
void start(void *data, const char *el, const char **attr)
{
	int i;

        (void)data;
	if (!strcmp( el, "config"))
		printf( "==> %s : %d\n", el, Depth);

	if (!strcmp( el, "module") ||
	    !strcmp( el, "container") || !strcmp( el, "list") ||
	    !strcmp( el, "leaf") || !strcmp( el, "leaf-list")) {
		for (i = 0; i < Depth; i++)
			printf("  ");

		printf("<%d%s", Depth, el);

		for (i = 0; attr[i]; i += 2) {
			printf(" %s='%s'", attr[i], attr[i + 1]);
		}

		printf("\n");
	}
	Depth++;
}


/* End of end handler */
void end(void *data, const char *el)
{
	int i;

        (void)data;
	Depth--;

	if (!strcmp( el, "module") ||
	    !strcmp( el, "container") || !strcmp( el, "list") ||
	    !strcmp( el, "leaf") || !strcmp( el, "leaf-list")) {
		for (i = 0; i < Depth; i++)
			printf("  ");
		printf("%s>\n", el);
	}

	if (!strcmp( el, "config"))
		printf( "==> %s : %d\n", el, Depth);
}


/* End of main */
int main(int argc, char **argv)
{
	XML_Parser p;

        (void)argc;
        (void)argv;

	p = XML_ParserCreate(NULL);
	if (!p) {
		fprintf(stderr, "Couldn't allocate memory for parser\n");
		exit(-1);
	}

	XML_SetElementHandler(p, start, end);

	for (;;) {
		int done;
		int len;

		len = fread(Buff, 1, BUFFSIZE, stdin);
		if (ferror(stdin)) {
			fprintf(stderr, "Read error\n");
			exit(-1);
		}
		done = feof(stdin);

		if (! XML_Parse(p, Buff, len, done)) {
			fprintf(stderr, "Parse error at line %ld:\n%s\n",
				XML_GetCurrentLineNumber(p),
				XML_ErrorString(XML_GetErrorCode(p)));
			exit(-1);
		}

		if (done)
			break;
	}
	return 0;
}
