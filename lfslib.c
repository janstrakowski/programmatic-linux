int fslib_direntries(lua_State *luast) {
  const char *path = luaL_checkstring(luast, 1);

  lua_newtable(luast);
  luaL_setmetatable(luast, "stdtable");
  DIR *dir = opendir(path);
  if (dir == NULL) {
    luaL_error(luast, "Could not opendir '%s': %s", path, strerror(errno));
  }

  int entord = 0;
  while(1) {
    errno = 0;
    struct dirent* dent = readdir(dir);
    if (dent == NULL) {
      if (errno != 0) {
        luaL_error(luast, "Could not readdir: %s", strerror(errno));
      }
      break;
    }
    entord++;
    lua_pushstring(luast, dent->d_name);
    lua_seti(luast, -2, entord);
  }

  return 1;
}

int filecopy(const char *src, const char *dest) {
  struct stat statbuf;
  if (lstat(src, &statbuf) == -1) {
    return -1;
  }
  if (S_ISREG(statbuf.st_mode)) {
    int srcfd = open(src, O_RDONLY);
    if (srcfd == -1) {
      return -1;
    }
    int destfd = open(dest, O_WRONLY | O_CREAT, 0666);
    if (destfd == -1) {
      return -1;
    }
    off_t bytesleft = statbuf.st_size;
    while (1) {
      ssize_t bytestransferred = sendfile(destfd, srcfd, NULL, bytesleft);
      if (bytestransferred == -1) {
        return -1;
      }
      if (bytesleft == bytestransferred) {
        break;
      }
      bytesleft -= bytestransferred;
    }
  } else if (S_ISDIR(statbuf.st_mode)) {
    DIR *dirstream = opendir(src);
    if (dirstream == NULL) {
      return -1;
    }
    if (mkdir(dest, 0777) == -1) {
      if (errno != EEXIST) {
        return -1;
      }
    }

    while (1) {
      errno = 0;
      struct dirent *dent = readdir(dirstream);
      if (dent == NULL) {
        if (errno == 0) {
          break;
        }
        return -1;
      }
      if (strcmp(dent->d_name, ".") == 0) {
        continue;
      }
      if (strcmp(dent->d_name, "..") == 0) {
        continue;
      }
      char newsrc[PATH_MAX];
      char newdest[PATH_MAX];
      if (strlen(src) + 1 + strlen(dent->d_name) > PATH_MAX) {
        errno = ENAMETOOLONG;
        if (closedir(dirstream) == -1) {
          return -1;
        }
        return -1;
      }
      if (strlen(dest) + 1 + strlen(dent->d_name) > PATH_MAX) {
        errno = ENAMETOOLONG;
        if (closedir(dirstream) == -1) {
          return -1;
        }
        return -1;
      }
      strcpy(newsrc, src);
      strcat(newsrc, "/");
      strcat(newsrc, dent->d_name);
      strcpy(newdest, dest);
      strcat(newdest, "/");
      strcat(newdest, dent->d_name);
      if (filecopy(newsrc, newdest) == -1) {
        if (closedir(dirstream) == -1) {
          return -1;
        }
        return -1;
      }
    }
    if (closedir(dirstream) == -1) {
      return -1;
    }
  } else if (S_ISLNK(statbuf.st_mode)) {
    char tgtpath[PATH_MAX + 1];
    ssize_t bytesplaced = readlink(src, tgtpath, PATH_MAX + 1);
    if (bytesplaced == -1) {
      return -1;
    }
    if (bytesplaced == PATH_MAX + 1) {
      errno = ENAMETOOLONG;
      return -1;
    }
    if (symlink(tgtpath, dest) == -1) {
      return -1;
    }
  } else {
    errno = EINVAL;
    return -1;
  }
}

int fs_copy(lua_State *L) {
  const char *src = luaL_checkstring(L, 1);
  const char *dest = luaL_checkstring(L, 2);

  if (filecopy(src, dest) == -1) {
    pusherrno(L, errno);
    lua_error(L);
  }

  return 0;
}

int fs_rename(lua_State *L) {
  const char *oldpath = luaL_checkstring(L, 1);
  const char *newpath = luaL_checkstring(L, 2);

  if (rename(oldpath, newpath) == -1) {
    pusherrno(L, errno);
    lua_error(L);
  }

  return 0;
}

int fs_mkdir(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  mode_t mode = 0777;
  if (lua_gettop(L) > 1) {
    mode = luaL_checkinteger(L, 2);
  }

  if (mkdir(path, mode) == -1) {
    pusherrno(L, errno);
    lua_error(L);
  }

  return 0;
}

const luaL_Reg luaapi_fslib[] = {
  {"direntries", &fslib_direntries},
  {"copy", &fs_copy},
  {"rename", &fs_rename},
  {"mkdir", &fs_mkdir},
  {NULL, NULL}
};

int luaapi_fslib_register(lua_State *luast) {
  luaL_newlib(luast, luaapi_fslib);
  lua_setglobal(luast, "fs");
}
