script_name = "test script"
script_version = "0.1"
script_author = "Joona"

import bcirc


def on_recv(params, count):
    srv = bcirc.get_chan_srv(params[0])

    # http://ascii.co.uk/art/snake
    bcirc.privmsg(" YOU ARE WIZARD, HARRY!", "#tesm", srv)
    bcirc.privmsg("      __/\__̣̣̣̣     ", "#tesm", srv)
    bcirc.privmsg("_,.-'`_   o`;__, ", "#tesm", srv)
    bcirc.privmsg(" _.-'` '---'   ' ", "#tesm", srv)

    return 1


def script_init(script):
    bcirc.register_script(script, script_name, script_version, script_author)
    bcirc.register_callback(script, "channel_join", on_recv, 20)

    return 1
