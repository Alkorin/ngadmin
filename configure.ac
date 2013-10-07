
AC_PREREQ([2.68])
AC_INIT([ngadmin], [0.1], [admin@darkcoven.tk])
AC_CONFIG_MACRO_DIR([.])
AC_CONFIG_SRCDIR([raw/src/attr.c])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE

# check for programs
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_LIBTOOL
AC_PROG_INSTALL

LT_PREREQ([2.4])
LT_INIT


# enable/disable readline
AC_ARG_WITH([readline],
	[AS_HELP_STRING([--with-readline], [support fancy command line editing @<:@default=check@:>@])],
	[with_readline="$withval"], [with_readline=yes])

# enable debug build
AC_ARG_ENABLE(debug,
	[AS_HELP_STRING([--enable-debug], [enable debug mode [default=no]])],
	[enable_debug=yes], [enable_debug=no])

# enable/disable documentation generation
AC_ARG_ENABLE(doc,
	[AS_HELP_STRING([--enable-doc], [enable documentation generation [default=no]])],
	[enable_doc=yes], [enable_doc=no])

# enable/disable build of NgSpy
AC_ARG_ENABLE(spy,
	[AS_HELP_STRING([--enable-spy], [enable NgSpy [default=no]])],
	[enable_spy=yes], [enable_spy=no])
AM_CONDITIONAL(ENABLE_SPY, test x$enable_spy = xyes)

# enable/disable build of NgEmu
AC_ARG_ENABLE(emu,
	[AS_HELP_STRING([--enable-emu], [enable NgEmu [default=no]])],
	[enable_emu=yes], [enable_emu=no])
AM_CONDITIONAL(ENABLE_EMU, test x$enable_emu = xyes)



AS_IF([test "x${enable_doc}" = "xyes"], [
	AC_CHECK_PROGS([DOXYGEN], [doxygen])
	AS_IF([test -z "$DOXYGEN"], [
		AC_MSG_ERROR([Doxygen not found])
	])
])
AM_CONDITIONAL([HAVE_DOXYGEN], [test -n "$DOXYGEN"])


AS_IF([test "x${with_readline}" != "xno"], [
	AC_CHECK_LIB([readline], [readline], [
		AC_SUBST([READLINE_LIBS], [-lreadline])
		AC_DEFINE([HAVE_LIBREADLINE], [1], [Define if you have libreadline])
	], [
		AC_MSG_FAILURE([readline test failed (--without-readline to disable)])
	])
])



CFLAGS="-Wall -Wextra -Os"

if test "x${enable_debug}" = xyes; then
	CFLAGS="$CFLAGS -g"
else
	CFLAGS="$CFLAGS -fomit-frame-pointer"
	LDFLAGS="$LDFLAGS -s"
fi


# check for header files
AC_CHECK_HEADERS([arpa/inet.h stdlib.h string.h sys/ioctl.h termios.h unistd.h])
AC_HEADER_STDBOOL

# check for typedefs, structures, and compiler characteristics
AC_C_INLINE
AC_TYPE_SIZE_T

# check for library functions
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

AM_COND_IF([ENABLE_SPY], [
	AC_CONFIG_FILES([
			spy/Makefile
			spy/man/Makefile
			spy/src/Makefile
		])
])

AM_COND_IF([ENABLE_EMU], [
	AC_CONFIG_FILES([
			emu/Makefile
			emu/man/Makefile
			emu/src/Makefile
		])
])


AC_OUTPUT


echo "
${PACKAGE_NAME} version ${PACKAGE_VERSION}
Prefix.............: ${prefix}
Debug..............: ${enable_debug}
Doc................: ${enable_doc}
Compiler...........: ${CC} ${CFLAGS} ${CPPFLAGS}
Readline suppport..: ${with_readline}
Spy................: ${enable_spy}
Emulator...........: ${enable_emu}
"


