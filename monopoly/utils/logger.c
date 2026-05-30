#include "logger.h"

void logger(char *message) {
  if (!isLoggingEnabled()) {
    return;
  }

  FILE *file =
      fopen("/dev/pts/5", "w"); // enter tty in terminal to find out the number
  fprintf(file, "LOG: %s\n", message);
  fclose(file);
}
