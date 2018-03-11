# coding: utf-8
import tornado.ioloop
import tornado.gen
import time
import sys
sys.path.insert(0, "../../../protobuf/python/proto")
from datetime import datetime
from nats.io.utils  import new_inbox
from nats.io import Client as NATS
import message_pb2

@tornado.gen.coroutine
def main():
    nc = NATS()

    # Establish connection to the server.
    options = { "verbose": True, "servers": ["nats://127.0.0.1:4222"] }
    yield nc.connect(**options)

    def discover(msg=None):
        channel_message = message_pb2.ChannelMessage()
        channel_message.ParseFromString(msg.data)
        print("[Received]: %s" % channel_message)
    

    send_message = message_pb2.ChannelMessage()
    send_message.chan_num = 5
    #send_message.from = "python"
    send_message.message = "Hello, World!"

    sid = yield nc.subscribe("world.channel_message", "", discover)

    yield nc.publish("world.channel_message", send_message.SerializeToString())

    loop = tornado.ioloop.IOLoop.instance()
    yield tornado.gen.Task(loop.add_timeout, time.time() + 20)
    try:
        start = datetime.now()
        # Make roundtrip to the server and timeout after 1 second
        yield nc.flush(1)
        end = datetime.now()
        print("Latency: %d µs" % (end.microsecond - start.microsecond))
    except tornado.gen.TimeoutError, e:
        print("Timeout! Roundtrip too slow...")

if __name__ == '__main__':
    tornado.ioloop.IOLoop.instance().run_sync(main)