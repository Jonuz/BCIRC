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
    char *pydir = getenv("BCIRC_PY_DIR");
    char filename[] = "test";

    bcirc_printf("????\n");

    setenv("PYTHONPATH", pydir, 1);

    PyImport_AppendInittab("bcirc", &InitMod);
    Py_Initialize();

    py_scripts_list = malloc(sizeof(py_script*));
    py_script_count = 0;

	int res = 0;
    if (! (res = load_script(filename)) )
	{
		bcirc_printf("load_script() returned: %d!\n", res);
		return BCIRC_PLUGIN_FAIL;
	}

    //register_callback(CALLBACK_CALLBACKS_EXECUTED, py_execute_callbacks, 20, pluginptr);

    return BCIRC_PLUGIN_OK;
}

int py_execute_callbacks(void **params, int argc) //Todo: Make this not so ugly.
{
    char *cb_name = params[argc];

    for (int i = 0; i < py_script_count; i++)
    {
        for (int y = 0; y < py_scripts_list[i]->cb_count; y++)
        {
            char *script_cb_name = py_scripts_list[i]->cbs[y]->cb_name;
            if (strcmp(cb_name, script_cb_name) == 0)
            {
                PyObject *ptrarray = PyList_New(argc);
                for (int x = 0; x < argc - 1; x++)
                {
                    if (!(PyList_SET_ITEM(ptrarray, x, PyLong_FromVoidPtr(params[x]))))
                    {
                        bcirc_printf("Failed to set arg. Callback %s | argc: %d\n", cb_name, x);
                        return BCIRC_PLUGIN_FAIL;
                    }
                }

                PyObject *pArgs = Py_BuildValue("(od)", ptrarray, PyLong_FromLong(argc - 1));
                PyObject *cb = py_scripts_list[i]->cbs[y]->cb_func;

                PyObject *res = PyObject_CallObject(cb, pArgs);
                Py_DECREF(pArgs);
                if (res == NULL)
                {
                    bcirc_printf("Failed to execute py-function!\n");

                }

            }
        }
    }
    return BCIRC_PLUGIN_OK;
}

int load_script(char *filename)
{
    py_script *new_script = malloc(sizeof(new_script));

    new_script->name = malloc(strlen(filename) + 1);
    strcpy(new_script->name, filename);

    PyObject *name = PyUnicode_DecodeFSDefault(new_script->name);
    new_script->handle = PyImport_Import(name);
    Py_DECREF(name);
    
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

	PyObject *pArgs = Py_BuildValue("(O)", PyLong_FromVoidPtr(script));

    PyObject *res = PyObject_CallObject(init_func, pArgs);
    Py_DECREF(pArgs);
    if (res == NULL)
    {
        bcirc_printf("cb returned %d\n", PyLong_AsLong(res));
        return -1;
    }
    bcirc_printf("Initalized script\n");


    return 1;
}
