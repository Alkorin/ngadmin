
#include "commands.h"


bool do_password_change (int argc, const char **argv, struct ngadmin *nga)
{
	int i;
	const struct swi_attr *sa;
	
	
	if (argc != 1) {
		printf("usage: password change <value>\n");
		return false;
	}
	
	sa = ngadmin_getCurrentSwitch(nga);
	if (sa == NULL) {
		printf("must be logged\n");
		return false;
	}
	
	i = ngadmin_changePassword(nga, argv[0]);
	printErrCode(i);
	
	
	return true;
}


bool do_password_set (int argc, const char **argv, struct ngadmin *nga)
{
	int i;
	char buf[64];
	const char *pass;
	
	
	if (argc > 1) {
		printf("usage: password set [<value>]\n");
		return false;
	}
	
	if (argc == 0) {
		printf("Enter password: ");
		fflush(stdout);
		current_term.c_lflag &= ~ECHO;
		tcsetattr(STDIN_FILENO, TCSANOW, &current_term);
		pass = fgets(buf, sizeof(buf), stdin);
		trim(buf, strlen(buf));
		current_term.c_lflag |= ECHO;
		tcsetattr(STDIN_FILENO, TCSANOW, &current_term);
		putchar('\n');
	} else {
		pass = argv[0];
	}
	
	if (pass != NULL) {
		i = ngadmin_setPassword(nga, pass);
		printErrCode(i);
	}
	
	
	return true;
}


