int xio_writefile(lua_State *st) {
  const char *pathname = luaL_checkstring(st, 1);
  const char *mode = "w";
  const char *content;
  if (lua_gettop(st) > 2) {
    mode = luaL_checkstring(st, 2);
    content = luaL_checkstring(st, 3);
  } else {
    content = luaL_checkstring(st, 2);
  }

  FILE *f = fopen(pathname, mode);
  if (f == NULL) {
    pusherrno(st, errno);
    lua_error(st);
  }

  if (fprintf(f, "%s", content) < 0) {
    pusherrno(st, errno);
    lua_error(st);
  }
  if (fclose(f) == EOF) {
    pusherrno(st, errno);
    lua_error(st);
  }
}

const luaL_Reg xiolib_functions[] = {
  {"writefile", &xio_writefile},
  {NULL, NULL}
};

int xiolib_init(lua_State *st) {
  luaL_newlib(st, xiolib_functions);
  lua_setglobal(st, "xio");
}
