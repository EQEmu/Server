package main

import (
	"fmt"
	"log"
	"time"

	"github.com/eqemu/server/protobuf/go/eqproto"
	"github.com/golang/protobuf/proto"
	"github.com/nats-io/go-nats"
)

func main() {
	fmt.Println("Starting...")
	var nc *nats.Conn
	var err error

	//create a nats connection, by default 127.0.0.1
	if nc, err = nats.Connect(nats.DefaultURL); err != nil {
		log.Fatal(err)
	}
	defer nc.Close()

	//listen for any channel messages from game
	go asyncChannelMessageSubscriber(nc) //async is recommended
	//go  syncChannelMessageSubscriber() //sync is here as example

	//send a channel message to broadcast channel
	go testBroadcastMessage(nc, "Hello, World!")

	time.Sleep(20 * time.Second)
	fmt.Println("Exited after 20 seconds")
}

// asyncChannelMessageSubscriber is an example of how to subscribe
// and invoke a function when a message is received
func asyncChannelMessageSubscriber(nc *nats.Conn) {
	nc.Subscribe("world.channel_message", func(m *nats.Msg) {
		message := &eqproto.ChannelMessage{}
		proto.Unmarshal(m.Data, message)
		log.Println(message)
	})
	log.Println("Waiting on async messages...")
}

// syncChannelMessageSubscriber is an example of how to subscribe
// and poll for messages syncronously
func syncChannelMessageSubscriber(nc *nats.Conn) {

	sub, err := nc.SubscribeSync("world.channel_message.out")
	if err != nil {
		log.Fatal(err)
	}
	var m *nats.Msg
	if m, err = sub.NextMsg(10 * time.Second); err != nil {
		log.Println("Timed out after 10 seconds waiting for message", err.Error())
		return
	}

	message := &eqproto.ChannelMessage{}
	proto.Unmarshal(m.Data, message)
	log.Println("Got message", message)
}

func testBroadcastMessage(nc *nats.Conn, msg string) {
	message := &eqproto.ChannelMessage{
		From:    "go",
		Message: msg,
		ChanNum: 5, //5 is ooc, 6 is bc
	}
	d, err := proto.Marshal(message)
	if err != nil {
		log.Fatal(err)
	}
	if err = nc.Publish("world.channel_message.in", d); err != nil {
		log.Println("Failed to publish:", err.Error())
		return
	}
	log.Println("Sending message", message)
}
