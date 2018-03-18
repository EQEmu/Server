//Makes a player say a message in local chat
package main

import (
	"fmt"
	"log"
	"time"

	"github.com/eqemu/server/protobuf/go/eqproto"
	"github.com/golang/protobuf/proto"
	"github.com/nats-io/go-nats"
	"github.com/pkg/errors"
)

var (
	nc       *nats.Conn
	err      error
	entities []*eqproto.Entity
)

func main() {

	if nc, err = nats.Connect(nats.DefaultURL); err != nil {
		log.Fatal(err)
	}
	defer nc.Close()

	zone := "ecommons"
	instance := int64(0)
	entities = zoneEntityList(zone, 0)
	if len(entities) == 0 {
		return
	}
	fmt.Println(len(entities), "entities known")
	//fmt.Println(entities)

	var entityID int32
	for _, entity := range entities {
		if entity.Name == "Shin" {
			fmt.Println("Found Shin as ID", entity.Id)
			entityID = entity.Id
			break
		}
	}
	if entityID == 0 {
		log.Fatal("Can't find entity!")
	}
	go asyncChannelMessageSubscriber(nc) //async is recommended
	go entityEventSubscriber(zone, instance, entityID)
	err := zoneChannelMessage(zone, instance, entityID, eqproto.EntityType_Client, eqproto.MessageType_Say, "Hello, World!")
	//err := tell("shin", "Testing tell")
	if err != nil {
		fmt.Println("Failed to send channel message:", err.Error())
		return
	}
	time.Sleep(1000 * time.Second)
}

func tell(to string, message string) (err error) {

	msg := &eqproto.ChannelMessage{
		Message: message,
		From:    "go",
		To:      to,
	}

	d, err := proto.Marshal(msg)
	if err != nil {
		log.Fatal(err)
	}

	channel := fmt.Sprintf("world.channel_message.in")
	reply, err := nc.Request(channel, d, 1*time.Second)
	if err != nil {
		err = errors.Wrap(err, "Failed to get request response on zone channel")
		return
	}

	err = proto.Unmarshal(reply.Data, msg)
	if err != nil {
		err = errors.Wrap(err, "failed to unmarshal")
		return
	}

	if msg.ResponseError > 0 {
		err = errors.New(msg.ResponseMessage)
		return
	}
	fmt.Println("Response:", msg)
	return
}

func zoneChannelMessage(zone string, instance int64, fromEntityID int32, fromEntityType eqproto.EntityType, chanNumber eqproto.MessageType, message string) (err error) {

	msg := &eqproto.ChannelMessage{
		Message:        message,
		Number:         eqproto.MessageType_SayLocal, //chanNumber,
		FromEntityId:   fromEntityID,
		FromEntityType: fromEntityType,
		From:           "go",
		Distance:       500,
		SkipSender:     false,
		//To:             "shin",
	}

	d, err := proto.Marshal(msg)
	if err != nil {
		log.Fatal(err)
	}

	channel := fmt.Sprintf("zone.%s.%d.channel_message.in", zone, instance)
	reply, err := nc.Request(channel, d, 1*time.Second)
	if err != nil {
		err = errors.Wrap(err, "Failed to get request response on zone channel")
		return
	}

	err = proto.Unmarshal(reply.Data, msg)
	if err != nil {
		err = errors.Wrap(err, "failed to unmarshal")
		return
	}

	if msg.ResponseError > 0 {
		err = errors.New(msg.ResponseMessage)
		return
	}
	fmt.Println("Response:", msg)
	return
}

func testAttack(zone string, entityID int64, targetID int64) {
	time.Sleep(10 * time.Second)
	fmt.Println("10 seconds, Having", entityID, "attack", targetID)
	params := []string{
		fmt.Sprintf("%d", entityID),
		fmt.Sprintf("%d", targetID), //attack first element
		"1", //amount of hate
	}
	command := "attack"
	zoneCommand(zone, command, params)
}

func zoneEntityList(zone string, instanceID int) (entities []*eqproto.Entity) {
	msg := &eqproto.CommandMessage{
		Author:  "xackery",
		Command: "entitylist",
		Params:  []string{"client"},
	}

	d, err := proto.Marshal(msg)
	if err != nil {
		log.Fatal(err)
	}

	channel := fmt.Sprintf("zone.%s.command_message.in", zone)
	reply, err := nc.Request(channel, d, 1*time.Second)
	if err != nil {
		log.Println("Failed to get response on", channel, err.Error())
		return
	}

	err = proto.Unmarshal(reply.Data, msg)
	if err != nil {
		fmt.Println("Failed to unmarshal", err.Error())
		return
	}
	if msg.ResponseError > 0 {
		fmt.Println("Failed to get entity list:", msg.ResponseError, msg.ResponseMessage)
		return
	}

	//fmt.Println("reply", len(msg.Payload), string(msg.Payload))
	rootEntities := &eqproto.Entities{}
	err = proto.Unmarshal([]byte(msg.ResponsePayload), rootEntities)
	if err != nil {
		fmt.Println("failed to unmarshal entities", err.Error(), msg)
		return
	}
	entities = rootEntities.Entities
	return
}

