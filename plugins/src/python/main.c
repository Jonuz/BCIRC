#include <Python.h>
#include <stdlib.h>
#include <string.h>

#include "pyapi.h"

#include "../headers/log.h"
#include "../headers/plugin_handler.h"
#include "../headers/callback_defines.h"


py_script **py_scripts_list;
extern unsigned int py_script_count;


char plugin_name[] = "Python";
char plugin_author[] = "Joona";
char plugin_version[] = "1.0";

static PyMethodDef BcircMethods[] = {
    { "register_script", py_register_script, METH_VARARGS, "Registers python script." },
    { "register_callback", py_register_callback, METH_VARARGS, "Registers python callback." },

    {NULL, NULL, 0, NULL}
};

static PyModuleDef BcircMod = {
    PyModuleDef_HEAD_INIT, "bcirc", NULL, -1, BcircMethods,
    NULL, NULL, NULL, NULL
};


static PyObject*
InitMod(void)
{
    return PyModule_Create(&BcircMod);
}



int plugin_init(plugin *pluginptr)
{
    PyImport_AppendInittab("bcirc", &InitMod);
    Py_Initialize();

    py_scripts_list = malloc(sizeof(py_script*));
    py_script_count = 0;

    char *pydir = getenv("BCIRC_PY_DIR");
    char *filename = "test";

    setenv("PYTHONPATH", pydir, 1);

	int res = 0;
    if (! (res = load_script(filename)) )
	{
		bcirc_printf("load_script() returned: %d!\n", res);
		return BCIRC_PLUGIN_FAIL;
	}
    return BCIRC_PLUGIN_OK;
}


int load_script(char *filename)
{
    py_script *new_script = malloc(sizeof(new_script));

    new_script->name = malloc(strlen(filename) + 1);
    strcpy(new_script->name, filename);
    new_script->handle = PyImport_Import(PyUnicode_DecodeFSDefault(new_script->name));

    if (!new_script->handle)
    {
        bcirc_printf("Failed to load %s\n", filename);
        PyErr_Print();
        return -1;
    }
	init_script(new_script);

    return 1;
}

int init_script(py_script *script)
{
    PyObject *init_func = PyObject_GetAttrString(script->handle, "script_init");
    if (!(init_func) || (!(PyCallable_Check(init_func))))
    {
        printf("no script_init() on script %s?\n", script->name);
        PyErr_Print();
        return -1;
    }

    PyObject *pArgs = PyTuple_New(1);
    PyTuple_SetItem(pArgs, 0, PyLong_FromVoidPtr(script));

    PyObject *res = PyObject_CallObject(init_func, pArgs);
    if (res == NULL)
    {
        bcirc_printf("cb returned %d\n", PyLong_AsLong(res));
    }

    Py_DECREF(pArgs);

/*
    if (*res != BCIRC_PLUGIN_OK)
    {
        bcirc_printf("Failed to init script %s\n", script->name);
        return -1;
    }
*/

    return 1;
}
