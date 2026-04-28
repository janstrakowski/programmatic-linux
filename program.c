# define _GNU_SOURCE

# include <dirent.h>
# include <errno.h>
# include <limits.h>
# include <linux/sched.h>
# include "lua/lua.h"
# include "lua/lauxlib.h"
# include "lua/lualib.h"
# include <sched.h>
# include <stdarg.h>
# include <stdlib.h>
# include <stdio.h>
# include <stdint.h>
# include <string.h>
# include <sys/mount.h>
# include <sys/sendfile.h>
# include <sys/stat.h>
# include <sys/syscall.h>
# include <sys/wait.h>
# include <unistd.h>

# include "header.h"

# include "common.c"
# include "lfslib.c"
# include "lxiolib.c"

#define intsrcpath "/src"
#define intdestpath "/dest"

#define FREADN 2048

struct freaderdata {
  int fd;
  char buf[FREADN];
  int ierrno;
};

const char *filereader(lua_State *L, void *data, size_t *size) {
  struct freaderdata *mydata = (struct freaderdata *) data;
  ssize_t bytesread = read(mydata->fd, mydata->buf, FREADN);
  if (bytesread == -1) {
    mydata->ierrno = errno;
    return 0;
  }
  mydata->ierrno = 0;
  *size = bytesread;
  return mydata->buf;
}

uint32_t main(uint32_t argc, char* argv[]) {
  // Parse CLI input
  int32_t opt;

  char srcpath[PATH_MAX] = "plsrc";
  char destpath[PATH_MAX] = "pldest";
  char buildpath[PATH_MAX] = "build";

  while(1){
    opt = getopt(argc, argv, "s:d:b:");
    if (opt == 's') {
      if (strlen(optarg) > PATH_MAX) {
        fprintf(stderr, "the source-path CLI argument given exceeds the maximum path length.");
        exit(1);
      }
      strcpy(srcpath, optarg);
      continue;
    }
    if (opt == 'd') {
      if (strlen(optarg) > PATH_MAX) {
        fprintf(stderr, "the destination-path CLI argument given exceeds the maximum path length.");
        exit(1);
      }
      strcpy(destpath, optarg);
      continue;
    }
    if (opt == 'b') {
      if (strlen(optarg) > PATH_MAX) {
        fprintf(stderr, "the build-environment-path CLI argument given exceeds the maximum path length.");
        exit(1);
      }
      strcpy(buildpath, optarg);
      continue;
    }
    break;
  }

  char wpscriptpath[PATH_MAX];
  if (strlen(srcpath) + sizeof("/build.lua")-1 >= PATH_MAX) {
    errno = ENAMETOOLONG;
    fprintf(stderr, "%s", strerror(errno));
    exit(errno);
  }
  strcpy(wpscriptpath, srcpath);
  strcat(wpscriptpath, "/build.lua");
  struct freaderdata frd;
  frd.fd = open(wpscriptpath, O_RDONLY);
  if (frd.fd == -1) {
    fprintf(stderr, "%s", strerror(errno));
    exit(errno);
  }
  if (build(destpath, buildpath, srcpath, NULL, &filereader, &frd, wpscriptpath, NULL) == -1) {
    fprintf(stderr, "%s", strerror(errno));
    exit(errno);
  }
  return 0;
}
