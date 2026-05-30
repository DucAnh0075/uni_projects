#include "fileReader.h"

char *readFile(const char *filename) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    perror("Error opening file");
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  long filesize = ftell(file);
  rewind(file);

  char *buffer = (char *)malloc(filesize + 1);
  if (!buffer) {
    perror("Memory allocation failed");
    fclose(file);
    return NULL;
  }

  size_t read_size = fread(buffer, 1, filesize, file);
  if (read_size != filesize) {
    perror("Error reading file");
    free(buffer);
    fclose(file);
    return NULL;
  }

  buffer[filesize] = '\0';
  fclose(file);
  return buffer;
}

struct Size getArtworkSize(const char *artwork) {
  int maxWidth = 0;
  int height = 1;

  int currentWidth = 0;
  for (int i = 0; artwork[i] != '\0'; i++) {
    if (artwork[i] == '\n') {
      height++;
      if (currentWidth > maxWidth) {
        maxWidth = currentWidth;
      }
      currentWidth = 0;
    } else {
      currentWidth++;
    }
  }

  return (struct Size){.height = height, .width = maxWidth};
}
