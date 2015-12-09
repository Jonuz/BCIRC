script_name = "test script"
script_version = "0.1"
script_author = "Joona"

import bcirc

def script_init(script):
    bcirc.register_script(script, script_name, script_version, script_author)
    print("yo")

