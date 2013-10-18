
#include "commands.h"


int do_password_change (int argc, const char **argv, struct ngadmin *nga)
{
	int i;
	char buf[64];
	const struct swi_attr *sa;
	const char *pass;
	
	
	if (argc > 1) {
		printf("usage: password change [<value>]\n");
		return 1;
	}
	
	sa = ngadmin_getCurrentSwitch(nga);
	if (sa == NULL) {
		printf("must be logged\n");
		return 1;
	}
	
	if (argc == 0) {
		printf("Enter new password: ");
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
		i = ngadmin_changePassword(nga, pass);
		printErrCode(i);
	}
	
	
	return 0;
}


int do_password_set (int argc, const char **argv, struct ngadmin *nga)
{
	int i;
	char buf[64];
	const char *pass;
	
	
	if (argc > 1) {
		printf("usage: password set [<value>]\n");
		return 1;
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
	
	
	return 0;
}


