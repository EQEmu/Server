#!/usr/bin/perl

# File Name: conf_to_oplist.pl
# Converts a Patch File into the Opcode List with Opcode Names for the Spreadsheet.

# Directions to use this script:
# 1. Paste the contents of the current patch file in the patch_OLD.conf file.
# 2. Run this script using "perl conf_to_oplist.pl"
# 3. This updates the opcodelist.txt with the values from the Patch File.


$stopmessage = "Failed to open file";
open OpcodeFile, "<", "opcodelist.txt" or die $stopmessage;
open PatchFile, "<", "patch_OLD.conf" or die $stopmessage;

my @OpcodeList = <OpcodeFile>;
my @PatchFile = <PatchFile>;
my %PatchHash = ();

foreach $line (@PatchFile)
{
        @equalssplit = split(/=/, $line);
        $ArraySize = @equalssplit;
        if ($ArraySize > 1)
        {
                @OpcodeArray = split(//, $equalssplit[1]);
                $CurOpcode = $OpcodeArray[0].$OpcodeArray[1].$OpcodeArray[2].$OpcodeArray[3].$OpcodeArray[4].$OpcodeArray[5];
                $CurOpcode = lc($CurOpcode);
                # Opcode Name => Opcode
                $PatchHash{ $CurOpcode } = $equalssplit[0];
        }
}

close(OpcodeFile);
close(PatchFile);

# Clear out file contents
open OpcodeResultFile, ">", "opcodelist.txt" or die $stopmessage;
print OpcodeResultFile "";
close(OpcodeResultFile);

open OpcodeResultFile, ">>", "opcodelist.txt" or die $stopmessage;

while( my ($k, $v) = each %$PatchFile )
{
        #print OpcodeResultFile "key: $k, value: $v.\n";
}


$TabSpace = "   ";      # Tab
foreach $line (@OpcodeList)
{
        @LineSplit = split(//, $line);
        $CurOpcode = $LineSplit[0].$LineSplit[1].$LineSplit[2].$LineSplit[3].$LineSplit[4].$LineSplit[5];
        $CurOpcode = lc($CurOpcode);
        $OpcodeName = "";       # Tab
        if ($PatchHash{$CurOpcode})
        {
                $NameKey = $PatchHash{$CurOpcode};
                $OpcodeName = $NameKey;
        }
        $CurLine = $CurOpcode.$TabSpace.$OpcodeName."\n";
        print OpcodeResultFile $CurLine;
}

close(OpcodeResultFile);
