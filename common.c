uint32_t mkdirs(const char *path, mode_t mode) {
  const char *lastslashpos;
  char currentpath[PATH_MAX];
  while (1) {
    lastslashpos = strchr(path, '/');
    if (lastslashpos == NULL) {
      strcpy(currentpath, path);
    } else {
      strncpy(currentpath, path, (int32_t) (lastslashpos - path));
      currentpath[(int32_t) (lastslashpos - path)] = '\0';
    }
    
    struct stat finfo;
    if (stat(currentpath, &finfo) == -1) {
      if (errno != ENOENT) {
        return -1;
      } else {
        if (mkdir(currentpath, mode) == -1) {
          return -1;
        }
      }
    } else {
      if (!S_ISDIR(finfo.st_mode)) {
        errno = ENOTDIR;
        return -1;
      }
    }

    if (lastslashpos == NULL) {
      break;
    }
    lastslashpos++;
  }
  return 0;
}

void *lnullunwrp(void *value) {
  if (value == luanull) {
    return NULL;
  }
  return value;
}

const void *lnullwrp(void *value) {
  if (value == NULL) {
    return luanull;
  }
  return value;
}

int stdtbltostring(lua_State *ls) {
  luaL_checktype(ls, 1, LUA_TTABLE);
  luaL_Buffer b;
  luaL_buffinit(ls, &b);
  luaL_addstring(&b, "{");
  int index = 1;
  while (1) {
    int valtype = lua_geti(ls, 1, index);
    if (valtype == LUA_TNIL) {
      break;
    }
    const char *hmnrdbl = lua_tolstring(ls, -1, NULL);
    if (index > 1) {
      luaL_addstring(&b, ", ");
    }
    luaL_addstring(&b, hmnrdbl);
    lua_pop(ls, 1);
    index++;
  }
  luaL_addstring(&b, "}");
  luaL_pushresult(&b);
  return 1;
}

int errnotostring(lua_State *ls) {
  int givenerrno = luaL_checkinteger(ls, 1);
  lua_pushstring(ls, strerror(givenerrno));
  return 1;
}

int pusherrno(lua_State *ls, int givenerrno) {
  lua_pushinteger(ls, givenerrno);
  luaL_setmetatable(ls, "errno");
}

pid_t build(
    const char *dest,
    const char *workplace, 
    const char *src,
    const char *prtbl,
    lua_Reader lrd,
    void *lrd_data,
    const char *lrd_chunkname,
    const char *lrd_mode
) {
  if (mkdir(dest, 0777) == -1) {
    if (errno != EEXIST) {
      return -1;
    }
  }
  if (mkdir(workplace, 0777) == -1) {
    if (errno != EEXIST) {
      return -1;
    }
  }
  pid_t pid = fork();
  if (pid == -1) {
    return -1;
  }
  if (pid == 0) {
    unshare(CLONE_NEWUSER | CLONE_NEWNS);

    char wpsrcpath[PATH_MAX];
    if (strlen(workplace) + sizeof("/src")-1 >= PATH_MAX) {
      errno = ENAMETOOLONG;
      fprintf(stderr, "%s", strerror(errno));
      exit(errno);
    }
    strcpy(wpsrcpath, workplace);
    strcat(wpsrcpath, "/src");

    char wpdestpath[PATH_MAX];
    if (strlen(workplace) + sizeof("/dest")-1 >= PATH_MAX) {
      errno = ENAMETOOLONG;
      fprintf(stderr, "%s", strerror(errno));
      exit(errno);
    }
    strcpy(wpdestpath, workplace);
    strcat(wpdestpath, "/dest");

    if (mkdirs(wpsrcpath, 0777) == -1) {
      fprintf(stderr, "%s", strerror(errno));
      exit(errno);
    }
    if (mkdirs(wpdestpath, 0777) == -1) {
      fprintf(stderr, "%s", strerror(errno));
      exit(errno);
    }

    if (mount(src, wpsrcpath, NULL, MS_BIND, NULL) == -1) {
      fprintf(stderr, "%s", strerror(errno));
      exit(errno);
    }
    if (mount(NULL, wpsrcpath, NULL, MS_REMOUNT | MS_BIND | MS_RDONLY, NULL) == -1) {
      fprintf(stderr, "%s", strerror(errno));
      exit(errno);
    }
    if (mount(dest, wpdestpath, NULL, MS_BIND, NULL) == -1) {
      fprintf(stderr, "%s", strerror(errno));
      exit(errno);
    }
    if (mount(NULL, wpdestpath, NULL, MS_REMOUNT | MS_BIND, NULL) == -1) {
      fprintf(stderr, "%s", strerror(errno));
      exit(errno);
    }

    chdir(workplace);
    chroot(".");

    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    luaapi_init(L);
    luaapi_fslib_register(L);
    xiolib_init(L);

    int loadresult = lua_load(L, lrd, lrd_data, lrd_chunkname, lrd_mode);
    if (loadresult != LUA_OK) {
      lua_error(L);
    }
    lua_call(L, 0, 0);
    exit(0);
  } else {
    if (waitpid(pid, NULL, 0) == -1) {
      fprintf(stderr, "%s", errno);
      exit(0);
    }
    return pid;
  }
}

