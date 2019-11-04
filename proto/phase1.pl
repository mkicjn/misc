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
open($fh,'>','cfas.c') or die;
print $fh "static void **cfas[] = {\n";
for (sort keys %defs) {
	print $fh "@{[shift @{$defs{$_}}]},\n"
}
print $fh "};";
close $fh;

open($fh,'>','dict.c') or die;
my $last="NULL";
for (reverse sort keys %defs) {
	print $fh <<"EOT";
static struct primitive $ct{$_}_def = {
	.link = {
		.prev = $last,
		.name = \"$_\",
		.namelen = @{[length]},
	},
	.cfa = NULL;
	.xt = {@{[join ', ',@{$defs{$_}}]}},
};
EOT
	$last="&$ct{$_}_def.link";
}
print $fh "latest = (struct primitive *)$last;\n";
close $fh;
