AC_PREREQ[(2.58)]
AC_INIT([simple-brightness-applet], [1.1])
AC_CONFIG_SRCDIR([src/simple-brightness-applet.c])
AM_INIT_AUTOMAKE
AC_CONFIG_HEADER([config.h])

AC_PROG_CC
AC_PROG_INSTALL
AM_PROG_LIBTOOL

PKG_CHECK_MODULES(BRIGHTNESS, hildon-1 libhildondesktop-1 gtk+-2.0 glib-2.0 gobject-2.0 gconf-2.0 libosso)
AC_SUBST(BRIGHTNESS_CFLAGS)
AC_SUBST(BRIGHTNESS_LIBS)

HILDON_HOME_DESKTOP_DIR=`pkg-config libhildondesktop-1 --variable=hildonstatusmenudesktopentrydir`
AC_SUBST(HILDON_HOME_DESKTOP_DIR)

HILDON_DESKTOP_LIB_DIR=`pkg-config libhildondesktop-1 --variable=hildondesktoplibdir`
AC_SUBST(HILDON_DESKTOP_LIB_DIR) 

AC_OUTPUT([Makefile src/Makefile data/Makefile])
