
#include "commands.h"


bool do_password_change (int nb, const char **com, struct ngadmin *nga)
{
	int i;
	const struct swi_attr *sa;
	
	
	if (nb != 1) {
		printf("Usage: password change <value>\n");
		return false;
	}
	
	sa = ngadmin_getCurrentSwitch(nga);
	if (sa == NULL) {
		printf("must be logged\n");
		return false;
	}
	
	i = ngadmin_changePassword(nga, com[0]);
	printErrCode(i);
	
	
	return true;
}


bool do_password_set (int nb, const char **com, struct ngadmin *nga)
{
	int i;
	char buf[64];
	const char *pass;
	
	
	if (nb > 1) {
		printf("Usage: password set [<value>]\n");
		return false;
	}
	
	if (nb == 0) {
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
		pass = com[0];
	}
	
	if (pass != NULL) {
		i = ngadmin_setPassword(nga, pass);
		printErrCode(i);
	}
	
	
	return true;
}


