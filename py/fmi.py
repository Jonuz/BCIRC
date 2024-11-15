#! /usr/bin/python3

running_inside_bcirc = False

import requests
from dateutil import parser
from datetime import datetime
from datetime import timedelta
from dateutil import tz
from datetime import datetime
import time
import pytz
import xml.etree.ElementTree as ET
from bs4 import BeautifulSoup
import sys
import re

import ctypes

try:
    import bcirc

    running_inside_bcirc = True
except ImportError:
    pass

script_name = "FMI"
script_version = "0.1"
script_author = "Joona"

# Hanki avain täältä https://ilmatieteenlaitos.fi/rekisteroityminen-avoimen-datan-kayttajaksi
FMI_API_KEY = ""

last_request_time = 0


class FmiException(Exception):
    pass

def from_gmt_to_utc(gmt_date, timezone="Europe/Helsinki", handle_offset=True):
    if not isinstance(gmt_date, str):
        gmt_date = str(gmt_date)

    offset = datetime.now(pytz.timezone(timezone)).strftime('%z')
    try:
        dt = parser.parse(gmt_date + " GMT" + offset, dayfirst=True)
    except ValueError:
        raise FmiException("Aika käsittämättömässä muodossa.")
    uctd = dt.replace(tzinfo=pytz.utc)
    if handle_offset:
        uctd += + dt.tzinfo._offset
    return uctd


def get_last_item_with_value(soup, gmlId, asc=True):
    result = None
    time_series = soup.find("wml2:MeasurementTimeseries", {"gml:id": gmlId})

    if not time_series or not len(time_series):
        return None

    measurements = time_series.findAll("wml2:MeasurementTVP")

    for it in measurements:
        value = it.find("wml2:value")
        if value and value.string != "NaN":
            res_date = it.find("wml2:time")
            result = {
                "value": value.string,
                "date": res_date.string if res_date else None
            }
            if not asc:
                return result
    return result


def get_soup_value(soup, name, attr={}):
    res = soup.find(name, attr)
    return res.string if res else None


def request_info(place, gmt_time=None):
    result_pick_order_is_asc = False

    request_forecast = False

    if gmt_time:
        try:
            if parser.parse(gmt_time) > datetime.now():
                request_forecast = True
                print("Requesting forecast")
        except ValueError:
            raise FmiException("Aika käsittämättömässä muodossa.")
    else:
        gmt_time = str((datetime.now()).time())
        result_pick_order_is_asc = True

    start_time = from_gmt_to_utc(gmt_time, handle_offset=True)

    if not request_forecast:
        start_time = (start_time - timedelta(minutes=30))

    if not start_time:
        return
    end_time = (start_time + timedelta(minutes=60))

    # Metodit täältä http://data.fmi.fi/fmi-apikey/*KEY*/wfs?request=describeStoredQueries
    if not request_forecast:
        fmiId = "fmi::observations::weather::timevaluepair"
    else:
        fmiId = "fmi::forecast::hirlam::surface::point::timevaluepair"

    endpoint = "http://data.fmi.fi/fmi-apikey/" \
               + FMI_API_KEY + \
               "/wfs?request=getFeature&storedquery_id=" + fmiId + "&place=" \
               + place + "&timestep=10" + "&parameters=temperature,windspeedms,humidity,pressure,snowdepth,windgust" \
               + "&timezone=Europe/Helsinki"

    endpoint += "&starttime=" + start_time.strftime("%Y-%m-%dT%H:%M:%SZ")
    endpoint += "&endtime=" + end_time.strftime("%Y-%m-%dT%H:%M:%SZ")

    try:
        print(endpoint)
        res = requests.get(endpoint, timeout=5)

        if res.status_code is not 200:
            raise FmiException("Kylää ei löytynyt!".format(res.status_code))

        soup = BeautifulSoup(res.content, features="xml")

        prefix = "obs-obs" if not request_forecast else "mts"

        return {
            "is_forecast": request_forecast,
            "place": get_soup_value(soup, "gml:name", {"codeSpace": "http://xml.fmi.fi/namespace/locationcode/name"}),

            "temperature": get_last_item_with_value(soup, prefix + "-1-1-temperature", result_pick_order_is_asc),
            "windspeed": get_last_item_with_value(soup, prefix + "-1-1-windspeedms", result_pick_order_is_asc),
            "windgust": get_last_item_with_value(soup, prefix + "-1-1-windgust", result_pick_order_is_asc),
            "humidity": get_last_item_with_value(soup, prefix + "-1-1-humidity", result_pick_order_is_asc),
            "pressure": get_last_item_with_value(soup, prefix + "-1-1-pressure", result_pick_order_is_asc),
            "snowdepth": get_last_item_with_value(soup, prefix + "-1-1-snowdepth", result_pick_order_is_asc),
        }

    except (requests.exceptions.Timeout, requests.exceptions.ConnectionError):
        raise FmiException("Kysely aikakatkaistiin.")


