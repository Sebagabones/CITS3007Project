#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>

void print_env() {
  extern char **environ;
  int i = 0;

  while (environ[i]) {
    printf("%s\n", environ[i++]); // prints in form of "variable=value"
  }
}

int addEnvVal(char *pathKey, char *pathVal) {
  /* returns 0 on success, exits on failure */
  int pwdDirPut = setenv(pathKey, pathVal, 0);

  if (pwdDirPut) {
    fprintf(stderr, "couldn't write value for %s!", pathKey);
    exit(EXIT_FAILURE);
  }

  return (0);
}

void sanitize_env() {
  int colorTermFlip = 0;
  int coloumnsFlip = 0;

  char *pathVar = getenv("PATH");

  if (!pathVar) {
    fprintf(stderr, "couldn't get value for PATH!");
    exit(EXIT_FAILURE);
  }

  char *pwdVar = getenv("PWD");

  if (!pwdVar) {
    fprintf(stderr, "couldn't get value for PWD!");
    exit(EXIT_FAILURE);
  }

  char *colorTermVar = getenv("COLORTERM");
  char *columnsVar = getenv("COLUMNS");

  if (colorTermVar) {
    colorTermFlip = 1;
  }

  if (columnsVar) {
    coloumnsFlip = 1;
  }

  clearenv();

  if (colorTermFlip) {
    addEnvVal("COLORTERM", colorTermVar);
  }

  if (coloumnsFlip) {
    addEnvVal("COLUMNS", "40");
  } else {
    addEnvVal("COLUMNS", "60");
  }

  addEnvVal("PATH", pathVar);
  addEnvVal("PWD", pwdVar);
  /* print_env(); */
}

int main() {
  /* print_env(); */
  sanitize_env();
}
