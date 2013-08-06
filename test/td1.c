#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pwd.h>
#include <limits.h>

#include <sys/types.h>



int main(int argc, char *argv[])
{
	int noprint_flag;

	noprint_flag = 0;

	{
		int i;
		for(i = 1; i < argc; i++) {
			printf("A: %s\n", argv[i]);
		}

		if( (argc > 1) && ('N' == argv[1][0]) )
		{
			noprint_flag = (int)(argv[1][1] - '0');
			printf("NPRT: %d\n", noprint_flag);
		}
	}

	if(0 == (noprint_flag & 1))
	{
		char *p;
		p = getenv("ENV1");
		printf("E1: %s\n", (NULL == p) ? "-" : p);
		p = getenv("ENV2");
		printf("E2: %s\n", (NULL == p) ? "-" : p);
	}

	if(0 == (noprint_flag & 2))
	{
		struct passwd *w;
		w = getpwuid(getuid());
		printf("Un: %s\n", (NULL == w) ? "?" : w->pw_name);
		w = getpwuid(geteuid());
		printf("Ue: %s\n", (NULL == w) ? "?" : w->pw_name);
	}

	if(0 == (noprint_flag & 4))
	{
		char buf[PATH_MAX];
		if(NULL == getcwd(buf, PATH_MAX))
		{ printf("PWD: -\n"); }
		else
		{
			buf[PATH_MAX-1] = '\0';
			printf("PWD: %s\n", buf);
		}
	}

	fprintf(stderr, "Output to STDERR.\n");

	return 0;
}

