#ifndef analyzer_h
#define analyzer_h

#include "zc_vector.c"

typedef struct _i3_window_t
{
  int   x;
  int   y;
  int   width;
  int   height;
  char* title;
  char* class;
} i3_window_t;

typedef struct _i3_workspace_t
{
  int    x;
  int    y;
  int    width;
  int    height;
  int    number;
  int    focused;
  char*  output;
  vec_t* windows;
} i3_workspace_t;

void analyzer_extract(char* ws_json, char* tree_json, vec_t* workspaces);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "json.c"
#include "zc_cstring.c"
#include "zc_cstrpath.c"

void i3_workspace_del(void* p)
{
  i3_workspace_t* ws = p;
  REL(ws->windows);
  REL(ws->output);
}

void i3_workspace_desc(void* p, int level)
{
  i3_workspace_t* ws = p;
  printf("workspace num %i w %i h %i focus %i output %s\n", ws->number, ws->width, ws->height, ws->focused, ws->output);
  mem_describe(ws->windows, level);
}

i3_workspace_t* i3_workspace_new()
{
  i3_workspace_t* ws = CAL(sizeof(i3_workspace_t), i3_workspace_del, i3_workspace_desc);
  ws->windows        = VNEW();
  return ws;
}

void i3_window_del(void* p)
{
  i3_window_t* wi = p;
  REL(wi->class);
  REL(wi->title);
}

void i3_window_desc(void* p, int level)
{
  i3_window_t* wi = p;
  printf("window x %i y %i w %i h %i title %s", wi->x, wi->y, wi->width, wi->height, wi->title);
}

i3_window_t* i3_window_new()
{
  i3_window_t* wi = CAL(sizeof(i3_window_t), i3_window_del, i3_window_desc);

  return wi;
}

char* i3_get_value(vec_t* vec, int pos, char* path)
{
  for (int index = pos; index < vec->length; index++)
  {
    if (strcmp(vec->data[index], path) == 0) return vec->data[index + 1];
  }
  for (int index = pos; index > -1; index--)
  {
    if (strcmp(vec->data[index], path) == 0) return vec->data[index + 1];
  }
  return NULL;
}

void analyzer_extract(char* ws_json, char* tree_json, vec_t* workspaces)
{
  vec_t* json = json_parse(ws_json);

  for (int index = 0; index < json->length; index += 2)
  {
    char* key     = json->data[index];
    char* type_id = strstr(key, "/id");

    if (type_id != NULL)
    {
      i3_workspace_t* ws = i3_workspace_new();

      char* path    = cstr_new_path_remove_last_component(key); // REL 2
      int   pathlen = strlen(path);

      char* wx = cstr_new_format(pathlen + 20, "%srect/x", path);      // REL 3
      char* wy = cstr_new_format(pathlen + 20, "%srect/y", path);      // REL 4
      char* wk = cstr_new_format(pathlen + 20, "%srect/width", path);  // REL 5
      char* hk = cstr_new_format(pathlen + 20, "%srect/height", path); // REL 6
      char* ok = cstr_new_format(pathlen + 20, "%soutput", path);      // REL 7
      char* nk = cstr_new_format(pathlen + 20, "%snum", path);         // REL 8
      char* fk = cstr_new_format(pathlen + 20, "%sfocused", path);     // REL 9

      char* x = i3_get_value(json, index, wx);
      char* y = i3_get_value(json, index, wy);
      char* w = i3_get_value(json, index, wk);
      char* h = i3_get_value(json, index, hk);
      char* o = i3_get_value(json, index, ok);
      char* n = i3_get_value(json, index, nk);
      char* f = i3_get_value(json, index, fk);

      ws->x       = atoi(x);
      ws->y       = atoi(y);
      ws->width   = atoi(w);
      ws->height  = atoi(h);
      ws->focused = strcmp(f, "true") == 0;
      ws->number  = atoi(n);
      ws->output  = cstr_new_cstring(o);

      REL(path); // REL 2
      REL(wx);   // REL 3
      REL(wy);   // REl 4
      REL(wk);   // REL 5
      REL(hk);   // REL 6
      REL(ok);   // REL 7
      REL(nk);   // REL 8
      REL(fk);   // REL 9

      VADDR(workspaces, ws);
    }
  }

  REL(json);

  json = json_parse(tree_json);

  int curr_wspc_n = 0;

  for (int index = 0; index < json->length; index += 2)
  {
    char* key      = json->data[index];
    char* type_prt = strstr(key, "type");

    if (type_prt != NULL)
    {
      if (strcmp(json->data[index + 1], "workspace") == 0)
      {
        char* path    = cstr_new_path_remove_last_component(key); // REL 1
        int   pathlen = strlen(path);
        char* pathkey = cstr_new_format(pathlen + 20, "%snum", path); // REL 2

        char* n     = i3_get_value(json, index, pathkey);
        curr_wspc_n = atoi(n);

        REL(path);
        REL(pathkey);
      }
    }

    char* wind_prt = strstr(key, "window_properties/instance");

    if (wind_prt != NULL && curr_wspc_n > -1)
    {
      i3_window_t* wi = i3_window_new();

      char* path    = cstr_new_path_remove_last_component(key); // REL 3
      int   pathlen = strlen(path);
      char* root    = cstr_new_path_remove_last_component(path); // REL 4

      char* tk = cstr_new_format(pathlen + 20, "%stitle", path);       // REL 5
      char* ck = cstr_new_format(pathlen + 20, "%sclass", path);       // REL 6
      char* xk = cstr_new_format(pathlen + 20, "%srect/x", root);      // REL 7
      char* yk = cstr_new_format(pathlen + 20, "%srect/y", root);      // REL 8
      char* wk = cstr_new_format(pathlen + 20, "%srect/width", root);  // REL 9
      char* hk = cstr_new_format(pathlen + 20, "%srect/height", root); // REL 10

      char* c = i3_get_value(json, index, ck);
      char* i = json->data[index + 1];
      char* t = i3_get_value(json, index, tk);

      char* rx = i3_get_value(json, index, xk);
      char* ry = i3_get_value(json, index, yk);
      char* rw = i3_get_value(json, index, wk);
      char* rh = i3_get_value(json, index, hk);

      wi->x      = atoi(rx);
      wi->y      = atoi(ry);
      wi->width  = atoi(rw);
      wi->height = atoi(rh);
      wi->class  = cstr_new_cstring(c);
      wi->title  = cstr_new_cstring(t);

      for (int wsi = 0; wsi < workspaces->length; wsi++)
      {
        i3_workspace_t* ws = workspaces->data[wsi];

        if (ws->number == curr_wspc_n) VADDR(ws->windows, wi);
      }

      REL(path); // REL 3
      REL(root); // REL 4
      REL(tk);   // REL 5
      REL(ck);   // REL 6
      REL(xk);   // REL 7
      REL(yk);   // REL 8
      REL(wk);   // REL 9
      REL(hk);   // REL 10
    }
  }

  REL(json);
}

#endif
