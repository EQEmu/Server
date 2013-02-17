#include <stdio.h>
#include <string>
#include "PFSArchive.h"

int main(int argc, char **argv) {
    if(argc < 3) {
        printf("Usage: %s ext file1 [file2...]\n", argv[0]);
        return 1;
    }

    for(int i = 2; i < argc; ++i) {
        PFSArchive archive;
        if(!archive.Open(argv[i])) {
            printf("Error: couldn't open %s\n", argv[i]);
            continue;
        }

        std::list<std::string> files;
        if(!archive.GetFiles(argv[1], files)) {
            printf("Error: couldn't return the files with ext: %s\n", argv[1]);
            continue;
        }

        printf("Files in %s:\n", argv[i]);
        std::list<std::string>::const_iterator iter = files.begin();
        while(iter != files.end()) {
            printf("%s\n", (*iter).c_str());
            iter++;
        }
        printf("\n");

        archive.Close();
    }
    return 0;
}
