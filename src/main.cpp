#include <argparse/argparse.hpp>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PROGRAM_NAME "gdf"

void unwrap_exit(int exit_value, const char *exit_str) {
  if (exit_value != 0) {
    printf("%s: %s\n", PROGRAM_NAME, exit_str);
    exit(-1);
  }
}

int main(int argc, char *argv[]) {
  argparse::ArgumentParser arg_parser(PROGRAM_NAME, "Hash " GIT_HASH);

  arg_parser.add_description(
      "GitDownloadFolder, something that should be builtin to git.\n\n"
      "Build of hash " GIT_HASH);

  return 0;
}
