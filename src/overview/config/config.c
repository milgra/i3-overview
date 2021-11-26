#ifndef config_h
#define config_h

#include "zc_map.c"

void  config_init();
void  config_destroy();
int   config_read(char* path);
void  config_set(char* key, char* value);
char* config_get(char* key);
int   config_get_int(char* key);
int   config_get_bool(char* key);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "kvlines.c"

map_t* confmap;

void config_init()
{
  confmap = MNEW(); // REL 0
}

void config_destroy()
{
  REL(confmap); // REL 0
}

int config_read(char* path)
{
  return kvlines_read(path, confmap);
}

void config_set(char* key, char* value)
{
  MPUT(confmap, key, value);
}

char* config_get(char* key)
{
  return MGET(confmap, key);
}

int config_get_bool(char* key)
{
  char* val = MGET(confmap, key);
  if (val && strcmp(val, "true") == 0)
    return 1;
  else
    return 0;
}

int config_get_int(char* key)
{
  char* val = MGET(confmap, key);
  if (val)
    return atoi(val);
  else
    return 0;
}

#endif
