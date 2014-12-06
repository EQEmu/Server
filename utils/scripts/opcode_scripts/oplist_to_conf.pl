#!/usr/bin/perl

# File Name: oplist_to_conf.pl
# Converts the Opcode List with Opcode Names from the Spreadsheet into a Patch File.

# Directions to use this script:
# 1. Copy the opcodes and opcode names columns from the opcode spreadsheet
#        for the columns you want to create a new .conf file from into the file.
# 2. Remove the header row entries in the text file.
# 3. Find/Replace all Tabs "	" with 2 spaces "  " and save the file.
# 4. Paste the contents of the current patch file in the patch_OLD.conf file.
# 5. Run this script using "perl oplist_to_conf.pl"
# 6. This creates a new .conf file named patch_NEW.conf
# 7. Rename patch_NEW.conf to the desired name and you are all done


$stopmessage = "Failed to open file";
# Open the text file that contains the opcode and opcode name rows for a single client from the opcode list spreadsheet
open OpcodeFile, "<", "opcodelist.txt" or die $stopmessage;
# Open the .conf file to copy from
open PatchFile, "<", "patch_OLD.conf" or die $stopmessage;

# Read these files into arrays for looping and close the files
my @OpcodeList = <OpcodeFile>;
my @PatchFile = <PatchFile>;
close(OpcodeFile);
close(PatchFile);

# Open the new/destination .conf file and clear out file contents
open OpcodeResultFile, ">", "patch_NEW.conf" or die $stopmessage;
print OpcodeResultFile "";
# Close out the newly cleared .conf file
close(OpcodeResultFile);

# Open the new/destination .conf file again for appending
open OpcodeResultFile, ">>", "patch_NEW.conf" or die $stopmessage;

my %PatchHash = ();

foreach $line (@OpcodeList)
{
        @equalssplit = split(/  /, $line);
        $ArraySize = @equalssplit;
        if ($ArraySize > 1)
        {
                my $CurOpcode = "";
                my $CurOpcodeName = "";
                @OpcodeArray = split(//, $equalssplit[0]);
                if ($equalssplit[1] =~ /^OP_(.*)/i)
                {
                        $CurOpcodeName = "OP_".$1;
                }
                foreach $Letter (@OpcodeArray)
                {
                        if ($Letter =~ /[A-Za-z0-9]/)
                        {
                                $CurOpcode .= $Letter;
                        }
                }
                if ($CurOpcode && $CurOpcodeName)
                {
                        $CurOpcode = lc($CurOpcode);
                        $PatchHash{ $CurOpcodeName } = $CurOpcode;
                        #print $CurOpcodeName."=". $CurOpcode."\n";
                }
        }
}

$TabSpace = "   ";      # Tab
foreach $line (@PatchFile)
{
        $CurLine = $line;
        if ($line =~ /^OP_([^\=]+)=([^\s]+)(.*)/i)
        {
                $NewOpcode = "0x0000";
                $OpcodeName = "OP_".$1;

                if ($PatchHash{$OpcodeName})
                {
                        $NewOpcode = $PatchHash{$OpcodeName};
                }

                $CurLine = $OpcodeName."=".$NewOpcode.$3."\n";
        }
        print OpcodeResultFile $CurLine;
}


close(OpcodeResultFile);
