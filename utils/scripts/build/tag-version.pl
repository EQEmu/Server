#!/usr/bin/perl
use strict;
use warnings FATAL => 'all';

open my $fh, '<', './package.json' or die "Can't open file $!";
my $package_json = do {
    local $/;
    <$fh>
};

my $version = "";
sub trim($)
{
    my $string = shift;
    $string =~ s/^\s+//;
    $string =~ s/\s+$//;
    return $string;
}

# manually parse because we can't guarantee json module is available
# it's jank but it's quick and dirty
# this is only used in the build pipeline
foreach my $line (split("\n", $package_json)) {
    if ($line =~ /version/i) {
        $version = $line;
        $version =~ s/version//g;
        $version =~ s/://g;
        $version =~ s/"//g;
        $version =~ s/,//g;
        $version = trim($version);
    }
}

print "Version is [" . $version . "]\n";

# server version file
my $version_file_name = "./common/version.h";
open my $vfh, '<', $version_file_name or die "Can't open file $!";
my $version_file = do {
    local $/;
    <$vfh>
};

# write new version
my $new_version_file = "";
foreach my $line (split("\n", $version_file)) {
    if ($line =~ /CURRENT_VERSION/i) {
        my @s = split("\"", $line);
        if ($#s == 2) {
            $line =~ s/$s[1]/$version/g;
        }
    }

    $new_version_file .= $line . "\n";
}

open(my $wfh, '>', $version_file_name) or die "Could not open file '$version_file_name' $!";
print $wfh $new_version_file;
close $wfh;

print $new_version_file;
