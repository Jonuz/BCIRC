script_name = "test script"
script_version = "0.1"
script_author = "Joona"

import bcirc
from ctypes import *

def on_recv(params, count):
    #buf = c_char_p(params[1])
    #rint("buf: ", buf)
    print ("something sent")


def script_init(script):
    bcirc.register_script(script, script_name, script_version, script_author)
    bcirc.register_callback(script, "server_recv", on_recv, 20)

    return 1
