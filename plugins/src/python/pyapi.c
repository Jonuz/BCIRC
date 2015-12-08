#include <Python.h>
#include <stdlib.h>
#include <string.h>

#include "../headers/log.h"
#include "../headers/callback_defines.h"

#include "pyapi.h"


py_script **py_scripts_list;
extern unsigned int py_script_count;


PyObject* py_register_script(PyObject *self, PyObject *args)
{
    PyObject *scriptptr = NULL;
    char *script_name, *script_version, *script_author;

    PyArg_ParseTuple(args, "0sss", &scriptptr, &script_name, &script_version, &script_author);


    if (!scriptptr)
    {
        bcirc_printf("scriptptr is null!\n");
        return PyLong_FromLong(-1);
    }

    py_script *script = (py_script*) PyLong_FromVoidPtr(scriptptr);

    script->name = malloc(strlen(script_name) + 1);
    script->version = malloc(strlen(script_version) + 1);
    script->author = malloc(strlen(script_author) + 1);

    strcpy(script->name, script_name);
    strcpy(script->version, script_version);
    strcpy(script->author, script_author);

    pthread_mutex_lock(&py_scripts_mutex);
    py_scripts_list = realloc(py_scripts_list, (py_script_count + 1) * sizeof(py_script*));
    if (!py_scripts_list)
    {
        bcirc_printf("Failed to realloc py_script_list(%s)\n", __PRETTY_FUNCTION__);
        return PyLong_FromLong(-2);
    }

    py_scripts_list[py_script_count] = script;
    py_script_count++;
    pthread_mutex_unlock(&py_scripts_mutex);

    return PyLong_FromLong(1);
}


PyObject* py_register_callback(PyObject *self, PyObject *args)
{
    PyObject *pyptr = NULL;
    char *cb_name = NULL;

    py_cb *new_cb = malloc(sizeof(py_cb));

    if (!PyArg_ParseTuple(args, "0s0d", &pyptr, &cb_name, &new_cb->cb_func, &new_cb->priority))
    {
        free(new_cb);
        return PyLong_FromLong(-1);
    }
    new_cb->cb_name = malloc( (strlen(cb_name) + 1) * sizeof(char));
    strcpy(new_cb->cb_name, cb_name);

    py_script *script = (py_script*) PyLong_FromVoidPtr(pyptr);
    if (!script)
    {
        bcirc_printf("script is null(%s)\n", __PRETTY_FUNCTION__);
        return PyLong_FromLong(-2);
    }

    script->cbs = realloc(script->cbs, (script->cb_count + 1) * sizeof(py_cb*));
    script->cbs[script->cb_count] = new_cb;


    return PyLong_FromLong(1);
}
