package main

import (
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"github.com/google/go-github/v41/github"
	"golang.org/x/oauth2"
	"log"
	"net/http"
	"os"
	"path/filepath"
	"strings"
)

type PackageJson struct {
	Name       string `json:"name"`
	Version    string `json:"version"`
	Repository struct {
		Type string `json:"type"`
		URL  string `json:"url"`
	} `json:"repository"`
}

func main() {
	// get latest release from github
	client := github.NewClient(nil)
	if len(os.Getenv("GITHUB_TOKEN")) > 0 {
		ts := oauth2.StaticTokenSource(
			&oauth2.Token{AccessToken: os.Getenv("GITHUB_TOKEN")},
		)
		tc := &http.Client{
			Transport: &oauth2.Transport{
				Source: ts,
			},
		}
		client = github.NewClient(tc)
	}

	release, _, err := client.Repositories.GetLatestRelease(context.Background(), "EQEmu", "Server")
	if err != nil {
		log.Println(err)
		os.Exit(1)
	}

	version := strings.ReplaceAll(*release.TagName, "v", "")

	// get current version from package.json
	currentLevel := filepath.Join("./package.json")
	packageJsonFile := currentLevel
	if _, err := os.Stat(currentLevel); errors.Is(err, os.ErrNotExist) {
		packageJsonFile = ""
		// this is only really needed when developing this binary
		walkUpToRoot := filepath.Join("../../../../package.json")
		if _, err := os.Stat(walkUpToRoot); err == nil {
			// path/to/whatever exists
			packageJsonFile = walkUpToRoot
		}
	}

	if len(packageJsonFile) == 0 {
		fmt.Printf("Could not find package.json\n")
		os.Exit(1)
	}

	packageJson, err := os.ReadFile(packageJsonFile)
	var p PackageJson
	_ = json.Unmarshal(packageJson, &p)

	// version compare
	if p.Version == version {
		fmt.Printf("Version [%v] already exists. No need to release\n", version)
		fmt.Printf("Exiting code 78 to halt pipeline steps gracefully\n")
		os.Exit(78)
	}
}
