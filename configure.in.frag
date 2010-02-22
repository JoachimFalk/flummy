# vim set syntax=config ts=8 sts=2 et:

if test x"$enable_vpc" = x"yes"; then
  AC_CONFIG_SUBDIRS([SystemC-VPC])
fi
