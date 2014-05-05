# pigeonhole.m4 - Check presence of pigeonhole -*-Autoconf-*-
#.

# serial 1

AC_DEFUN([DC_PIGEONHOLE],[
	AC_ARG_WITH(pigeonhole,
	[  --with-pigeonhole=DIR   Pigeonhole base directory],
	pigeonholedir="$withval",
	[
		pg_prefix=$prefix
		test "x$pg_prefix" = xNONE && pg_prefix=$ac_default_prefix
		pigeonholedir="$pg_prefix/include/dovecot/sieve"
	]
	)

	AC_MSG_CHECKING([for pigeonhole in "$pigeonholedir"])

	top=`pwd`
	cd $pigeonholedir
	pigeonholedir=`pwd`
	cd $top
	AC_SUBST(pigeonholedir)

	PIGEONHOLE_TESTSUITE=
	if test -f "$pigeonholedir/src/lib-sieve/sieve.h"; then
		AC_MSG_RESULT([found])
		pigeonhole_incdir="$pigeonholedir"
		LIBSIEVE_INCLUDE='\
			-I$(pigeonhole_incdir) \
			-I$(pigeonhole_incdir)/src/lib-sieve \
			-I$(pigeonhole_incdir)/src/lib-sieve/util \
			-I$(pigeonhole_incdir)/src/lib-sieve/plugins/copy \
			-I$(pigeonhole_incdir)/src/lib-sieve/plugins/enotify \
			-I$(pigeonhole_incdir)/src/lib-sieve/plugins/variables'
		if test -f "$pigeonholedir/src/testsuite/testsuite"; then
			PIGEONHOLE_TESTSUITE="${pigeonholedir}/src/testsuite/testsuite"
  		fi
	elif test -f "$pigeonholedir/sieve.h"; then
		AC_MSG_RESULT([found])
		pigeonhole_incdir="$pigeonholedir"
		LIBSIEVE_INCLUDE='-I$(pigeonhole_incdir)'
	else
		AC_MSG_RESULT([not found])
		AC_MSG_NOTICE([
			Pigeonhole Sieve headers not found from $pigeonholedir and they
			are not installed in the Dovecot include path, use --with-pigeonhole=PATH
 			to give path to Pigeonhole sources or installed headers.])
		AC_MSG_ERROR([pigeonhole not found])
	fi

	DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS --with-pigeonhole=$pigeonholedir"
	
	AM_CONDITIONAL(PIGEONHOLE_TESTSUITE_AVAILABLE, ! test -z "$PIGEONHOLE_TESTSUITE")

	pigeonhole_incdir="$pigeonholedir"

	AC_ARG_ENABLE(valgrind,
	[AC_HELP_STRING([--enable-valgrind], [Enable Valgrind memory leak checks in testsuite [default=no]])],
	    if test x$enableval = xno || test x$enableval = xauto; then
    	    want_valgrind=$enableval
	    else
    	    want_valgrind=yes
	    fi,
	want_valgrind=no)
	AM_CONDITIONAL(PIGEONHOLE_TESTSUITE_VALGRIND, test "$want_valgrind" = "yes")

	AC_SUBST(pigeonhole_incdir)

	AC_SUBST(LIBSIEVE_INCLUDE)
	AC_SUBST(PIGEONHOLE_TESTSUITE)
])
