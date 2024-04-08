## Prerequisites

The dotnet runtime >=8 is required to run dotnet quests. If you're on Debian, you can install from the command line

```
wget https://packages.microsoft.com/config/debian/12/packages-microsoft-prod.deb -O packages-microsoft-prod.deb
sudo dpkg -i packages-microsoft-prod.deb
rm packages-microsoft-prod.deb
sudo apt-get update && \
  sudo apt-get install -y aspnetcore-runtime-8.0
```

Or on windows from https://dotnet.microsoft.com/en-us/download/dotnet

## Directory Layout and Quest Examples

In the *working directory* where the `zone` executable is run (in akk-stack by default this will be `~/server`) this should be the dir layout:

- bin
    - zone
    - loginserver
    - etc...
- assets
- quests
- dotnet
    - dotnet quests
        - qeynos
            - Captain_Tillin.jsx
            - Executioner.jsx
            - player.jsx
        - qeynos2
            - Guard_Gehnus.jsx
    - RoslynBridge.dll
    - questinterface.so
    - etc.

When the zone boots up, it will create three files in the zone directory:

- qeynos.dll
- qeynos.pdb
- qeynos.cs

To debug with vscode, open the `dotnet_quests` folder as a working dir in VS code and run the .NET Core Attach debug and pick the process `zone` . To hit breakpoints, open `qeynos.cs` and set breakpoints accordingly and attach to the process. You will not be able to set breakpoints in the .csx files as they are just used to pull in text for the final compilation.

## Third party dotnet libs

If you want to include an external library (.dll) like a package from nuget, you need to place it in the `dotnet` directory and reference it with a using in the .csx file, that will instruct the assaembly referencer in the compilation step to link to that dll.

## Quest Example

An example quest is included that illustrates particular events and how they're handled for an NPC. To get the full benefit of intellisense it's recommend to use VSCode and install the C# extension which will provide all of the types and methods when developing. These quests are interoperable with existing LUA/PERL quests by receiving signal events and sending signal events as well.