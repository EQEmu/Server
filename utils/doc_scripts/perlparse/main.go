//Parses perl scripts
package main

import (
	"bufio"
	"fmt"
	"log"
	"os"
	"regexp"
	"strings"

	"github.com/pkg/errors"
)

func main() {
	path := "../../../zone/embparser_api.cpp"
	err := readFile(path)
	if err != nil {
		log.Panicf("Failed to read file: %s", err.Error())
	}
}

type API struct {
	Function  string
	Arguments []*Argument
}

type Argument struct {
	Name string
	Type string
	API  *API
}

func readFile(path string) (err error) {
	inFile, err := os.Open(path)
	if err != nil {
		err = errors.Wrap(err, "Failed to open file")
	}
	defer inFile.Close()
	scanner := bufio.NewScanner(inFile)
	scanner.Split(bufio.ScanLines)

	arguments := map[string][]*Argument{}
	functions := []*API{}
	reg, err := regexp.Compile(`\]+|\[+|\?+|[...]+`)
	if err != nil {
		err = errors.Wrap(err, "Failed to compile regex")
		return
	}
	regType, err := regexp.Compile(`(unsigned long|long|int32|bool|uint[0-9]+|int|auto|float|unsigned int|char[ \*]).+([. a-zA-Z]+=)`)
	if err != nil {
		err = errors.Wrap(err, "Failed to compile type regex")
		return
	}

	lastArguments := []*Argument{}
	lastAPI := &API{}
	lineNum := 0
	for scanner.Scan() {
		lineNum++
		key := ""
		line := scanner.Text()
		if len(line) < 1 {
			continue
		}
		if len(lastArguments) > 0 { //existing args to parse
			for i, argument := range lastArguments {
				key = fmt.Sprintf("ST(%d)", i)
				if strings.Contains(line, key) {
					//We found a definition argument line
					if argument.Type != "" {
						continue
					}

					match := regType.FindStringSubmatch(line)
					if len(match) < 2 {
						continue
					}

					//key = `int`
					//function = line[strings.Index(line, key)+len(key):]
					newType := ""

					switch v := strings.TrimSpace(match[1]); v {
					case "int":
						newType = "int"
					case "int32":
						newType = "int"
					case "float":
						newType = "float"
					case "unsigned int":
						newType = "uint"
					case "uint32":
						newType = "uint"
					case "uint8":
						newType = "uint"
					case "uint":
						newType = "uint"
					case "bool":
						newType = "bool"
					case "uint16":
						newType = "uint"
					case "long":
						newType = "long"
					case "unsigned long":
						newType = "unsigned long"
					case "char":
						newType = "string"
					case "auto":
						//Auto is tricky
						if strings.Contains(line, "glm::vec4") {
							newType = "float"
						}

					default:
						log.Printf(`Unknown type: "%s" on line %d`, v, lineNum)
					}
					//log.Println("Found arg type", newType, "on index", i, argument.Name)
					lastArguments[i].Type = newType
				}
			}
		}

		function := ""

		argLine := ""
		args := []string{}
		//Find line
		key = `Perl_croak(aTHX_ "Usage:`
		if strings.Contains(line, key) {
			function = line[strings.Index(line, key)+len(key):]
		}

		for _, argument := range lastArguments {
			arguments[argument.Name] = append(arguments[argument.Name], argument)
		}

		lastArguments = []*Argument{}

		//Trim off the endings
		key = `");`
		if strings.Contains(function, key) {
			function = function[0:strings.Index(function, key)]
		}
		//Strip out the arguments
		key = `(`
		if strings.Contains(function, key) {
			argLine = function[strings.Index(function, key)+len(key):]
			function = function[0:strings.Index(function, key)]
			key = `)`
			if strings.Contains(argLine, key) {
				argLine = argLine[:strings.Index(argLine, key)]
			}
			key = `=`
			if strings.Contains(argLine, key) {
				argLine = argLine[:strings.Index(argLine, key)]
			}
			argLine = reg.ReplaceAllString(argLine, "")
		}
		key = `,`
		argLine = strings.TrimSpace(argLine)

		if strings.Contains(argLine, key) {
			args = strings.Split(argLine, key)
		}

		if len(function) < 1 {
			continue
		}

		newArgs := []string{}
		for j, _ := range args {
			args[j] = strings.TrimSpace(args[j])
			if len(args[j]) == 0 {
				continue
			}
			newArgs = append(newArgs, args[j])
		}

		lastAPI = &API{
			Function: function,
		}

		for _, arg := range newArgs {
			argType, _ := knownTypes[arg]
			argument := &Argument{
				Name: arg,
				Type: argType,
				API:  lastAPI,
			}
			lastArguments = append(lastArguments, argument)
		}
		lastAPI.Arguments = lastArguments

		functions = append(functions, lastAPI)
	}

	foundCount := 0
	failCount := 0
	for key, val := range arguments {
		isMissing := false
		line := ""
		line = fmt.Sprintf("%s used by %d functions:", key, len(val))
		for _, fnc := range val {
			line += fmt.Sprintf("%s(%s %s), ", fnc.API.Function, fnc.Type, key)
			if fnc.Type == "" {
				isMissing = true
			}
		}
		if isMissing {
			fmt.Println(line)
			failCount++
		} else {
			foundCount++
		}
	}
	log.Println(foundCount, "functions properly identified,", failCount, "have errors")

	line := ""
	for _, api := range functions {
		line += fmt.Sprintf("void %s(", strings.TrimSpace(api.Function))
		for _, argument := range api.Arguments {
			line += fmt.Sprintf("%s %s, ", argument.Type, argument.Name)
		}
		if len(api.Arguments) > 0 {
			line = line[0 : len(line)-2]
		}
		line += ")\n"
	}
	fmt.Println(line)
	return
}

