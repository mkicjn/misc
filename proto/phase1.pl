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
	'/*:' => sub {
		$state=shift @line;
		$defs{$state}=["$ct{$state}_code"];
	},
	':' => sub {
		$state=shift @line;
		$defs{$state}=["$ct{'DOCOL'}_code"];
	},
	';' => sub {
		comma("&$ct{'EXIT'}_def.xt");
		undef $state;
	},
	';*/' => sub {
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
			$imm{$word}();
		} elsif ($state && $ct{$word}) {
			comma("&$ct{$word}_def.xt");
		}
	}
}

my @lines=(<>);
/: (\S+) \( (\S+) \)/ and $ct{$1}="$2" for @lines;
&interp for @lines;

my $fh;
open($fh,'>','primdecs.c') or die;
for (sort keys %defs) {
	print $fh "static struct primitive $ct{$_}_def;\n"
}
close $fh;

open($fh,'>','primdefs.c') or die;
my $last="NULL";
for (sort keys %defs) {
	print $fh <<"EOT";
$ct{$_}_def = (struct primitive){
	.link = {
		.prev = $last,
		.name = \"$_\",
		.namelen = @{[length]},
	},
	.cfa = &&@{[shift @{$defs{$_}}]},
	.xt = {@{[join ', ',@{$defs{$_}}]}},
};
EOT
	$last="&$ct{$_}_def.link";
}
close $fh;
