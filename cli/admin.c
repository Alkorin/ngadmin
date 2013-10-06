
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <setjmp.h>

#include <getopt.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "common.h"
#include "commands.h"


#define MAXCOM	32


int main_loop_continue = 1;


static const struct TreeNode* getSubCom (char **com, int n, int *t)
{
	int i;
	const struct TreeNode *cur, *next;
	
	
	cur = &commands;
	for (i = 0; i < n; i++) {
		/* we have reached a terminal command, exit */
		if (cur->sub == NULL)
			break;
		
		/* search sub command in sub command array */
		for (next = cur->sub; next->name != NULL && strcmp(next->name, com[i]) != 0; next++);
		
		/* sub command not found, exit */
		if (next->name == NULL)
			break;
		
		/* next command is now the current one */
		cur = next;
	}
	
	*t = i;
	
	
	return cur;
}


static const struct TreeNode *compcur;


static char* my_generator (const char* text, int state)
{
	static int len;
	static const struct TreeNode *tn;
	const char *name;
	
	
	if (compcur == NULL) {
		/* sub command not found */
		return NULL;
	} else if (state == 0) {
		tn = compcur->sub;
		len = strlen(text);
	}
	
	if (tn == NULL) /* terminal command */
		return NULL;
	
	while ((name = tn++->name) != NULL) {
		if (strncmp(name, text, len) == 0)
			return strdup(name);
	}
	
	
	return NULL;
}


static char** my_completion (const char *text, int start, int end UNUSED)
{
	char **matches = NULL;
	char *line, *com[MAXCOM];
	int i, n;
	
	
	memset(com, 0, MAXCOM * sizeof(char*));
	line = strdup(rl_line_buffer);
	line[start] = '\0';
	trim(line, start);
	n = explode(line, com, MAXCOM);
	free(line);
	
	compcur = getSubCom(com, n, &i);
	
	if (i < n)
		compcur = NULL;
	matches = rl_completion_matches(text, my_generator);
	
	for (i = 0; com[i] != NULL; i++)
		free(com[i]);
	
	
	return matches;
}


static struct ngadmin *nga;
static sigjmp_buf jmpbuf;
static struct termios orig_term;
struct termios current_term;
static bool batch;


NORET static void handler (int sig)
{
	switch (sig) {
	
	case SIGTERM:
	case SIGINT:
		printf("interrupt\n");
		
		current_term.c_lflag |= ECHO;
		tcsetattr(STDIN_FILENO, TCSANOW, &current_term);
		
		if (!batch)
			siglongjmp(jmpbuf, 1);
	
	default:
		ngadmin_close(nga);
		
		tcsetattr(STDIN_FILENO, TCSANOW, &orig_term);
		
		exit(0);
	}
}


static int pre_login (const struct ether_addr *mac, int retries)
{
	const struct swi_attr *sa;
	int i, n, err;
	
	
	for (i = 1; retries <= 0 || i <= retries; i++) {
		/* scan */
		printf("scan... ");
		fflush(stdout);
		err = ngadmin_scan(nga);
		if (err < 0) {
			printErrCode(err);
			continue;
		}
		
		/* search switch with requested MAC */
		sa = ngadmin_getSwitchTab(nga, &n);
		while (--n >= 0) {
			if (memcmp(mac, &sa[n].mac, ETH_ALEN) == 0)
				break;
		}
	
		if (n < 0) {
			printf("no switch found\n");
		} else {
			printf("done\n");
			break;
		}
	}
	
	if (n < 0)
		return 1;

	/* login */
	printf("login... ");
	fflush(stdout);
	err = ngadmin_login(nga, n);
	if (err < 0)
		printErrCode(err);
	else
		printf("done\n");
	
	return err;
}