var knownTypes = map[string]string{
	"activity_id":               "uint",
	"alt_mode":                  "bool",
	"anim_num":                  "int",
	"best_z":                    "float",
	"buttons":                   "int",
	"channel_id":                "int",
	"char_id":                   "int",
	"charges":                   "int",
	"class_id":                  "int",
	"client_name":               "string",
	"color":                     "int",
	"color_id":                  "int",
	"condition_id":              "int",
	"copper":                    "int",
	"count":                     "int",
	"debug_level":               "int",
	"decay_time":                "int",
	"dest_heading":              "float",
	"dest_x":                    "float",
	"dest_y":                    "float",
	"dest_z":                    "float",
	"distance":                  "int",
	"door_id":                   "int",
	"doorid":                    "uint",
	"duration":                  "int",
	"effect_id":                 "int",
	"elite_material_id":         "int",
	"enforce_level_requirement": "bool",
	"explore_id":                "uint",
	"faction_value":             "int",
	"fade_in":                   "int",
	"fade_out":                  "int",
	"fadeout":                   "uint",
	"firstname":                 "string",
	"from":                      "string",
	"gender_id":                 "int",
	"gold":                      "int",
	"grid_id":                   "int",
	"guild_rank_id":             "int",
	"heading":                   "float",
	"hero_forge_model_id":       "int",
	"ignore_quest_update":       "bool",
	"instance_id":               "int",
	"int_unused":                "int",
	"int_value":                 "int",
	"is_enabled":                "bool",
	"is_strict":                 "bool",
	"item_id":                   "int",
	"key":                       "string",
	"language_id":               "int",
	"lastname":                  "string",
	"leader_name":               "string",
	"level":                     "int",
	"link_name":                 "string",
	"macro_id":                  "int",
	"max_level":                 "int",
	"max_x":                     "float",
	"max_y":                     "float",
	"max_z":                     "float",
	"message":                   "string",
	"milliseconds":              "int",
	"min_level":                 "int",
	"min_x":                     "float",
	"min_y":                     "float",
	"min_z":                     "float",
	"name":                      "string",
	"new_hour":                  "int",
	"new_min":                   "int",
	"node1":                     "int",
	"node2":                     "int",
	"npc_id":                    "int",
	"npc_type_id":               "int",
	"object_type":               "int",
	"options":                   "int",
	"platinum":                  "int",
	"popup_id":                  "int",
	"priority":                  "int",
	"quantity":                  "int",
	"race_id":                   "int",
	"remove_item":               "bool",
	"requested_id":              "int",
	"reset_base":                "bool",
	"saveguard":                 "bool",
	"seconds":                   "int",
	"send_to_world":             "bool",
	"signal_id":                 "int",
	"silent":                    "bool",
	"silver":                    "int",
	"size":                      "int",
	"stat_id":                   "int",
	"str_value":                 "string",
	"subject":                   "string",
	"target_enum":               "string",
	"target_id":                 "int",
	"task":                      "int",
	"task_id":                   "uint",
	"task_id1":                  "int",
	"task_id10":                 "int",
	"task_id2":                  "int",
	"task_set":                  "int",
	"taskid":                    "int",
	"taskid1":                   "int",
	"taskid2":                   "int",
	"taskid3":                   "int",
	"taskid4":                   "int",
	"teleport":                  "int",
	"temp":                      "int",
	"texture_id":                "int",
	"theme_id":                  "int",
	"update_world":              "int",
	"updated_time_till_repop":   "uint",
	"version":                   "int",
	"wait_ms":                   "int",
	"window_title":              "string",
	"x":                         "float",
	"y":                         "float",
	"z":                         "float",
	"zone_id":                   "int",
	"zone_short":                "string",
	`task_id%i`:                 "int",
}
