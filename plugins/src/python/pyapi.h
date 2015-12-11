#ifndef PY_API_H_
#define PY_API_H

#include <Python.h>
#include <pthread.h>



typedef struct py_cb
{
    PyObject *cb_func;
    char *cb_name;

    int priority;
} py_cb;

typedef struct py_script
{
    char *name;
    char *author;
    char *version;

    char *file;

    PyObject *path;
    PyObject *handle;

    py_cb **cbs;
    int cb_count;
} py_script;

py_script **py_scripts_list;
unsigned int py_script_count;

pthread_mutex_t py_scripts_mutex;

PyThreadState *mainThreadState;

int load_script(char *filename);
int init_script(py_script *script);
int py_execute_callbacks(void **params, int argc);


PyObject* py_register_script(PyObject *self, PyObject *args);
PyObject* py_register_callback(PyObject *self, PyObject *args);


PyObject* py_server_send(PyObject *self, PyObject *args);


#endif


/*

def on_recv(params, count):
    srv = get_server(params[0])
    buf = get_string(params[1])
    print("server sent: ", buf)

    return BCIRC_SCRIPT_OK

script_name = "test script"
script_version = "0.1"
script_authotr = "Joona"

def py_init(script):
    register_script(script, script_name, script_version, script_author)

    register_callback(script, "callback_server_recv", on_recv , 20)

*/