// TODO memory leak
struct bytecodebuf {
  char *buf;
  char *readptr;
  char *writeptr;
  size_t sz;
};

const char *bufreader(lua_State *L, void *data, size_t *size) {
  struct bytecodebuf *dt = (struct bytecodebuf *) data;
  *size = (size_t) (dt->writeptr - dt->readptr) + 1;
  char *currentreadptr = dt->readptr;
  dt->readptr += *size;
  return currentreadptr;
}

int bufwriter(lua_State *L, const void* p, size_t sz, void* ud) {
  struct bytecodebuf *dt = (struct bytecodebuf *) ud;
  if (dt->sz < dt->sz + sz) {
    char *oldptr = dt->buf;
    dt->buf = realloc(dt->buf, dt->sz * 2 < dt->sz + sz? (dt->sz + sz) * 2 : dt->sz * 2);
    if (dt->buf == NULL) {
      return errno;
    }
    dt->readptr = dt->buf + (dt->readptr - oldptr);
    dt->writeptr = dt->buf + (dt->writeptr - oldptr);
  }
  memcpy(dt->writeptr, p, sz);
  dt->writeptr += sz;
  return 0;
}
 
int l_build(lua_State *L) {
  const char *src = luaL_checkstring(L, 1);
  const char *wplc = luaL_checkstring(L, 2);
  const char *dest = luaL_checkstring(L, 3);
  luaL_checktype(L, 4, LUA_TFUNCTION);

  struct bytecodebuf bcbuf;
  bcbuf.sz = 1024;
  bcbuf.buf = malloc(bcbuf.sz);
  if (bcbuf.buf == NULL) {
    pusherrno(L, errno);
    lua_error(L);
  }
  bcbuf.readptr = bcbuf.buf;
  bcbuf.writeptr = bcbuf.buf;
  int result = lua_dump(L, &bufwriter, &bcbuf, 0);
  if (result != 0) {
    pusherrno(L, result);
    lua_error(L);
  }
  
  int bresult = build(dest, wplc, src, NULL, &bufreader, &bcbuf, "TODO", NULL);
  if (bresult == -1) {
    pusherrno(L, errno);
    lua_error(L);
  }
  return 0;
}

int luaapi_init(lua_State *ls) {
  luaL_newmetatable(ls, "stdtable");
  lua_pushcfunction(ls, &stdtbltostring);
  lua_setfield(ls, -2, "__tostring");
  lua_pop(ls, 1);

  luaL_newmetatable(ls, "errno");
  lua_pushcfunction(ls, &errnotostring);
  lua_setfield(ls, -2, "__tostring");
  lua_pop(ls, 1);

  lua_pushcfunction(ls, &l_build);
  lua_setglobal(ls, "build");
}
