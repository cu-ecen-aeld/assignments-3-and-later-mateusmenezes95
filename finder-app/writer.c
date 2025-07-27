#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  setlogmask(LOG_UPTO (LOG_DEBUG));
  openlog("writer", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_USER);

	if (strcmp(argv[1], "-h") == 0) {
		printf("Usage: writer [FILE_PATH] [STRING_TO_WRITE_TO]");
    return 0;
	}

  if (argc < 3) {
		syslog(
			LOG_ERR,
			"Expected 2 arguments, but only %d was given. "
			"Run the app with -h or --help for arguments suggestions",
			argc);
		return 1;
  }

  int file_fd;
	char * dir_path = argv[1];
  char * str_to_write = argv[2];

  file_fd = open(dir_path, O_RDWR | O_CREAT | O_TRUNC, 0644);
  if (file_fd == -1) {
    syslog(LOG_ERR, "Error while trying to open the file '%s'", dir_path);
    return 1;
  }

  syslog(LOG_DEBUG, "Writting %s to file %s", str_to_write, dir_path);

  int write_ret = write(file_fd, str_to_write, strlen(str_to_write));
  if (write_ret == -1) {
    perror("Error while writting to file");
    return 1;
  }

  closelog();
  return 0;
}

