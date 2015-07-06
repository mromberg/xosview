#!/bin/sh
#  Check that certain unsafe things aren't in any of the
#  code.
echo "***  Grep'ing for uses of strcmp, sprintf, strcat, strcmp,"
echo "     strcasecmp instead of safer (but now depricated)"
echo "     strncmp, snprintf, strncat,strncmp, strncasecmp...."
#  If egrep finds no matches, it returns as if it is an
#  error.  Reverse that error code....
#  We also should not check snprintf.cc, which calls vsprintf.

egrep "strcmp|sprintf|strcat|strcmp|strcasecmp" \
		`echo *.cc *.h */*.cc */*.h`;
if [ $? == 0 ]; then
    echo "***  Bad functions found -- error!";
    exit 1;
fi

echo "Depricated string functions used in:"
egrep -l "strncmp|snprintf|strncat|strncmp|strncasecmp" \
      `echo *.cc *.h */*.cc */*.h`;
if [ $? == 0 ]; then
    echo "Convert to std::string"
fi

exit 0
