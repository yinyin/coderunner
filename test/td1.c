#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pwd.h>

#include <sys/types.h>



int main(int argc, char *argv[])
{
	{
		int i;
		for(i = 1; i < argc; i++) {
			printf("A: %s\n", argv[i]);
		}
	}

	{
		char *p;
		p = getenv("ENV1");
		printf("E1: %s\n", (NULL == p) ? "-" : p);
		p = getenv("ENV2");
		printf("E2: %s\n", (NULL == p) ? "-" : p);
	}

	{
		struct passwd *w;
		w = getpwuid(getuid());
		printf("Un: %s\n", (NULL == w) ? "?" : w->pw_name);
		w = getpwuid(geteuid());
		printf("Ue: %s\n", (NULL == w) ? "?" : w->pw_name);
	}

	fprintf(stderr, "Output to STDERR.\n");

	return 0;
}

