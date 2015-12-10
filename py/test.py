script_name = "test script"
script_version = "0.1"
script_author = "Joona"

import bcirc

def on_recv(params, count):
    print("server sent something")

def script_init(script):
    bcirc.register_script(script, script_name, script_version, script_author)
    bcirc.register_callback(script, "callback_server_recv", on_recv, 20)

    return 1;
