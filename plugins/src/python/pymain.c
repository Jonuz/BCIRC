#include <Python.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include "pyapi.h"

#include "../headers/log.h"
#include "../headers/plugin_handler.h"
#include "../headers/callback_defines.h"


py_script **py_scripts_list;
unsigned int py_script_count;


char plugin_name[] = "Python";
char plugin_author[] = "Joona";
char plugin_version[] = "0.1";

static PyMethodDef BcircMethods[] = {
    { "register_script", py_register_script, METH_VARARGS, "Registers python script." },
    { "register_callback", py_register_callback, METH_VARARGS, "Registers python callback." },

    { "get_chan_srv", py_get_chan_srv, METH_VARARGS, "Returns pointer to channel's server." },
	{ "get_string_from_ptr", py_get_string_from_ptr, METH_VARARGS, "Returns string value from pointer." },


    { "server_send", py_server_send, METH_VARARGS, "Sends to server." },
    { "server_connect", py_server_connect, METH_VARARGS, "Connects to server." },
    { "add_to_serverpool", py_add_to_serverpool, METH_VARARGS, "Adds server yo server pool." },
    { "free_server", py_free_server, METH_VARARGS, "Frees server." },
    { "load_servers", py_load_servers, METH_VARARGS, "Load server(s) from config file and adds those to serverpool." },

    { "bcirc_printf", py_bcirc_printf, METH_VARARGS, "Frees server." },
    { "bcirc_log", py_bcirc_log, METH_VARARGS, "Frees server." },

    { "privmsg", py_privmsg, METH_VARARGS, "Sends privmsg." },
    { "privmsg_queue", py_privmsg_queue, METH_VARARGS, "Adds message to queue to be sent." },

    { "join_channel", py_join_channel, METH_VARARGS, "Join to channel." },
    { "part_channel", py_part_channel, METH_VARARGS, "Parts from channel." },
    { "nick", py_nick, METH_VARARGS, "Changes nick." },

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


int load_script(char *filename, char *dir)
{
    py_script *new_script = malloc(sizeof(new_script));

    new_script->name = malloc(strlen(filename) + 1);
    strcpy(new_script->name, filename);

    PyObject *name = PyUnicode_DecodeFSDefault(filename);
	new_script->handle = malloc(sizeof(PyObject));
    new_script->handle = PyImport_Import(name);
    Py_DECREF(name);

    //setenv("PYTHONPATH", dir, 1);

    if (!new_script->handle)
    {
        bcirc_printf("Failed to load %s\n", filename);
        PyErr_Print();
        return -1;
    }

	if (init_script(new_script) != 1)
    {
        bcirc_printf("Failed to initalize script\n");
        return -2;
    }

    return 1;
}

int load_script_dir(char *dirname)
{
    DIR *d = NULL;
    struct dirent *dir;

    d = opendir(dirname);
    if (d)
    {
        while( (dir = readdir(d)) != NULL )
        {
            if (dir->d_type == DT_REG)
            {
                if (strcmp(dir->d_name, "__init__") == 0)
                    continue;

                size_t len = strlen(dir->d_name) - 3;
                char *filename = malloc(len+1);
                strncpy(filename, dir->d_name, len);
                filename[len] = '\0';
                load_script(filename, dirname);
            }
        }
    }
    return 1;
}

int plugin_init(plugin *pluginptr)
{
    char *pydir = getenv("BCIRC_PY_DIR");
    //setenv("PYTHONPATH", pydir, 1);

    PyImport_AppendInittab("bcirc", &InitMod);

    Py_Initialize();
    PyEval_InitThreads();

	PyObject* sysPath = PySys_GetObject((char*)"path");
	PyList_Append(sysPath, PyUnicode_DecodeFSDefault("./py/"));

	//Py_DECREF(programName);


    mainThreadState = PyThreadState_Get();

    py_scripts_list = malloc(sizeof(py_script*));
    py_script_count = 0;

    pthread_mutex_init(&py_scripts_mutex, NULL);

    load_script_dir(pydir);
/*
	int res = 0;
    if (! (res = load_script(filename)) )
	{
		bcirc_printf("load_script() returned: %d!\n", res);
		return BCIRC_PLUGIN_FAIL;
	}
*/
    register_callback(CALLBACK_CALLBACKS_EXECUTED, py_execute_callbacks, 20, pluginptr);

    return BCIRC_PLUGIN_OK;
}

int py_execute_callbacks(void **params, int argc) //Todo: Make this not so ugly.
{
    char *cb_name = params[argc-1];

    for (int i = 0; i < py_script_count; i++)
    {
        for (int y = 0; y < py_scripts_list[i]->cb_count; y++)
        {
            char *script_cb_name = py_scripts_list[i]->cbs[y]->cb_name;
            if (strcmp(cb_name, script_cb_name) == 0)
            {
                PyObject *ptrarray = PyList_New(argc-1);
                for (int x = 0; x < argc - 1; x++)
                {
                    if (PyList_SetItem(ptrarray, x, PyLong_FromVoidPtr(params[x])) != 0 )
                    {
                        bcirc_printf("Failed to set arg. Callback %s | argc: %d\n", cb_name, x);
                        //Py_DECREF(ptrarray);
                        return BCIRC_PLUGIN_FAIL;
                    }
                }

                //PyThreadState *tstate = PyThreadState_New(mainThreadState->interp);
                //PyThreadState *this_thread = PyThreadState_New(tstate);
                //PyEval_AcquireThread(tstate);

                PyObject *pArgs = Py_BuildValue("(Ol)", ptrarray, PyLong_FromLong(argc - 1));
                PyObject *cb = py_scripts_list[i]->cbs[y]->cb_func;
                Py_DECREF(ptrarray);

                if (pArgs == NULL)
                {
                    bcirc_printf("pArgs is null!\n");
                    Py_DECREF(pArgs);
                    //PyEval_ReleaseThread(tstate);
                    //PyThreadState_Delete(tstate);
                    break;
                }

                if (!PyCallable_Check(cb))
                {
                    bcirc_printf("callback is not callable!\n");
                    Py_DECREF(pArgs);
                    //PyEval_ReleaseThread(tstate);
                    //PyThreadState_Delete(tstate);
                    break;
                }

                bcirc_printf("calling callback!\n");
                PyObject *res = PyObject_CallObject(cb, pArgs);
                Py_DECREF(pArgs);

                if (res == NULL)
                {
                    bcirc_printf("Failed to execute py-function!\n");
					PyErr_Print();
                }
                //PyEval_ReleaseThread(tstate);
                //PyThreadState_Delete(tstate);

            }
        }
    }
    return BCIRC_PLUGIN_OK;
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

    script->cbs = NULL;

	PyObject *pArgs = Py_BuildValue("(O)", PyLong_FromVoidPtr(script));

    PyObject *res = PyObject_CallObject(init_func, pArgs);
    //Py_DECREF(pArgs);
    if (res == NULL)
    {
        bcirc_printf("cb returned %d\n", PyLong_AsLong(res));
        return -1;
    }
    bcirc_printf("Initalized script\n");


    return 1;
}
