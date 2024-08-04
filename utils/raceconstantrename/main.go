package main

import (
	"fmt"
	"github.com/gammazero/workerpool"
	"io/fs"
	"log"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"strconv"
	"strings"
)

func main() {
	loadDefinitions()

	// get processor count
	wp := workerpool.New(runtime.NumCPU())

	// loop through all files in current dir that are cpp files or h files
	err := filepath.WalkDir("../../", func(path string, d fs.DirEntry, err error) error {
		if d.IsDir() {
			return nil
		}

		if !strings.Contains(path, ".cpp") && !strings.Contains(path, ".h") {
			return nil
		}

		// if file ends with ".o" skip it
		if strings.HasSuffix(path, ".o") {
			return nil
		}

		var ignoreFiles = []string{
			"submodules", "/libs", "utils/", "races.h", "backward", "database_update_manifest.cpp", "zonedb.h",
		}

		ignore := false
		for _, ignoreString := range ignoreFiles {
			if strings.Contains(path, ignoreString) {
				ignore = true
				break
			}
		}
		if ignore {
			return nil
		}

		wp.Submit(func() {

			// open file for reading
			// get file contents
			contents, err := os.ReadFile(path)
			if err != nil {
				log.Fatalf("impossible to read file: %s", err)
			}

			content := string(contents)

			wroteChanges := false

			var newLines []string
			for _, line := range strings.Split(content, "\n") {
				newLine := line

				// loop through oldDefs and see if any of them are in contents
				for key, value := range oldDefs {
					// combine all of the above contains into a slice
					// loop through slice and if any of them are in line, continue
					var ignoreMatches = []string{
						"#define ", "MALE", "FEMALE", "_BIT", "LANG_",
					}

					ignore := false
					for _, ignoreString := range ignoreMatches {
						if strings.Contains(newLine, ignoreString) && !strings.Contains(newLine, "NPC_") {
							ignore = true
							break
						}
					}
					if ignore {
						continue
					}

					// below we hackishly use a series of specific string contains to avoid
					// making blind and wrong replacements
					// but hey - at least its 100% accurate :)
					if strings.Contains(line, "case "+key+":") {
						for key2, value2 := range newDefs {
							if value == value2 {
								newLine = strings.ReplaceAll(newLine, " "+key+":", " "+key2+":")
								wroteChanges = true
								break
							}
						}
					}
					if strings.Contains(line, "\t"+key) {
						for key2, value2 := range newDefs {
							if value == value2 {
								newLine = strings.ReplaceAll(newLine, "\t"+key, "\t"+key2)
								wroteChanges = true
								break
							}
						}
					}
					if strings.Contains(line, key+",") {
						for key2, value2 := range newDefs {
							if value == value2 {
								newLine = strings.ReplaceAll(newLine, key+",", key2+",")
								wroteChanges = true
								break
							}
						}
					}
					if strings.Contains(line, ", "+key) {
						for key2, value2 := range newDefs {
							if value == value2 {
								newLine = strings.ReplaceAll(newLine, ", "+key, ", "+key2)
								wroteChanges = true
								break
							}
						}
					}
					if strings.Contains(line, "= "+key+" ") {
						for key2, value2 := range newDefs {
							if value == value2 {
								newLine = strings.ReplaceAll(newLine, "= "+key+" ", "= "+key2+" ")
								wroteChanges = true
								break
							}
						}
					}
					if strings.Contains(line, "= "+key+")") {
						for key2, value2 := range newDefs {
							if value == value2 {
								newLine = strings.ReplaceAll(newLine, "= "+key+")", "= "+key2+")")
								wroteChanges = true
								break
							}
						}
					}
					if strings.Contains(line, "= "+key+";") {
						for key2, value2 := range newDefs {
							if value == value2 {
								newLine = strings.ReplaceAll(newLine, "= "+key+";", "= "+key2+";")
								wroteChanges = true
								break
							}
						}
					}
					if strings.Contains(line, "= "+key+" ||") {
						for key2, value2 := range newDefs {
							if value == value2 {
								newLine = strings.ReplaceAll(newLine, "= "+key+" ||", "= "+key2+" ||")
								wroteChanges = true
								break
							}
						}
					}

					// match cases where our match is on the last line and last column
					// we need to be exact in the last column and not do a partial because we can
					// accidentally rename say OGRE to OGRE2 mistakenly
					if strings.Contains(line, key) {
						columns := strings.Split(line, " ")
						// get the last column
						lastColumn := strings.TrimSpace(columns[len(columns)-1])
						if lastColumn == key {
							for key2, value2 := range newDefs {
								if value == value2 {
									newLine = strings.ReplaceAll(newLine, lastColumn, key2)
									wroteChanges = true
									break
								}
							}
						}
					}

					//if strings.Contains(line, "race == "+key) {
					//	for key2, value2 := range newDefs {
					//		if value == value2 {
					//			newLine = strings.ReplaceAll(newLine, "race == "+key, "race == "+key2)
					//			wroteChanges = true
					//			break
					//		}
					//	}
					//}
				}

				newLines = append(newLines, newLine)
			}

			// write contents back to file
			if wroteChanges {
				fmt.Printf("wrote changes to file [%v]\n", path)
				err = os.WriteFile(path, []byte(strings.Join(newLines, "\n")), 0644)
				if err != nil {
					log.Fatalf("impossible to write file: %s", err)
				}
			}

			return
		})
		return nil
	})
	if err != nil {
		log.Fatalf("impossible to walk directories: %s", err)
	}

	wp.StopWait()
}

