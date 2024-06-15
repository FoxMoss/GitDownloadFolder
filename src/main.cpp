#include <argparse/argparse.hpp>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <git2.h>
#include <git2/checkout.h>
#include <git2/clone.h>
#include <git2/global.h>
#include <git2/repository.h>
#include <git2/strarray.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#define PROGRAM_NAME "gdf"
#define STRINGIFY(x) #x

char *tmp_folder;
char *absolute_src;
git_repository *new_repo = NULL;

void cleanup() {
  try {
    std::filesystem::remove_all(tmp_folder);
  } catch (const std::exception &err) {
    printf("%s: %s\n", PROGRAM_NAME, err.what());
  }
  free(tmp_folder);
  free(absolute_src);
  git_repository_free(new_repo);
  git_libgit2_shutdown();
}

enum RETURN_TYPE { RETURN_POSITIVE, RETURN_ZERO };

void unwrap_exit(int exit_value, const char *exit_str,
                 RETURN_TYPE return_type = RETURN_POSITIVE) {
  bool should_exit = false;
  switch (return_type) {
  case RETURN_POSITIVE:
    should_exit = exit_value < 0;
    break;
  case RETURN_ZERO:
    should_exit = exit_value != 0;
    break;
  }
  if (should_exit) {
    printf("%s: %s\n", PROGRAM_NAME, exit_str);
    cleanup();
    exit(1);
  }
}

int main(int argc, char *argv[]) {
  srand(time(NULL));
  argparse::ArgumentParser arg_parser(PROGRAM_NAME, "Hash " GIT_HASH);

  arg_parser.add_description(
      "GitDownloadFolder, something that should be builtin to git.\n\n"
      "Build of hash " GIT_HASH);

  arg_parser.add_argument("url").required().help(
      "The git repo you want to download the folder from.");
  arg_parser.add_argument("folder").required().help(
      "The folder of the repo to download.");
  arg_parser.add_argument("path").default_value(".").help(
      "The destination of the folder.");
  arg_parser.add_argument("-vv", "--verbose")
      .nargs(0)
      .default_value(false)
      .implicit_value(true)
      .help("Prints extra debug info.");

  try {
    arg_parser.parse_args(argc, argv);
  } catch (const std::exception &err) {
    unwrap_exit(-1, err.what());
  }

  if (*(arg_parser.get<std::string>("path").end() - 1).base() == '/' ||
      *(arg_parser.get<std::string>("folder").end() - 1).base() == '/') {
    unwrap_exit(-1, "Path and exit arguments need no trailing slash.");
  }

  const size_t tmp_folder_size = sizeof(char) * strlen("/tmp/gdf-") +
                                 sizeof(char) * strlen(STRINGIFY(RAND_MAX)) +
                                 sizeof(char) * 5;
  tmp_folder = (char *)malloc(tmp_folder_size);
  unwrap_exit(snprintf(tmp_folder, tmp_folder_size, "/tmp/gdf-%d", rand()),
              "Failed to generate temp folder name.", RETURN_POSITIVE);

  unwrap_exit(mkdir(tmp_folder, 0700), "Failed to make temp folder.");
  if (arg_parser.get<bool>("--verbose")) {
    printf("Created temp repo %s\n", tmp_folder);
  }

  git_libgit2_init();
  git_clone_options opts = {};
  git_clone_options_init(&opts, 1);

  opts.checkout_opts.checkout_strategy = GIT_CHECKOUT_FORCE;

  auto path = arg_parser.get<std::string>("folder");

  const char *paths[] = {path.c_str()};
  git_strarray checkout_paths = {(char **)paths, 1};
  opts.checkout_opts.paths = checkout_paths;
  opts.checkout_opts.paths.count = 1;

  unwrap_exit(git_clone(&new_repo, arg_parser.get<std::string>("url").c_str(),
                        tmp_folder, &opts),
              "Failed to clone repo.");

  absolute_src = (char *)malloc(tmp_folder_size + path.size());
  unwrap_exit(snprintf(absolute_src, tmp_folder_size + path.size(), "%s/%s",
                       tmp_folder, path.c_str()),
              "Failed to generate temp folder src path name.", RETURN_POSITIVE);

  auto final_dest = (arg_parser.get<std::string>("path") + "/" +
                     arg_parser.get<std::string>("folder"));
  if (std::filesystem::exists(final_dest)) {

    char *exists_error = (char *)malloc(
        strlen("Destination  already exists. ") + final_dest.size());
    sprintf(exists_error, "Destination %s already exists.", final_dest.c_str());
    // "leaking memory is a safe opperation"
    unwrap_exit(-1, exists_error);
  }
  mkdir(final_dest.c_str(), 0700);

  if (arg_parser.get<bool>("--verbose")) {
    printf("Copying to %s\n", final_dest.c_str());
  }

  std::filesystem::copy(std::filesystem::path(absolute_src),
                        std::filesystem::path(final_dest));

  cleanup();
  return 0;
}