func zoneCommandEntity(zone string, command string, params []string) (entityID int32) {
	msg := &eqproto.CommandMessage{
		Author:  "xackery",
		Command: command,
		Params:  params,
	}
	d, err := proto.Marshal(msg)
	if err != nil {
		log.Fatal(err)
	}
	reply, err := nc.Request(fmt.Sprintf("zone.%s.command_message.in", zone), d, 1*time.Second)
	if err != nil {
		log.Println("Failed to get request response:", err.Error())
		return
	}

	err = proto.Unmarshal(reply.Data, msg)
	if err != nil {
		fmt.Println("Failed to unmarshal", err.Error())
		return
	}
	//fmt.Println("Response:", msg)
	if msg.ResponseError > 0 {
		fmt.Println("Failed to get response:", msg.ResponseError, msg.ResponseMessage)
		return
	}

	entityID = msg.ResponseValue
	return
}

func zoneCommand(zone string, command string, params []string) {
	msg := &eqproto.CommandMessage{
		Author:  "xackery",
		Command: command,
		Params:  params,
	}
	d, err := proto.Marshal(msg)
	if err != nil {
		log.Fatal(err)
	}
	reply, err := nc.Request(fmt.Sprintf("zone.%s.command_message.in", zone), d, 1*time.Second)
	if err != nil {
		log.Println("Failed to get request response:", err.Error())
		return
	}

	err = proto.Unmarshal(reply.Data, msg)
	if err != nil {
		fmt.Println("Failed to unmarshal", err.Error())
		return
	}
	fmt.Println("Response:", msg)
	return
}

func entityEventSubscriber(zone string, instance int64, entityID int32) {

	var index int

	channel := fmt.Sprintf("zone.%s.%d.entity.%d.event.out", zone, instance, entityID)
	nc.Subscribe(channel, func(m *nats.Msg) {
		event := &eqproto.Event{}
		err = proto.Unmarshal(m.Data, event)
		if err != nil {
			fmt.Println("invalid event data passed", m.Data)
			return
		}

		var eventPayload proto.Message
		switch event.Op {
		case eqproto.OpCode_OP_ClientUpdate:
			eventPayload = &eqproto.PlayerPositionUpdateEvent{}
		case eqproto.OpCode_OP_Animation:
			eventPayload = &eqproto.AnimationEvent{}
		case eqproto.OpCode_OP_NewSpawn:
			eventPayload = &eqproto.SpawnEvent{}
		case eqproto.OpCode_OP_ZoneEntry:
			eventPayload = &eqproto.SpawnEvent{}
		case eqproto.OpCode_OP_HPUpdate:
			eventPayload = &eqproto.HPEvent{}
		case eqproto.OpCode_OP_MobHealth:
			eventPayload = &eqproto.HPEvent{}
		case eqproto.OpCode_OP_DeleteSpawn:
			eventPayload = &eqproto.DeleteSpawnEvent{}
		case eqproto.OpCode_OP_Damage:
			eventPayload = &eqproto.DamageEvent{}
		case eqproto.OpCode_OP_SpecialMesg:
			eventPayload = &eqproto.SpecialMessageEvent{}
		case eqproto.OpCode_OP_ChannelMessage:
			eventPayload = &eqproto.ChannelMessageEvent{}
		default:
			return
		}
		err = proto.Unmarshal(event.Payload, eventPayload)
		if err != nil {
			fmt.Println("Invalid data passed for opcode", event.Op, err.Error(), string(m.Data[index+1:]))
			return
		}
		fmt.Println(m.Subject, event.Op, eventPayload)
		//log.Printf("Received a message on %s: %s\n", m.Subject, string(m.Data))

		//proto.Unmarshal(m.Data, event)
		//log.Println(event.Op.String(), event.Entity, event.Target)
	})
	log.Println("Subscribed to", channel, ", waiting on messages...")

	time.Sleep(500 * time.Second)
}

// asyncChannelMessageSubscriber is an example of how to subscribe
// and invoke a function when a message is received
func asyncChannelMessageSubscriber(nc *nats.Conn) {
	nc.Subscribe("world.channel_message.out", func(m *nats.Msg) {
		message := &eqproto.ChannelMessage{}
		proto.Unmarshal(m.Data, message)
		log.Println(message)
	})
	nc.Subscribe("zone.ecommons.0.channel_message.out", func(m *nats.Msg) {
		message := &eqproto.ChannelMessage{}
		proto.Unmarshal(m.Data, message)
		log.Println(message)
	})
	log.Println("Waiting on async messages...")
}