int main (int argc, char **argv)
{
	static const struct option opts[] = {
		{"batch", no_argument, NULL, 'a'},
		{"keep-broadcasting", no_argument, NULL, 'b'},
		{"force-interface", no_argument, NULL, 'f'},
		{"help", no_argument, NULL, 'h'},
		{"interface", required_argument, NULL, 'i'},
		{"local-broadcast", no_argument, NULL, 'l'},
		{"mac", required_argument, NULL, 'm'},
		{"password", required_argument, NULL, 'p'},
		{"retries", required_argument, NULL, 'r'},
		{"timeout", required_argument, NULL, 't'},
		{0, 0, 0, 0}
	};
	char *line, *com[MAXCOM];
	const char *iface = "eth0", *password = NULL;
	float timeout = 0.f;
	bool kb = false, force = false, global = true;
	struct timeval tv;
	const struct TreeNode *cur, *next;
	struct ether_addr *mac = NULL;
	int i, n, retries = 3;
	
	
	tcgetattr(STDIN_FILENO, &orig_term);
	current_term = orig_term;
	batch = false;
	
	opterr = 0;
	
	while ((n = getopt_long(argc, argv, "abfhi:lm:p:r:t:", opts, NULL)) != -1) {
		switch (n) {
		
		case 'a':
			batch = true;
			break;
		
		case 'b':
			kb = true;
			break;
		
		case 'f':
			force = true;
			break;
		
		case 'h':
			printf("usage: %s [-a] [-b] [-f] [-g] [-i <interface>] [-m <MAC>] [-p <password>]\n", argv[0]);
			goto end;
		
		case 'i':
			iface = optarg;
			break;
		
		case 'l':
			global = false;
			break;
		
		case 'm':
			mac = ether_aton(optarg);
			if (mac == NULL) {
				printf("invalid MAC\n");
				goto end;
			}
			break;
		
		case 'p':
			password = optarg;
			break;
		
		case 'r':
			retries = strtol(optarg, NULL, 0);
			break;
		
		case 't':
			timeout = strtof(optarg, NULL);
			break;
		
		case '?':
			printf("unknown option: \"%s\"\n", argv[optind - 1]);
			goto end;
		}
	}
	
	argc -= optind;
	argv += optind;
	
	if (argc != 0) {
		printf("unknown trailing options\n");
		goto end;
	}
	
	
	memset(com, 0, MAXCOM * sizeof(char*));
	
	nga = ngadmin_init(iface);
	if (nga == NULL) {
		fprintf(stderr, "initialization error\n");
		goto end;
	}
	
	/* set timeout */
	if (timeout > 0.f) {
		tv.tv_sec = (int)timeout;
		tv.tv_usec = (int)((timeout - (float)tv.tv_sec) * 1.e6f);
		ngadmin_setTimeout(nga, &tv);
	}
	
	
	if (ngadmin_setKeepBroadcasting(nga, kb) != ERR_OK)
		goto end;
	
	if (force && ngadmin_forceInterface(nga) != ERR_OK)
		goto end;
	
	if (ngadmin_useGlobalBroadcast(nga, global) != ERR_OK)
		goto end;
	
	/* non-TTY inputs are automatically set to batch mode */
	if (!isatty(STDIN_FILENO))
		batch = true;
	
	if (password != NULL)
		ngadmin_setPassword(nga, password);
	
	signal(SIGTERM, handler);
	signal(SIGINT, handler);
	
	/* automatic scan & login when switch MAC is specified on the command line */
	if (mac != NULL && pre_login(mac, retries) != 0)
		goto end;
	
	if (batch) {
		/* in batch mode, we must be logged to continue */
		if (ngadmin_getCurrentSwitch(nga) == NULL) {
			printf("must be logged\n");
			goto end;
		}
	} else {
		/* initialize readline functions */
		rl_attempted_completion_function = my_completion;
		rl_completion_entry_function = my_generator;
		
		sigsetjmp(jmpbuf, 1);
	}
	
	while (main_loop_continue) {
		/* read user input */
		line = NULL;
		n = 0;
		if (batch)
			n = getline(&line, (size_t*)&i, stdin);
		else
			line = readline("> ");
		if (n < 0 || line == NULL)
			goto end;
		
		/* split string into words */
		trim(line, strlen(line));
		n = explode(line, com, MAXCOM);
		
		if (n == 0) {
			free(line);
			continue;
		} else {
			if (!batch)
				add_history(line);
			free(line);
		}
		
		cur = getSubCom(com, n, &i);
		
		if (cur->sub != NULL) {
			/* not terminal command */
			if (i == 0) {
				/* root command */
				printf("unknown command: %s\n", com[i]);
			} else if (i < n) {
				/* intermediate command, remaining string */
				printf("unknown %s subcommand: %s\n", com[i - 1], com[i]);
			} else {
				/* intermediate command, no remaining string */
				/* print available subcommands */
				for (next = cur->sub; next->name != NULL; next++)
					printf("%s ", next->name);
				putchar('\n');
			}
		} else if (cur->comfunc == NULL) {
			/* erroneous terminal command without function */
			printf("terminal command without function\n");
		} else {
			/* execute terminal command */
			cur->comfunc(n - i, (const char**)&com[i], nga);
		}
		
		for (i = 0; com[i] != NULL; i++) {
			free(com[i]);
			com[i] = NULL;
		}
	}
	
end:
	handler(0);
}


