pid_t build(
    const char *dest,
    const char *workplace, 
    const char *src,
    const char *prtbl,
    lua_Reader lrd,
    void *lrd_data,
    const char *lrd_chunkname,
    const char *lrd_mode
);

int errnotostring(lua_State *ls);

int filecopy(const char *src, const char *dest);

int fs_copy(lua_State *L);

int fs_rename(lua_State *L);

int fslib_direntries(lua_State *luast);

int l_build(lua_State *L);

void *lnullunwrp(void *value);

const void *lnullwrp(void *value);

int luaapi_fslib_register(lua_State *luast);

int luaapi_init(lua_State *ls);

const luaL_Reg luaapi_fslib[];

const void *luanull = "LUANULL";

uint32_t mkdirs(const char *path, mode_t mode);

int pusherrno(lua_State *ls, int givenerrno);

int stdtbltostring(lua_State *ls);

int xio_writefile(lua_State *st);

const luaL_Reg xiolib_functions[];

int xiolib_init(lua_State *st);
