## Quest Dir Example

In the main binary directory, the dir layout is expected to be

- dotnet_quests
    - qeynos
        - Captain_Tillin.jsx
        - Executioner.jsx
        - player.jsx
    - qeynos2
        - Guard_Gehnus.jsx

When the zone boots up, it will create three files in the zone directory:

- qeynos.dll
- qeynos.pdb
- qeynos.cs

To debug with vscode, open the `dotnet_quests` folder as a working dir in VS code and run the .NET Core Attach debug and pick the process `zone` . To hit breakpoints, open qeynos.cs and set breakpoints accordingly and attach to the process. You will not be able to set breakpoints in the .csx files as they are just used to pull in text for the final compilation.

## External libs

If you want to include an external library (.dll) you need to place it in the binary directory and reference it with a using in the .csx file, that will instruct the assembly referencer in the compilation step to link to that dll.