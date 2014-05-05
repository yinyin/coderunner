#include <stdint.h>
#include <stdio.h>


const char *DEFAULT_INPUT_PATH = "/dev/null";
const char *DEFAULT_OUTPUT_PATH = "/dev/null";


typedef struct _T_DaemonizeOption {
	uint32_t action_code;	/* use argument hash as action code */

	char *path_pidfile;

	char *run_as_user;
	char *path_workfolder;
	char *path_terminallogfile;

	uint32_t second_int_before_term;

	char **cmd;

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



static void init_daemonize_option(DaemonizeOption *dmnzopt)
{
	memset(dmnzopt, 0, sizeof(DaemonizeOption));

	dmnzopt->action_code = 0;
	dmnzopt->path_pidfile = NULL;
	dmnzopt->run_as_user = NULL;
	dmnzopt->path_workfolder = NULL;
	dmnzopt->path_terminallogfile = NULL;
	dmnzopt->second_int_before_term = 0;
	dmnzopt->cmd = NULL;
	dmnzopt->is_verbose = 0;
}

static int parse_daemonize_cmdoption(DaemonizeOption *dmnzopt, int argc, char **argv)
{
	int i;

	for(i = 1; i < argc; i++) {
		uint32_t opt;
		char *arg;

		opt = arg_hash(argv[i], &arg);
		switch(opt)
		{
		case ARGHASH_PIDFILE:
			if('\0' != *arg)
			{ dmnzopt->path_pidfile = arg; }
			break;
		case ARGHASH_WORKDIR_LONG:
		case ARGHASH_WORKDIR_SHORT:
			if('\0' != *arg)
			{ dmnzopt->path_workfolder = arg; }
			break;
		case ARGHASH_TRMLOG_LONG:
		case ARGHASH_TRMLOG_SHORT:
			if('\0' != *arg)
			{ dmnzopt->path_terminallogfile = arg; }
			break;
		case ARGHASH_RUN_AS:
			if('\0' != *arg)
			{ dmnzopt->run_as_user = arg; }
			break;
		case ARGHASH_SEND_INT_BEFORE_TERM:
			{
				long int aux;
				char *endptr;
				aux = strtol(arg, &endptr, 10);
				if( ('\0' != *endptr) || (aux < 0) || (aux > 120) )
				{
					fprintf(stderr, "ERR: invalid argument for option \"--send-int-before-term=\".\n");
					return 1;
				}
				dmnzopt->second_int_before_term = aux;
			}
			break;
		case ARGHASH_VERBOSE_LONG:
		case ARGHASH_VERBOSE_SHORT:
			dmnzopt->is_verbose++;
			if(dmnzopt->is_verbose > 9)
			{ dmnzopt->is_verbose = 9; }
			break;
		case ARGHASH_VERBOSE_ARGEND:
			if((i + 1) < argc)
			{
				dmnzopt->cmd = argv + i + 1;
				return 0;
			}
			else
			{
				fprintf(stderr, "ERR: user command not found.\n");
				return 1;
			}
			break;
		case ARGHASH_ACT_STARTSTART:
			dmnzopt->action_code = ARGHASH_ACT_STARTSTART;
			break;
		case ARGHASH_ACT_START:
			dmnzopt->action_code = ARGHASH_ACT_START;
			break;
		case ARGHASH_ACT_STOP:
			dmnzopt->action_code = ARGHASH_ACT_STOP;
			break;
		case ARGHASH_ACT_STATUS:
		case ARGHASH_ACT_CHECK:
			dmnzopt->action_code = ARGHASH_ACT_STATUS;
			break;
		default:
			fprintf(stderr, "ERR: unknown option: %s\n", argv[i]);
			return 1;
		}
	}

	return 0;
}

static int valid_daemonize_option(DaemonizeOption *dmnzopt)
{
	if(0 == dmnzopt->action_code)
	{
		fprintf(stderr, "ERR: action is required.\n");
		fprintf(stdout,
			"Argument: [Action] [Options...] -- <Executable> [Parameters...]\n\n"
			"Action:\n"
			"  startstart  start program and automatically restart when program stops.\n"
			"  start       start program without automatically restart.\n"
			"  stop        stop program via given PID file.\n"
			"  status      check status of program via given PID file.\n"
			"Options:\n"
			"  --pid=[PID_FILE]          save/load process information in given path.\n"
			"  --working-dir=[WORK_FOLDER] | --wd=[WORK_FOLDER]\n"
			"                            path of work folder.\n"
			"  --terminal-log=[LOG_FILE_PATH] | --log=[LOG_FILE_PATH]\n"
			"                            path of terminal log.\n"
			"  --run-as=[USER_ACCOUNT]   run program with given user account.\n"
			"  --send-int-before-term=[SECONDs]\n"
			"                            send SIGINT before SIGTERM for stopping program.\n"
			"  -v | --verbose            enable verbose mode.\n"
			"\n");
		return 1;
	}

	if( (ARGHASH_ACT_STARTSTART == dmnzopt->action_code) || (ARGHASH_ACT_START == dmnzopt->action_code) )
	{
		if(NULL == dmnzopt->path_pidfile)
		{ fprintf(stderr, "WARN: PID record file does not given.\n"); }

		if(NULL == dmnzopt->path_terminallogfile)
		{ dmnzopt->path_terminallogfile = DEFAULT_OUTPUT_PATH; }

		if(NULL == dmnzopt->cmd)
		{
			fprintf(stderr, "ERR: program command (executable + parameters) is required.\n");
			return 1;
		}
	}
	else if( (ARGHASH_ACT_STOP == dmnzopt->action_code) || (ARGHASH_ACT_STATUS == dmnzopt->action_code) )
	{
		if(NULL == dmnzopt->path_pidfile)
		{
			fprintf(stderr, "ERR: require PID record file.\n");
			return 1;
		}
	}

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