var newDefs = make(map[string]int)
var oldDefs = make(map[string]int)

func loadDefinitions() {
	// git show master:common/races.h
	cmd := exec.Command("git", "show", "master:common/races.h")
	out, err := cmd.Output()
	if err != nil {
		println(err.Error())
		return
	}

	// load into a string -> int map
	for _, line := range strings.Split(string(out), "\n") {
		if strings.Contains(line, "#define ") {
			if len(strings.Split(line, " ")) <= 2 {
				continue
			}

			// ignore
			// #define MALE 0
			// #define FEMALE 1
			// #define NEUTER 2
			if strings.Contains(line, "#define MALE") {
				continue
			}
			if strings.Contains(line, "#define FEMALE") {
				continue
			}
			if strings.Contains(line, "#define NEUTER") {
				continue
			}

			// load "#define RACE_FLYING_CARPET_720 720" into map

			key := strings.Split(line, " ")[1]
			value := strings.Split(line, " ")[2]
			value = strings.ReplaceAll(value, "//", "")
			value = strings.TrimSpace(value)
			//fmt.Printf("key [%v] value [%v]\n", key, value)

			if !strings.HasPrefix(key, "RACE_") && !strings.HasPrefix(key, "RT_") {
				continue
			}

			// convert value to int
			intValue, err := strconv.Atoi(value)
			if err != nil {
				println(err.Error())
				return
			}

			oldDefs[key] = intValue

			fmt.Printf("oldDefs key [%v] value [%v]\n", key, intValue)
		}
	}

	// cleanup/races_cpp_h
	cmd = exec.Command("git", "show", "cleanup/races_cpp_h:common/races.h")
	out, err = cmd.Output()
	if err != nil {
		println(err.Error())
		return
	}

	// load into a string -> int map
	for _, line := range strings.Split(string(out), "\n") {
		if strings.Contains(line, "constexpr uint16") {
			if len(strings.Split(line, " ")) <= 2 {
				continue
			}

			// remove all extra spaces in between characters in line
			line = strings.Join(strings.Fields(line), " ")

			// load "	constexpr uint16 Doug = 0;" into map
			key := strings.Split(line, " ")[2]
			value := strings.Split(line, " ")[4]
			value = strings.ReplaceAll(value, "//", "")
			value = strings.ReplaceAll(value, ";", "")
			value = strings.TrimSpace(value)

			// convert value to int
			intValue, err := strconv.Atoi(value)
			if err != nil {
				println(err.Error())
				return
			}

			mapKey := "Race::" + key

			newDefs[mapKey] = intValue

			fmt.Printf("newDefs key [%v] value [%v]\n", mapKey, value)
		}
	}
}
