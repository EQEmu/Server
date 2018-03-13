//Makes a player say a message in local chat
package main

import (
	"fmt"
	"log"
	"strconv"
	"time"

	"github.com/eqemu/server/protobuf/go/eqproto"
	"github.com/golang/protobuf/proto"
	"github.com/nats-io/go-nats"
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

	fmt.Println(len(entities), "entities known")

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

	go entityEventSubscriber(zone, instance, entityID)
	zoneChannel(zone, instance, entityID, eqproto.EntityType_Client, 256, "Hello, World!")
	time.Sleep(1000 * time.Second)
}

func zoneChannel(zone string, instance int64, fromEntityID int32, fromEntityType eqproto.EntityType, chanNumber int32, message string) {

	msg := &eqproto.ChannelMessage{
		Message:        message,
		ChanNum:        chanNumber,
		FromEntityId:   fromEntityID,
		FromEntityType: fromEntityType,
		Distance:       500,
		SkipSender:     true,
	}
	d, err := proto.Marshal(msg)
	if err != nil {
		log.Fatal(err)
	}
	channel := fmt.Sprintf("zone.%s.channel_message.in", zone)
	log.Println(channel, "sending channel request", msg)
	reply, err := nc.Request(channel, d, 1*time.Second)
	if err != nil {
		log.Println("Failed to get request response on zone channel:", err.Error())
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
		log.Println("Failed to get response on", channel, "", err.Error())
		return
	}

	err = proto.Unmarshal(reply.Data, msg)
	if err != nil {
		fmt.Println("Failed to unmarshal", err.Error())
		return
	}

	rootEntities := &eqproto.Entities{}
	err = proto.Unmarshal([]byte(msg.Payload), rootEntities)
	if err != nil {
		fmt.Println("failed to unmarshal entities", err.Error(), msg)
		return
	}
	entities = rootEntities.Entities
	return
}

func zoneCommandEntity(zone string, command string, params []string) (entityID int64) {
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
	entityID, err = strconv.ParseInt(msg.Result, 10, 64)
	if err != nil {
		fmt.Println("Failed to parse response", err.Error(), msg.Result)
		return
	}
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

	/*event := &eqproto.EntityEvent{
		Entity: &eqproto.Entity{
			Id: 1,
		},
	}
	d, err := proto.Marshal(event)
	if err != nil {
		log.Fatal(err)
	}
	if err = nc.Publish(fmt.Sprintf("zone.%s.entity.event_subscribe.all", zone), d); err != nil {
		log.Println("Failed to publish event subscribe:", err.Error())
		return
	}*/

	var opCode int64
	var index int
	channel := fmt.Sprintf("zone.%s.%d.entity.%d.event.out", zone, instance, entityID)
	nc.Subscribe(channel, func(m *nats.Msg) {
		event := &eqproto.Event{}
		err = proto.Unmarshal(m.Data, event)
		if err != nil {
			fmt.Println("invalid event data passed", m.Data)
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
		default:
			return
		}
		err = proto.Unmarshal(event.Payload, eventPayload)
		if err != nil {
			fmt.Println("Invalid data passed for opcode", eqproto.OpCode(opCode), err.Error(), string(m.Data[index+1:]))
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
