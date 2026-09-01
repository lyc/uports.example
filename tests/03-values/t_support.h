#ifndef T_SUPPORT_H
#define T_SUPPORT_H

#include <stdio.h>
#include <stdlib.h>

#define T_CHECK(expr)                                                   \
	do {                                                            \
		if (!(expr)) {						\
			fprintf(stderr, "%s:%d: check failed: %s\n",	\
				__FILE__, __LINE__, #expr);		\
			return 1;					\
		}                                                       \
	} while (0)

#define T_RUN(fn)							\
	do {                                                            \
		int t_rc = (fn)();					\
		if (t_rc) {						\
			fprintf(stderr, "%s failed\n", #fn);		\
			return t_rc;					\
		}                                                       \
	} while (0)

#define T_REQUIRE(expr)                                                 \
	do {                                                            \
		if (!(expr)) {						\
			fprintf(stderr, "%s:%d: invariant failed: %s\n", \
				__FILE__, __LINE__, #expr);		\
			abort();					\
		}                                                       \
	} while (0)

#endif /* T_SUPPORT_H */
