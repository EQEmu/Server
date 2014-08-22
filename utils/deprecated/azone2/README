
Azone2, awater, listobj, glModelViewer

azone2 creates .map files for EQEmu, used in LOS and BestZ, etc.

.S3D and .EQG zone files found with the Titanium EQ distribution are
supported.

to use all of these utilities, copy them to a directory containing your .S3D and .EQG
files.

IMPORTANT NOTE:

Older EQGs had the .ZON file within the .EQG container. Newer EQGs, e.g.
tutorialb, dreadspire, have the .ZON file as a separate file in your EQ directory.
To include placeable objects for these, YOU MUST have both the .EQG and .ZON file
in the same directory as azone2.

To generate a map file, specifit the shortname of the zone (without the extension). 

e.g.

./azone2 tox
./azone2 anguish

Copy the resultant .map file to the Maps directory on your server.


20/06/08 Derision:
   To include placeable objects, use the listobj program to find the models you want,
   e.g. ./listobj tox

LISTOBJ: List Placeable Objects in .S3D or .EQG zone files.
Placeable Object    0 @ (  -420.42,   2443.43,    -50.30 uses model   27 TIKI_DMSPRITEDEF
Placeable Object    1 @ (  -482.54,   2446.06,    -45.13 uses model   27 TIKI_DMSPRITEDEF
Placeable Object    2 @ (  -439.99,   2603.35,    -29.50 uses model   28 TORCH1_DMSPRITEDEF
Placeable Object    3 @ (   158.28,   1094.74,    -50.40 uses model   27 TIKI_DMSPRITEDEF
Placeable Object    4 @ (   137.83,   1074.79,    -49.91 uses model   17 HUT4_DMSPRITEDEF
Placeable Object    5 @ (   150.64,   1205.48,    -48.46 uses model   27 TIKI_DMSPRITEDEF
Placeable Object    6 @ (   128.19,   1204.95,    -48.37 uses model   27 TIKI_DMSPRITEDEF
Placeable Object    7 @ (   139.57,   1153.50,    -53.74 uses model   20 KDOCK_DMSPRITEDEF
Placeable Object    8 @ (  1139.42,   -837.60,    -55.38 uses model    5 ERBRAZIER_DMSPRITEDEF

                                                                       ^
								       |
								       |
				This is the model number  -------------|

If you want azone to include all occurrences of HUT4_DMSPRITEDF (model 17) and KDOCK_DMSPRITEDF (20),
you would add a line to azone.ini as follows:

# Include huts, houses and bridge from tox
tox.s3d,17,20
# Include two bridges from Tutorialb
tutorialb.eqg,17,47
# Include a couple of towers in Anguish
anguish.eqg,7,8

Note that as you can only specify a model number, it is not possible to only have selected occurrences
of that model included.

Lines in azone.ini beginning with a # are ignored.

TO INCLUDE PLACEABLE MODELS FOR .S3D ZONES, THE _OBJ.S3D FILE FOR THAT ZONE MUST ALSO BE PRESENT IN THE
SAME DIRECTORY.

You can also use glmodelviewer (Windows only) to display the models in a zone to find the ones you want
to include. To run it, from a command prompt, e.g.

glmodelviewer wallofslaughter

Finally, awater generates .wtr files containing details of the water, lava, PVP etc areas in an S3D zone.
Usage example:

./awater fieldofbone

The windows solution and project files provided were produced with Visual C++ 2008 and as configured,
require a statically linkable zlib.lib. 

