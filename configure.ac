AC_INIT([subvertx], [0.1], [genjix@riseup.net])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([-Wall -Werror])
LT_INIT
AC_PROG_CXX
CHECK_GXX_CPP0X
CFLAGS="-ggdb"
CXXFLAGS="-ggdb"
my_CXXFLAGS="$ac_cv_cxx_cpp0x_flag -Wall -pedantic -pthread -Wextra -fstack-protector-all"
AC_SUBST([my_CXXFLAGS])
AX_BOOST_BASE
#AX_BOOST_SYSTEM
#AX_BOOST_THREAD
PKG_CHECK_MODULES([SSL], [libssl >= 0.9])
PKG_CHECK_MODULES([libbitcoin], [libbitcoin])
AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT

