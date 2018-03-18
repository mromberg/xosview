AC_DEFUN(XO_CXX_THREAD_SLEEP, [
  # check for working support for std::this_thread::sleep_for()
  AC_CHECK_HEADERS([thread])
  if test "$ac_cv_header_thread" = "yes"; then
    AC_MSG_CHECKING([for std::this_thread::sleep_for])
    AC_COMPILE_IFELSE([
    AC_LANG_SOURCE(
        [[
            #include <thread>
            #include <chrono>
            int main() {
                std::this_thread::sleep_for(std::chrono::microseconds(1));
                return 0;
            }
        ]]
    )],
    [
        AC_MSG_RESULT(yes)
        AC_DEFINE(HAVE_TRHEAD_SLEEP,[1],[Have std::thread::sleep_for])
        sleep_for=yes
    ],
    [
        AC_MSG_RESULT(no)
        sleep_for=no
    ])
  else
      #  Check for usleep().  Currently, only HP-UX doesn't have it.
      AC_CHECK_FUNCS(usleep)
  fi
])
