# vim set syntax=config ts=8 sts=2 et:

### SystemC-VPC ###
# --enable-vpc option
AC_ARG_ENABLE(
  [vpc],
  [[  --enable-vpc            enable VPC support [default=auto]]],
  [case "$enableval" in
     yes) enable_vpc=yes  ;;
     no)  enable_vpc=no   ;;
     *)   enable_vpc=auto ;;
   esac],
  [enable_vpc=auto # default]
)

if test x"$enable_vpc" = x"auto" -a -d $srcdir/SystemC-VPC; then
  enable_vpc="yes";
fi
if test x"$enable_vpc" = x"yes"; then
  AC_CONFIG_SUBDIRS([SystemC-VPC])
fi