def handle_request(params, count):
    srv = params[0]
    target = bcirc.get_string_from_ptr(params[3])

    msg = str(bcirc.get_string_from_ptr(params[4]))

    if not msg.startswith("!fmi "):
        return 1

    global last_request_time
    time_now = int(time.time())

    if time_now < last_request_time + 3:
        bcirc.privmsg_queue("Odota hetki!", target, srv, 1)
        return

    msg = msg[5:]

    place_and_time = parse_args(msg)

    place = place_and_time["place"]
    request_time = place_and_time["time"]

    try:
        last_request_time = time_now
        weather_data = request_info(place, request_time)
    except FmiException as e:
        print(e)
        bcirc.privmsg(str(e), target, srv)
        return 1

    if not weather_data or "temperature" not in weather_data:
        print("täh")
        bcirc.privmsg("Säätietoja ei löytynyt", target, srv)
        return

    response = format_message(weather_data)

    print(response)

    bcirc.privmsg(response, target, srv)

    return 1


def format_message(weather_data):
    formatted_date = None

    def has_value(key):
        return weather_data and weather_data[key] \
               and key in weather_data \
               and "value" in weather_data[key] \
               and weather_data[key]["value"] \
               and "date" in weather_data[key] \
               and weather_data[key]["date"]

    msg = ""

    if has_value("temperature"):
        formatted_date = parser.parse(weather_data["temperature"]["date"]).strftime("%d.%m.%Y %H:%M")

    if formatted_date and "place" in weather_data and weather_data["place"]:
        if not weather_data["is_forecast"]:
            msg = "Ilmastotiedot paikassa " + weather_data["place"] + " " + formatted_date + ";"
        else:
            msg = "Ennuste kohteeseen "+ weather_data["place"] + " " + formatted_date + ";"

    if has_value("temperature"):
        msg += " lämpötila " + weather_data["temperature"]["value"] + "°C"

    gustspeed = weather_data["windgust"]["value"]
    if has_value("windspeed"):
        msg += ", tuulen nopeus " + weather_data["windspeed"]["value"] + " (+" + gustspeed + ") m/s"

    if has_value("humidity"):
        msg += ", kosteus " + weather_data["humidity"]["value"] + "%"

    if has_value("snowdepth"):
        msg += ", lumensyvyys " + weather_data["snowdepth"]["value"] + " cm"

    if has_value("pressure"):
        msg += ", ilmanpaine " + weather_data["pressure"]["value"] + " hPa"

    return msg


def script_init(script):
    print("Setting up FMI script")

    bcirc.register_script(script, script_name, script_version, script_author)
    bcirc.register_callback(script, "got_privmsg", handle_request, 20)

    return 1


def parse_args(args):
    date = None

    time_starts = re.search("\d", args)

    if not time_starts:
        place = args
    else:
        place = args[0:time_starts.start()].strip()
        date = args[time_starts.start():].strip()

    print(date)

    return {
        "place": place,
        "time": date,
    }


if not running_inside_bcirc:
    args = ' '.join(sys.argv[1:])

    place_and_time = parse_args(args)

    res = request_info(place_and_time["place"], place_and_time["time"])
    msg = format_message(res)
    print(msg)
