#!/usr/bin/env python
#
# Pelion Virtual Demo
# (C) COPYRIGHT 2021 Pelion Ltd.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# --------------------------------------------------------------------------
"""Pelion virtual demo bootstrap."""

import logging
import os
import subprocess
import threading
import posix_ipc
import asyncio
import contextlib

import tornado.escape
import tornado.ioloop
import tornado.options
import tornado.web
import tornado.websocket

from tornado.options import define, options

define("port", default=8888, help="run on the given port", type=int)

config = {}
sensor_type = "vibration"  # default to vibration

MQUEUE_CMD = "/mqueue-cmd"
MQUEUE_RESP = "/mqueue-resp"
qd_cmd = posix_ipc.MessageQueue(
    name=MQUEUE_CMD, flags=posix_ipc.O_CREAT, max_messages=10, max_message_size=256)
qd_resp = posix_ipc.MessageQueue(
    name=MQUEUE_RESP, flags=posix_ipc.O_CREAT, max_messages=10, max_message_size=256)


class Application(tornado.web.Application):
    def __init__(self):
        handlers = [(r"/", MainHandler), (r"/comsock", ComSocketHandler)]
        settings = dict(
            cookie_secret="__TODO:_GENERATE_YOUR_OWN_RANDOM_VALUE_HERE__",
            template_path=os.path.join(os.path.dirname(__file__), "templates"),
            static_path=os.path.join(os.path.dirname(__file__), "static"),
            xsrf_cookies=True,
        )
        # disable web logging access
        logging.getLogger('tornado.access').disabled = True
        # initialize mqueue handler
        MqueuHandler()

        super().__init__(handlers, **settings)


class MqueuHandler():
    def __init__(self):
        mqueue_listen_thread = threading.Thread(
            target=self.mqueue_resp_listen, daemon=True)
        mqueue_listen_thread.start()

    def mqueue_resp_listen(name):
        # logging.info("listening on (qd_resp)...")
        asyncio.set_event_loop(asyncio.new_event_loop())

        while True:
            s, _ = qd_resp.receive()
            s = s.decode()
            # logging.info("got (qd_resp) msg: '%s'", s)
            ComSocketHandler.send_update(s)


class MainHandler(tornado.web.RequestHandler):
    def get(self):
        self.render("index.html", sensor_type=os.getenv('SENSOR', 'vibration'))


class ComSocketHandler(tornado.websocket.WebSocketHandler):
    waiters = set()

    def get_compression_options(self):
        # Non-None enables compression with default options.
        return {}

    def open(self):
        # logging.info("open(self)")
        ComSocketHandler.waiters.add(self)

    def on_close(self):
        # logging.info("on_close(self)")
        ComSocketHandler.waiters.remove(self)

    @classmethod
    def send_update(cls, message):
        for waiter in cls.waiters:
            try:
                waiter.write_message(message)
            except:
                logging.error("Error sending message", exc_info=True)

        # logging.info("sent (ws) msg to %d waiters", len(cls.waiters))

    def on_message(self, message):
        # logging.info("got (ws) msg: %r", message)

        # add null termination expected from C backend
        cmd = message.encode('ascii') + b'\x00'
        qd_cmd.send(cmd)
        # logging.info("sent (qd_cmd) msg: '%s'", cmd)

def gencerts():
    # invoke Pelion 'dev_init.py' to create certs
    subprocess.Popen(['./dev_init.py', 'with-credentials', '-a', config["api_key"],
                    '-u', 'https://api.us-east-1.mbedcloud.com'],
                    cwd='/build/mbed-cloud-client-example/utils').wait()

def build():
    # delete the last instance of the app so that we don't automatically
    # execute the old app if the new build fails
    with contextlib.suppress(FileNotFoundError):
        os.remove('/build/mbed-cloud-client-example/__x86_x64_NativeLinux_mbedtls/Debug/mbedCloudClientExample.elf')

    # spawn process to build pelion-client
    subprocess.Popen(['make', 'mbedCloudClientExample.elf'],
                     cwd='/build/mbed-cloud-client-example/__x86_x64_NativeLinux_mbedtls').wait()


def _main():
    tornado.options.parse_command_line()

    # check if CLOUD_SDK_API_KEY env is configured
    try:
        config["api_key"] = os.environ['CLOUD_SDK_API_KEY']

    except KeyError as e:
        logging.error(
            'Missing CLOUD_SDK_API_KEY environmental key !'
        )
        exit(1)

    # check if SENSOR env is configured
    if "SENSOR" in os.environ:
        sensor_type = os.environ['SENSOR']
        if sensor_type != "vibration" and sensor_type != "temperature":
            logging.error(
                "unknown sensor type configured, please use either 'vibration' or 'temperature'\n"
            )
            exit(1)

    # check if we need to generate certs
    if not os.path.isfile('certexists'):
        # generate dev and fw certs
        gencerts()
        # raise flag so that we don't regenerate on next run
        open('certexists', 'w').close()

    # check if we need to build app
    if not os.path.isfile('firstrun'):
        # build application
        build()
        # raise flag so that we don't rebuild on next run
        open('firstrun', 'w').close()

    # launch pelion client in a separate process
    subprocess.Popen(['./mbedCloudClientExample.elf'],
                     cwd='/build/mbed-cloud-client-example/__x86_x64_NativeLinux_mbedtls/Debug/')

    # launch web app
    app = Application()
    app.listen(options.port)
    tornado.ioloop.IOLoop.current().start()


if __name__ == "__main__":
    _main()
