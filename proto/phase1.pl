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
		comma("&$ct{'EXIT'}_def.cfa");
		undef $state;
	},
	';*/' => sub {
		undef $state;
	},
	'(' => sub {
		while (@line && shift @line ne ')') {}
	},
	')' => sub {
	},
	'POSTPONE' => sub {
		comma("&$ct{shift @line}_def.cfa");
	},
	'[\']' => sub {
		comma("&$ct{'DOLIT'}_def.cfa");
		comma("&$ct{shift @line}_def.cfa");
	},
	#TODO Other immediates
);

sub interp ($) {
	chomp;
	@line=split ' ',$_;
	while (@line) {
		my $word=shift @line;
		if ($imm{$word}) {
			$imm{$word}();
		} elsif ($word=~/^-?\d+$/) {
			comma("&$ct{'DOLIT'}_def.cfa");
			comma("(void **)$word");
		} elsif ($state && $ct{$word}) {
			comma("&$ct{$word}_def.cfa");
		}
	}
}

my @lines=(<>);
/: (\S+) \( (\S+) \)/ and $ct{$1}="$2" for @lines;
&interp for @lines;

my $fh;
open($fh,'>','cfas.c') or die;
print $fh "static void *cfas[] = {\n";
for (sort keys %defs) {
	print $fh "\t&&@{$defs{$_}}[0],\n"
}
print $fh "};";
close $fh;

open($fh,'>','dict.c') or die;
my $last;
print $fh "static struct primitive $ct{$_}_def;\n" for keys %defs;
print $fh "\n";
for (reverse sort keys %defs) {
	print $fh <<"EOT";
static struct primitive $ct{$_}_def = {
	.link = {
		.prev = @{[$last?"&${last}.link":"NULL"]},
		.name = \"$_\",
		.namelen = @{[length]},
	},
	// .cfa = &&@{[shift @{$defs{$_}}]},
	.data = {@{[join ', ',@{$defs{$_}}]}},
};
EOT
	$last="$ct{$_}_def";
}
print $fh "static struct primitive *latest = &$last;\n";
close $fh;
