#include <stdio.h>
#include <syslog.h>

int main(int argc, char *argv[]) {
  setlogmask(LOG_UPTO (LOG_NOTICE));
  openlog(argv[0], LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
  if (argc < 3) {
	syslog(LOG_ERR, "Expected 2 arguments, but only %d was given. Run the app with -h or --help for arguments suggestions", argc);
	return 1;
  }
  closelog();
  return 0;
}

