script_name = "test script"
script_version = "0.1"
script_author = "Joona"

import bcirc
from ctypes import *

def on_recv(params, count):
    srv = bcirc.get_chan_srv(params[0])

    bcirc.server_send(srv, "PRIVMSG #tesm :       ____\r\n");
    bcirc.server_send(srv, "PRIVMSG #tesm : _,.-'`_ o `;__,\r\n")
    bcirc.server_send(srv, "PRIVMSG #tesm : _.-'` '---'   '\r\n");


def script_init(script):
    bcirc.register_script(script, script_name, script_version, script_author)
    bcirc.register_callback(script, "channel_join", on_recv, 20)

    return 1
