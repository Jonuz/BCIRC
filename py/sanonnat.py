#! /usr/bin/python3

running_inside_bcirc = False

import requests
import xml.etree.ElementTree as ET
from bs4 import BeautifulSoup
import re
import random

import ctypes

try:
    import bcirc
    running_inside_bcirc = True
except ImportError:
    pass

script_name = "FMI"
script_version = "0.1"
script_author = "Joona"


global sanonnat


def omaanko_sanonnat():
    return len(sanonnat)


def alusta_sanonnat():
    global sanonnat
    tulos = requests.get("https://fi.wikiquote.org/wiki/Suomalaisia_sananlaskuja", timeout=15)
    if tulos.status_code is not 200:
        print("sanontoja ei saatu haettua")
        return
    soppa =  BeautifulSoup(tulos.content, "lxml")
    haetut_laskut = soppa.find("div", {"id": "mw-content-text"}).findChildren("li")

    def listaksi(lasku):
        return lasku.text

    listatut_sanonnat = list(map(listaksi, haetut_laskut))
    sanonnat = listatut_sanonnat

def hae_sanonta(haku):
    global sanonnat
    haku = haku.lower().strip()
    if not haku or not len(haku):
        sanonta = sanonnat[random. randrange(len(sanonnat))]
    else:
        suodatetut_sanonnat = list(filter((lambda i: haku in i.lower()), sanonnat))
        sanonta = suodatetut_sanonnat[random.randrange(len(suodatetut_sanonnat))]
    return sanonta


def kasittele_viesti(parametrit, maara):

    servu = parametrit[0]
    mista_luettiin = bcirc.get_string_from_ptr(parametrit[3])
    saatu_viesti = str(bcirc.get_string_from_ptr(parametrit[4]))


    if not saatu_viesti.startswith("!sano"):
        return 1

    haettu_sanonta = saatu_viesti[6:]
    sanonta = hae_sanonta(haettu_sanonta)

    vastaus = sanonta if sanonta else "Paskan hait"

    bcirc.privmsg(vastaus, mista_luettiin, servu)

    return 1


def script_init(script):

    bcirc.register_script(script, script_name, script_version, script_author)
    bcirc.register_callback(script, "got_privmsg", kasittele_viesti, 20)

    alusta_sanonnat()

    print("Alustettiin sanonnat")

    return 1

if not running_inside_bcirc:
    alusta_sanonnat()
    print(hae_sanonta("paska"))
