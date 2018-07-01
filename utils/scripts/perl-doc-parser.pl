use File::Find;
use Data::Dumper;

my @files;
my $start_dir = "zone/";
find(
    sub { push @files, $File::Find::name unless -d; },
    $start_dir
);
for my $file (@files) {

	#::: Skip non Perl files
	if($file!~/perl_/i){ 
		next; 
	}

	@methods = ();

	#::: Open File
	print "Opening '" . $file . "'\n";
	open (FILE, $file);
	while (<FILE>) {
		chomp;
		$line = $_;

		if ($line=~/Client::/i) {

			$split_key = "Client::";
			$object_prefix = "\$client->";

			#::: Split on croak usage
			@data  = split($split_key, $line);
			$usage = trim($data[1]);

			#::: Split out param borders and get method name
			@params_begin = split('\(', $usage);
			$method_name  = trim($params_begin[0]);

			#::: Get params string built
			@params_end    = split('\)', $params_begin[1]);
			$params_string = trim($params_end[0]);
			$params_string =~s/THIS\,//g;
			$params_string =~s/THIS//g;
			$params_string = trim($params_string);

			$method = $object_prefix . $method_name . "(" . lc($params_string) . ")\n";

			push @methods, $method;
		}
	}

	@methods = sort @methods;
	foreach $method (@methods) {
		print $method;
	}
}

#::: Trim Whitespaces
sub trim { 
	my $string = $_[0]; 
	$string =~ s/^\s+//; 
	$string =~ s/\s+$//; 
	return $string; 
}