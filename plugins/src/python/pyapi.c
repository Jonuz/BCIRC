#include <Python.h>
#include <stdlib.h>
#include <string.h>

#include "../headers/log.h"
#include "../headers/server.h"
#include "../headers/channel.h"
#include "../headers/irc_cmds.h"
#include "../headers/callback_defines.h"

#include "pyapi.h"


py_script **py_scripts_list;
extern unsigned int py_script_count;


PyObject* py_register_script(PyObject *self, PyObject *args)
{
    PyObject *scriptptr = NULL;
    char *script_name, *script_version, *script_author;

    PyArg_ParseTuple(args, "Osss", &scriptptr, &script_name, &script_version, &script_author);


    if (scriptptr == NULL)
    {
        bcirc_printf("scriptptr is null!\n");
        return PyLong_FromLong(-1);
    }

    py_script *script = (py_script*) PyLong_AsVoidPtr(scriptptr);

    script->name = malloc(strlen(script_name) + 1);
    script->version = malloc(strlen(script_version) + 1);
    script->author = malloc(strlen(script_author) + 1);

    strcpy(script->name, script_name);
    strcpy(script->version, script_version);
    strcpy(script->author, script_author);

    script->cb_count = 0;
    script->cbs = malloc(sizeof(py_cb*));

    pthread_mutex_lock(&py_scripts_mutex);
    py_scripts_list = realloc(py_scripts_list, (py_script_count + 1) * sizeof(py_script*));
    if (!py_scripts_list)
    {
        bcirc_printf("Failed to realloc py_script_list(%s)\n", __PRETTY_FUNCTION__);
		pthread_mutex_unlock(&py_scripts_mutex);

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

    if (!PyArg_ParseTuple(args, "OsOd", &pyptr, &cb_name, &new_cb->cb_func, &new_cb->priority))
    {
        free(new_cb);
        return PyLong_FromLong(-1);
    }
    new_cb->cb_name = malloc( (strlen(cb_name) + 1) * sizeof(char));
    strcpy(new_cb->cb_name, cb_name);

    py_script *script = (py_script*) PyLong_AsVoidPtr(pyptr);
    if (!script)
    {
        bcirc_printf("script is null(%s)\n", __PRETTY_FUNCTION__);
        free(new_cb);
        return PyLong_FromLong(-2);
    }

    if (!PyCallable_Check(new_cb->cb_func))
    {
        printf("Can't register callback, object is not callable!\n");
        free(new_cb);
        return PyLong_FromLong(-3);
    }

    script->cbs = realloc(script->cbs, (script->cb_count + 1) * sizeof(py_cb*));
    script->cbs[script->cb_count] = new_cb;
    script->cb_count++;

    return PyLong_FromLong(1);
}



PyObject* py_get_chan_srv(PyObject *self, PyObject *args)
{
    PyObject *pyptr = NULL;

    if (!PyArg_ParseTuple(args, "O", &pyptr))
    {
        return PyLong_FromLong(-1);
    }

    channel *chan = (channel*) PyLong_AsVoidPtr(pyptr);
    if (!chan)
    {
        bcirc_printf("channel is null(%s)\n", __PRETTY_FUNCTION__);
        return PyLong_FromLong(-2);
    }

    return PyLong_FromVoidPtr(chan->srv);
}



PyObject* py_server_send(PyObject *self, PyObject *args)
{
    PyObject *pyptr = NULL;
    char *buf;

    if (!PyArg_ParseTuple(args, "Os", &pyptr, &buf))
    {
        return PyLong_FromLong(-1);
    }

    if (!pyptr)
        return PyLong_FromLong(-2);

    server *srv = (server*) PyLong_AsVoidPtr(pyptr);
    if (!srv)
        return PyLong_FromLong(-3);


    int res = server_send(srv, buf);

    return PyLong_FromLong(res);
}

PyObject *py_server_connect(PyObject *self, PyObject *args)
{
    PyObject *pyptr = NULL;

    if (!PyArg_ParseTuple(args, "O", &pyptr))
    {
        return PyLong_FromLong(-1);
    }

    if (!pyptr)
        return PyLong_FromLong(-2);

    server *srv = (server*) PyLong_AsVoidPtr(pyptr);
    if (!srv)
        return PyLong_FromLong(-3);


    int res = server_connect(srv);

    return PyLong_FromLong(res);

}

PyObject *py_add_to_serverpool(PyObject *self, PyObject *args)
{
    PyObject *pyptr = NULL;

    if (!PyArg_ParseTuple(args, "O", &pyptr))
    {
        return PyLong_FromLong(-1);
    }

    if (!pyptr)
        return PyLong_FromLong(-2);

    server *srv = (server*) PyLong_AsVoidPtr(pyptr);
    if (!srv)
        return PyLong_FromLong(-3);


    int res = add_to_serverpool(srv);

    return PyLong_FromLong(res);

}

PyObject *py_remove_from_serverpool(PyObject *self, PyObject *args)
{
    PyObject *pyptr = NULL;

    if (!PyArg_ParseTuple(args, "O", &pyptr))
    {
        return PyLong_FromLong(-1);
    }

    if (!pyptr)
        return PyLong_FromLong(-2);

    server *srv = (server*) PyLong_AsVoidPtr(pyptr);
    if (!srv)
        return PyLong_FromLong(-3);


    int res = remove_from_serverpool(srv);

    return PyLong_FromLong(res);
}

PyObject *py_free_server(PyObject *self, PyObject *args)
{
    PyObject *pyptr = NULL;

    if (!PyArg_ParseTuple(args, "O", &pyptr))
    {
        return PyLong_FromLong(-1);
    }

    if (!pyptr)
        return PyLong_FromLong(-2);

    server *srv = (server*) PyLong_AsVoidPtr(pyptr);
    if (!srv)
        return PyLong_FromLong(-3);


    int res = free_server(srv);

    return PyLong_FromLong(res);

}

PyObject *py_load_servers(PyObject *self, PyObject *args)
{
    char *file;

    if (!PyArg_ParseTuple(args, "s", &file))
    {
        return PyLong_FromLong(-1);
    }

    int res = load_servers(file);

    return PyLong_FromLong(res);

}



PyObject *py_bcirc_printf(PyObject *self, PyObject *args)
{
    char *buf;

    if (!PyArg_ParseTuple(args, "s", &buf))
    {
        return PyLong_FromLong(-1);
    }

    int res = bcirc_printf(buf);

    return PyLong_FromLong(res);

}

PyObject *py_bcirc_log(PyObject *self, PyObject *args)
{
    char *buf, *logname;

    if (!PyArg_ParseTuple(args, "s", &buf, "s", &logname))
    {
        return PyLong_FromLong(-1);
    }

    int res = bcirc_log(buf, logname);

    return PyLong_FromLong(res);
}



PyObject *py_privmsg(PyObject *self, PyObject *args)
{
    char *newnick;
    PyObject *pyptr = NULL;

    if (!PyArg_ParseTuple(args, "s", &newnick, "O", &pyptr))
    {
        return PyLong_FromLong(-1);
    }

    server *srv = (server*) PyLong_AsVoidPtr(pyptr);
    if (!srv)
        return PyLong_FromLong(-2);

    int res = nick(newnick, srv);

    return PyLong_FromLong(res);
}

PyObject *py_join_channel(PyObject *self, PyObject *args)
{
    char *channame, *chanpass;
    PyObject *pyptr = NULL;

    if (!PyArg_ParseTuple(args, "s", &channame, "s", &chanpass, "O", &pyptr))
    {
        return PyLong_FromLong(-1);
    }

    server *srv = (server*) PyLong_AsVoidPtr(pyptr);
    if (!srv)
        return PyLong_FromLong(-2);

    int res = join_channel(channame, chanpass, srv);

    return PyLong_FromLong(res);
}

PyObject *py_part_channel(PyObject *self, PyObject *args)
{
    char *reason;
    PyObject *pyptr = NULL;

    if (!PyArg_ParseTuple(args, "s", &reason, "O", &pyptr))
    {
        return PyLong_FromLong(-1);
    }

    channel *chan = (channel*) PyLong_AsVoidPtr(pyptr);
    if (!chan)
        return PyLong_FromLong(-2);

    int res = part_channel(reason, chan);

    return PyLong_FromLong(res);
}

PyObject *py_nick(PyObject *self, PyObject *args)
{
    char *newnick;
    PyObject *pyptr = NULL;

    if (!PyArg_ParseTuple(args, "s", &newnick, "O", &pyptr))
    {
        return PyLong_FromLong(-1);
    }

    server *srv = (server*) PyLong_AsVoidPtr(pyptr);
    if (!srv)
        return PyLong_FromLong(-2);

    int res = nick(newnick, srv);

    return PyLong_FromLong(res);
}
