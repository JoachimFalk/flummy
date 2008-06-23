# vim set syntax=config ts=8 sts=2 et:

### SystemC-VPC ###
# --enable-vpc option
AC_ARG_ENABLE(
  [vpc],
  [[  --enable-vpc            enable VPC support [default=yes]]],
  [case "$enableval" in
     yes) enable_vpc=yes ;;
     no)  enable_vpc=no  ;;
     *)   enable_vpc=no  ;;
   esac],
  [enable_vpc=yes # default]
)

if test x"$enable_vpc" = x"yes"; then
  AC_CONFIG_SUBDIRS([SystemC-VPC])
  dnl AC_CONFIG_SUBDIRS(ACJF_VAR_SUBPROJECT_DIR)
fi
