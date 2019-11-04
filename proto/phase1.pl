#!/usr/bin/perl
use strict;
use warnings;

my @stack=();
my $state;
my @line=(); # String array
my %ct=();
my %defs=(); # Hash of string array refs

sub comma ($) {
	my ($v)=@_;
	push @{$defs{$state}},$v;
}

my %imm = (
	'C:' => sub {
		$state=shift @line;
		$defs{$state}=["$ct{$state}_code"];
		@line=();
	},
	':' => sub {
		$state=shift @line;
		$defs{$state}=["$ct{'DOCOL'}_code"];
	},
	';' => sub {
		comma("&$ct{'EXIT'}_def.xt");
		undef $state;
	},
	'(' => sub {
		while (@line && shift @line ne ')') {}
	},
	')' => sub {},
	#TODO Other immediates
);

sub interp ($) {
	chomp;
	@line=split ' ',$_;
	while (@line) {
		my $word=shift @line;
		#print "$word\n"; next;
		if ($imm{$word}) {
			print "Executing Perl definition of '$word'\n";
			$imm{$word}();
		} elsif ($ct{$word}) {
			print "Compiling word '$word' using C token '$ct{$word}'\n";
			comma("&$ct{$word}_def.xt");
		} else {
			die "$word?\n"
		}
	}
}

$ct{'DOCOL'}="docol"; #TODO: Remove
$ct{'EXIT'}="exit"; #TODO: Remove

my @lines=(<>);
/: (\S+) \( (\S+) \)/ and $ct{$1}="$2" for @lines;
&interp for @lines;

print "Done\n";
for (keys %defs) {
	printf("Definition of $_: ");
	for (@{$defs{$_}}) {
		print $_,' ';
	}
	print "\n";
}
