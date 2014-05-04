#include <stdint.h>
#include <stdio.h>



#define PATH_BUFFER_LEN 128
#define USERACCOUNT_BUFFER_LEN 32

typedef struct _T_DaemonizeOption {
	uint32_t action_code;	/* use argument hash as action code */

	char *path_pidfile;
	char buffer_path_pidfile[PATH_BUFFER_LEN];

	char *run_as_user;
	char *path_workfolder;
	char *path_terminallogfile;
	char buffer_run_as_user[USERACCOUNT_BUFFER_LEN];
	char buffer_path_workfolder[PATH_BUFFER_LEN];
	char buffer_path_terminallogfile[PATH_BUFFER_LEN];

	uint32_t second_int_before_term;

	uint8_t is_verbose;
} DaemonizeOption;



/*
ARGHASH_PIDFILE			--pid=
ARGHASH_WORKDIR_LONG	--working-dir=
ARGHASH_WORKDIR_SHORT	--wd=
ARGHASH_TRMLOG_LONG		--log=
ARGHASH_TRMLOG_SHORT	--terminal-log=/dev/null
ARGHASH_RUN_AS			--run-as=
ARGHASH_SEND_INT_BEFORE_TERM	--send-int-before-term=
ARGHASH_VERBOSE_LONG	--verbose
ARGHASH_VERBOSE_SHORT	-v
ARGHASH_VERBOSE_ARGEND	--
ARGHASH_ACT_STARTSTART	startstart
ARGHASH_ACT_START		start
ARGHASH_ACT_STOP		stop
ARGHASH_ACT_STATUS		status
ARGHASH_ACT_CHECK		check
*/
/* {{{ generated argument hash */
#define ARGHASH_PIDFILE 0x8ECD0378
#define ARGHASH_WORKDIR_LONG 0x6755CF40
#define ARGHASH_WORKDIR_SHORT 0xE31BB564
#define ARGHASH_TRMLOG_LONG 0x993FB47F
#define ARGHASH_TRMLOG_SHORT 0x6A9BDAE0
#define ARGHASH_RUN_AS 0xC90C372B
#define ARGHASH_SEND_INT_BEFORE_TERM 0x33419A50
#define ARGHASH_VERBOSE_LONG 0x4CF1F27F
#define ARGHASH_VERBOSE_SHORT 0x59CD76CA
#define ARGHASH_VERBOSE_ARGEND 0x20CD1D0F
#define ARGHASH_ACT_STARTSTART 0x6B6DFC95
#define ARGHASH_ACT_START 0x652B04DF
#define ARGHASH_ACT_STOP 0xCB532AE5
#define ARGHASH_ACT_STATUS 0xBA4B77EF
#define ARGHASH_ACT_CHECK 0xB3711C47
/* }}} generated argument hash */


static uint32_t arg_hash(char *p, char **stop_pointer)
{
	uint32_t result;
	char ch;

	result = 2166136261;

	while('\0' != (ch = *p)) {
		if('=' == ch)
		{
			p++;
			break;
		}

		result = result ^ ((uint32_t)(ch) & 0xFF);
		result = result * 16777619;

		p++;
	}

	*stop_pointer = p;
	return result;
}


#define cmdoptcpy(buffer_ptr, value_ptr, arg, buffer_len) {	\
			strncpy(buffer_ptr, arg, buffer_len);	\
			buffer_ptr[(buffer_len-1)] = '\0';		\
			value_ptr = (('\0' == buffer_ptr[0]) ? NULL : buffer_ptr);	}

