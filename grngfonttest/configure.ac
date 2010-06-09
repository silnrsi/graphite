AC_PREREQ(2.54)
# ---------------------------------------------------------------------
# Initialisation
# ---------------------------------------------------------------------
AC_INIT(testGraphiteFontFile, 2.0.0, silgraphite-devel@lists.sourceforge.net)
AC_CONFIG_SRCDIR(grngFontTest.cpp)
AC_CONFIG_AUX_DIR(config)
AC_REVISION($Revision: 1.5 $)
AM_INIT_AUTOMAKE(foreign)
AM_MAINTAINER_MODE

# ---------------------------------------------------------------------
# Check Programs
# ---------------------------------------------------------------------
AC_LANG_CPLUSPLUS
AC_PROG_INSTALL
AC_HEADER_STDC
AC_CHECK_FUNCS([iconv strtol strtof])

# ---------------------------------------------------------------------
# Enable options
# ---------------------------------------------------------------------
AC_ARG_ENABLE(manage-tables, AC_HELP_STRING([--enable-manage-tables],[build with app owned table support. (default=no)]),
    ,enable_manage_tables=no)
AC_ARG_ENABLE(debug, AC_HELP_STRING([--enable-debug],[build with debuging symbols. (default=no)]),
    ,enable_debug=no)
AC_ARG_ENABLE(profile, AC_HELP_STRING([--enable-profile],[allow profiling (default=no)]),
    ,enable_profile=no)
AC_ARG_ENABLE(profilefn, AC_HELP_STRING([--enable-profilefn],[allow functioncheck profiling (default=no)]),
    ,enable_profilefn=no)
AM_MAINTAINER_MODE

# ---------------------------------------------------------------------
# Debug and profile
# ---------------------------------------------------------------------

build_flags=""
if test x$enable_manage_tables = xyes; then
    build_flags="$build_flags -DMANAGE_MEM"
fi

if test "$enable_debug" = yes; then
  build_flags="$build_flags -O0 -g -Wall -Wno-unknown-pragmas -Wparentheses -Werror"
else
  build_flags="$build_flags -DNDEBUG"
fi

# Enable Graphite Trace logs - these are only used if a 
# LayoutEnvironment has a valid ostream pointer
if test "$enable_tracing" = yes; then
  build_flags="$build_flags -DTRACING"
fi

# Enable final
if test "$enable_final" = yes; then
  build_flags="$build_flags -O3 -DNDEBUG"
fi

# Enable profile
if test "$enable_profile" = yes; then
  build_flags="$build_flags -pg "
fi
if test "$enable_profilefn" = yes; then
  build_flags="$build_flags -g -finstrument-functions "
  LIBS="$LIBS -g -finstrument-functions -lfnccheck "
fi

CFLAGS="$CFLAGS $build_flags"
CXXFLAGS="$CXXFLAGS $build_flags"

# ---------------------------------------------------------------------
# Find pkg-config
# ---------------------------------------------------------------------
AC_PATH_PROG(PKG_CONFIG, pkg-config, no)
if test x$PKG_CONFIG = xno ; then
  AC_MSG_ERROR([*** pkg-config not found. See http://www.freedesktop.org/software/pkgconfig/])
fi

#if $PKG_CONFIG --atleast-pkgconfig-version 0.14 ; then
#  PKG_CHECK_MODULES(GR, silgraphite >= 2.2.0)
#else
#  AC_MSG_ERROR([*** pkg-config too old; version 0.14 or better required.])
#fi


# ---------------------------------------------------------------------
# Final output
# ---------------------------------------------------------------------
AC_CONFIG_FILES(Makefile)
AC_OUTPUT
