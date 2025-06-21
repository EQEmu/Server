#!/usr/bin/perl

use strict;
use warnings FATAL => 'all';

my $filename = './common/eqemu_logsys.h';
open(my $fh, '<:encoding(UTF-8)', $filename)
    or die "Could not open file '$filename' $!";

my $contents   = "";
while (my $row = <$fh>) {
    chomp $row;
    $contents .= $row . "\n";
}

my @enum = split('enum LogCategory \{', $contents);

if (scalar(@enum) > 0) {
    # print $enum[1];
    my @second_split = split('};', $enum[1]);
    if (scalar(@second_split) > 0) {
        my $categories = $second_split[0];
        $categories =~ s/^\s+//;
        $categories =~ s/\s+$//;
        $categories =~ s/ //g;
        $categories =~ s/	//g;
        $categories =~ s/\n//g;
        $categories =~ s/None=0,//g;
        $categories =~ s/,MaxCategoryID//g;
        $categories =~ s/\/\*//g;
        $categories =~ s/\*\///g;
        $categories =~ s/Don'tRemovethis//g;

        my @cats = split(',', $categories);

        foreach my $cat (@cats) {
            print "#define Log" . $cat . "(message, ...) do {\\
    if (EQEmuLogSys::Instance()->IsLogEnabled(Logs::General, Logs::" . $cat . "))\\
        OutF(LogSys, Logs::General, Logs::" . $cat . ", __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\\
} while (0)

#define Log" . $cat . "Detail(message, ...) do {\\
    if (EQEmuLogSys::Instance()->IsLogEnabled(Logs::Detail, Logs::" . $cat . "))\\
        OutF(LogSys, Logs::Detail, Logs::" . $cat . ", __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\\
} while (0)

";
            # print "$cat\n";
        }
    }
}

close $fh;