static int parse_command_arg(DaemonizeOption *cmdarg, int argc, char **argv)
{
	int i;

	for(i = 1; i < argc; i++) {
		uint32_t opt;
		char *arg;

		opt = arg_hash(argv[i], &arg);
		switch(opt)
		{
		case ARGHASH_PIDFILE:
			cmdoptcpy(cmdarg->buffer_path_pidfile, cmdarg->path_pidfile, arg, PATH_BUFFER_LEN);
			break;
		case ARGHASH_WORKDIR_LONG:
		case ARGHASH_WORKDIR_SHORT:
			cmdoptcpy(cmdarg->buffer_path_workfolder, cmdarg->path_workfolder, arg, PATH_BUFFER_LEN);
			break;
		case ARGHASH_TRMLOG_LONG:
		case ARGHASH_TRMLOG_SHORT:
			cmdoptcpy(cmdarg->buffer_path_terminallogfile, cmdarg->path_terminallogfile, arg, PATH_BUFFER_LEN);
			break;
		case ARGHASH_RUN_AS:
			cmdoptcpy(cmdarg->buffer_run_as_user, cmdarg->run_as_user, arg, USERACCOUNT_BUFFER_LEN);
			break;
		case ARGHASH_SEND_INT_BEFORE_TERM:
			{
				long int aux;
				char *endptr;
				aux = strtol(arg, &endptr, 10);
				if( ('\0' != *endptr) || (aux < 0) || (aux > 120) )
				{
					fprintf(stderr, "ERR: invalid argument for --send-int-before-term=\n");
					return 1;
				}
				cmdarg->second_int_before_term = aux;
			}
			break;
		case ARGHASH_VERBOSE_LONG:
		case ARGHASH_VERBOSE_SHORT:
			cmdarg->is_verbose++;
			break;
		case ARGHASH_VERBOSE_ARGEND:
			/* TODO: keep command line argument */
			break;
		case ARGHASH_ACT_STARTSTART:
			cmdarg->action_code = ARGHASH_ACT_STARTSTART;
			break;
		case ARGHASH_ACT_START:
			cmdarg->action_code = ARGHASH_ACT_START;
			break;
		case ARGHASH_ACT_STOP:
			cmdarg->action_code = ARGHASH_ACT_STOP;
			break;
		case ARGHASH_ACT_STATUS:
		case ARGHASH_ACT_CHECK:
			cmdarg->action_code = ARGHASH_ACT_STATUS;
			break;
	}

	/* TODO: validate command argument */

	return 0;
}



#if ENABLE_ARGHASH_CACULATOR
/* cc -DENABLE_ARGHASH_CACULATOR=1 src/daemonize.c
*/

#define OUTPUT_BUFFER_SIZE 2048
#define HASH_HISTORY_SIZE 64
int main(int argc, char **argv)
{
	char output_buffer_storage[OUTPUT_BUFFER_SIZE];
	char * output_buffer_current;
	int output_buffer_remain;

	uint32_t hash_history_storage[HASH_HISTORY_SIZE];
	int hash_history_size;

	char arghash_name[64];
	char arghash_target[64];

	output_buffer_current = output_buffer_storage;
	output_buffer_remain = OUTPUT_BUFFER_SIZE - 1;

	hash_history_size = 0;

	while(EOF != scanf("%s %s", arghash_name, arghash_target)) {
		uint32_t h;
		int i;
		char *stopped_p;
		int output_written;

		h = arg_hash(arghash_target, &stopped_p);

		for(i = 0; i < hash_history_size; i++) {
			if(h == hash_history_storage[i])
			{
				printf("WARN: hash collision at %d-th definition (hash=0x%08lX).\n", i, ((unsigned long int)(h)));
			}
		}
		hash_history_storage[hash_history_size] = h;
		hash_history_size++;

		output_written = snprintf(output_buffer_current, output_buffer_remain, "#define %s 0x%08lX\n", arghash_name, (unsigned long int)(h));
		output_buffer_current += output_written;
		output_buffer_remain -= output_written;
		if(output_buffer_remain < 8)
		{
			fprintf(stderr, "WARN: buffer size not enough.\n");
		}
	}

	printf("/* {{{ generated argument hash */\n");
	printf("%s", output_buffer_storage);
	printf("/* }}} generated argument hash */\n");

	return 0;
}
#else	/* ENABLE_ARGHASH_CACULATOR */

/*
TODO: main function
*/
#endif	/* ENABLE_ARGHASH_CACULATOR */

/*
vim: ts=4 sw=4 ai nowrap
*/
