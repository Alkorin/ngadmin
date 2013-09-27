
AC_PREREQ([2.68])
AC_INIT([ngadmin], [0.1], [admin@darkcoven.tk])
AC_CONFIG_MACRO_DIR([.])
AC_CONFIG_SRCDIR([raw/src/attr.c])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE

LT_PREREQ([2.4])
LT_INIT

# Checks for programs
AC_PROG_CC
AM_PROG_CC_C_O

# Checks for libraries
AC_CHECK_LIB([readline], [readline])

# Checks for header files
AC_CHECK_HEADERS([arpa/inet.h stdlib.h string.h sys/ioctl.h termios.h unistd.h])
AC_HEADER_STDBOOL

# Checks for typedefs, structures, and compiler characteristics
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions
AC_FUNC_MALLOC
AC_CHECK_FUNCS([inet_ntoa memchr memset select socket strcasecmp strdup strtol strtoul])


AC_CONFIG_FILES([
	Makefile
	raw/Makefile
	raw/include/Makefile
	raw/src/Makefile
	lib/Makefile
	lib/include/Makefile
	lib/src/Makefile
	cli/Makefile
	cli/man/Makefile
	cli/src/Makefile
])

AC_OUTPUT

