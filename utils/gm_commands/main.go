package main

import (
	"fmt"
	"io/ioutil"
	"log"
	"os"
	"sort"
	"strings"
)

func main() {
	// zone/command.cpp
	commands, err := os.ReadFile("./zone/command.cpp")
	if err != nil {
		log.Fatal(err)
	}
	commandsString := string(commands)

	s := strings.Split(commandsString, "void command_")
	commandFiles := []string{}
	if len(s) > 1 {
		startListing := false
		for i, chunk := range s {
			if strings.Contains(chunk, "logcommand(Client *c") {
				startListing = true
			}

			// get function name
			functionName := ""
			nameSplit := strings.Split(chunk, "(Client")
			if len(nameSplit) > 0 {
				functionName = strings.TrimSpace(nameSplit[0])
			}

			if startListing &&
				len(s[i-1]) > 0 &&
				!strings.Contains(s[i-1], "#ifdef") &&
				!strings.Contains(chunk, "#ifdef") &&
				!strings.Contains(chunk, "#ifdef BOTS") &&
				!strings.Contains(chunk, "#ifdef EQPROFILE") &&
				!strings.Contains(functionName, "bot") &&
				!strings.Contains(functionName, "help") &&
				!strings.Contains(functionName, "findaliases") {

				fmt.Println(functionName)

				// build command file name
				commandFile := fmt.Sprintf("zone/gm_commands/%v.cpp", functionName)

				// append command file nam eto list
				commandFiles = append(commandFiles, commandFile)

				includes := ""
				if strings.Contains(chunk, "Client") {
					includes = fmt.Sprintf("%v%v\n", includes, "#include \"../client.h\"")
				}
				if strings.Contains(chunk, "parse->") {
					includes = fmt.Sprintf("%v%v\n", includes, "#include \"../quest_parser_collection.h\"")
				}
				if strings.Contains(chunk, "worldserver.") {
					includes = fmt.Sprintf("%v%v\n", includes, "#include \"../worldserver.h\"")
					includes = fmt.Sprintf("%v%v\n", includes, "extern WorldServer worldserver;")
				}
				if strings.Contains(chunk, "RegionType") {
					includes = fmt.Sprintf("%v%v\n", includes, "#include \"../water_map.h\"")
				}
				if strings.Contains(chunk, "Corpse") {
					includes = fmt.Sprintf("%v%v\n", includes, "#include \"../corpse.h\"")
				}
				if strings.Contains(chunk, "Object") {
					includes = fmt.Sprintf("%v%v\n", includes, "#include \"../object.h\"")
				}
				if strings.Contains(chunk, "DoorManipulation") {
					includes = fmt.Sprintf("%v%v\n", includes, "#include \"door_manipulation.h\"")
				}
				if strings.Contains(chunk, "Group") {
					includes = fmt.Sprintf("%v%v\n", includes, "#include \"../groups.h\"")
				}
				if strings.Contains(chunk, "httplib") {
					includes = fmt.Sprintf("%v%v\n", includes, "#include \"../../common/http/httplib.h\"")
				}
				if strings.Contains(chunk, "guild_mgr") {
					includes = fmt.Sprintf("%v%v\n", includes, "#include \"../guild_mgr.h\"")
				}
				if strings.Contains(chunk, "expedition") {
					includes = fmt.Sprintf("%v%v\n", includes, "#include \"../expedition.h\"")
				}
				if strings.Contains(chunk, "DataBucket::") {
					includes = fmt.Sprintf("%v%v\n", includes, "#include \"../data_bucket.h\"")
				}
				if strings.Contains(chunk, "file_exists") {
					includes = fmt.Sprintf("%v%v\n", includes, "#include \"../../common/file_util.h\"")
				}
				if strings.Contains(chunk, "std::thread") {
					includes = fmt.Sprintf("%v%v\n", includes, "#include <thread>")
				}
				if strings.Contains(chunk, "Door") {
					includes = fmt.Sprintf("%v%v\n", includes, "#include \"../doors.h\"")
				}
				if strings.Contains(chunk, "NOW_INVISIBLE") {
					includes = fmt.Sprintf("%v%v\n", includes, "#include \"../string_ids.h\"")
				}
				if strings.Contains(chunk, "Expansion::") {
					includes = fmt.Sprintf("%v%v\n", includes, "#include \"../../common/content/world_content_service.h\"")
				}
				if strings.Contains(chunk, "MobMovementManager::") {
					includes = fmt.Sprintf("%v%v\n", includes, "#include \"../mob_movement_manager.h\"")
				}
				if strings.Contains(chunk, "MobStuckBehavior::") {
					includes = fmt.Sprintf("%v%v\n", includes, "#include \"../mob_movement_manager.h\"")
				}
				if strings.Contains(chunk, "ReloadAllPatches") {
					includes = fmt.Sprintf("%v%v\n", includes, "#include \"../../common/patches/patches.h\"")
				}
				if strings.Contains(chunk, "ProfanityManager") {
					includes = fmt.Sprintf("%v%v\n", includes, "#include \"../../common/profanity_manager.h\"")
				}
				if strings.Contains(chunk, "npc_scale_manager") {
					includes = fmt.Sprintf("%v%v\n", includes, "#include \"../npc_scale_manager.h\"")
				}
				if strings.Contains(chunk, "g_Math") {
					includes = fmt.Sprintf("%v%v\n", includes, "#include \"../fastmath.h\"")
					includes = fmt.Sprintf("%v%v\n", includes, "extern FastMath g_Math;")
				}
				if strings.Contains(chunk, "raid") {
					includes = fmt.Sprintf("%v%v\n", includes, "#include \"../raids.h\"")
				}
				if strings.Contains(chunk, "Raid") {
					includes = fmt.Sprintf("%v%v\n", includes, "#include \"../raids.h\"")
				}
				if strings.Contains(chunk, "GetOS") {
					includes = fmt.Sprintf("%v%v\n", includes, "#include \"../../common/serverinfo.h\"")
				}
				if strings.Contains(chunk, "LANG_") {
					includes = fmt.Sprintf("%v%v\n", includes, "#include \"../../common/languages.h\"")
				}
				if strings.Contains(chunk, "ServerOP_Shared") {
					includes = fmt.Sprintf("%v%v\n", includes, "#include \"../../common/shared_tasks.h\"")
				}
				if strings.Contains(chunk, "title_manager") {
					includes = fmt.Sprintf("%v%v\n", includes, "#include \"../titles.h\"")
				}
				if strings.Contains(chunk, "CatchSignal") {
					includes = fmt.Sprintf("%v%v\n", includes, "#include \"../../world/main.h\"")
				}

				// build the contents of the command file
				commandString := fmt.Sprintf("%v\nvoid command_%v", includes, chunk)

				//write file contents
				err := ioutil.WriteFile(commandFile, []byte(commandString), 0777)
				if err != nil {
					fmt.Println(err)
				}

				commandOnly := fmt.Sprintf("void command_%v", chunk)
				commandsString = strings.ReplaceAll(commandsString, commandOnly, "")

			}
		}

		// rewrite commands.cpp with functions removed
		err := ioutil.WriteFile("zone/command.cpp", []byte(commandsString), 0777)
		if err != nil {
			fmt.Println(err)
		}

		fmt.Println("# CmakeLists")

		// sort a-z
		sort.Slice(commandFiles, func(i, j int) bool {
			return commandFiles[i] < commandFiles[j]
		})

		for _, file := range commandFiles {
			file = strings.ReplaceAll(file, "zone/", "")
			fmt.Println(file)
		}
	}

	//fmt.Print(string(commands))
}
