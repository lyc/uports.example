#include <stdio.h>
#include <stdlib.h>
#include <ezxml.h>

int main(int argc, char *argv[])
{
        ezxml_t f1, t, d;
        const char *fn, *teamname;

        if (argc > 1)
                fn = argv[1];
        else
                fn = "tests/22-ezxml/formula1.xml";

        f1 = ezxml_parse_file(fn);
        for (t=ezxml_child(f1, "team"); t; t=t->next) {
                teamname = ezxml_attr(t, "name");
                for (d=ezxml_child(t, "driver"); d; d=d->next) {
                        printf("%s, %s: %s\n",
                               ezxml_child(d, "name")->txt, teamname,
                               ezxml_child(d, "points")->txt);
                }
        }
        ezxml_free(f1);
        return 0;
}
