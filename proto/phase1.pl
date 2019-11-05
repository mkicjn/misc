#!/usr/bin/perl
use strict;
use warnings;

my @stack=();
my $state;
my @line=();
my %ct=();
my %cfa=();
my %data=(); # Hash of string array refs

sub comma ($) {
	my ($cell)=@_;
	push @{$data{$state}},$cell;
}

my %imm = (
	'/*:' => sub {
		$state=shift @line;
		$cfa{$state}="&&$ct{$state}_code";
		$data{$state}=[];
	},
	':' => sub {
		$state=shift @line;
		$cfa{$state}="&&$ct{'DOCOL'}_code";
		$data{$state}=[];
	},
	';' => sub {
		comma("&$ct{'EXIT'}_def.cfa");
		undef $state;
	},
	';*/' => sub {
		undef $state;
	},
	'(' => sub {
		do {} while @line and shift @line ne ')';
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
/: (\S+) \( (\S+) \)/ and $ct{$1}=$2 for @lines;
&interp for @lines;

my $fh;
open($fh,'>','cfas.c') or die;
print $fh "static void *cfas[] = {\n";
for (sort keys %ct) {
	print $fh "\t$cfa{$_},\n"
}
print $fh "};";
close $fh;

open($fh,'>','dict.c') or die;
my $last;
print $fh "static struct primitive $ct{$_}_def;\n" for keys %ct;
print $fh "\n";
for (reverse sort keys %ct) {
	print $fh <<"EOT";
static struct primitive $ct{$_}_def = {
	.link = {
		.prev = @{[$last?"&${last}_def.link":"NULL"]},
		.name = "$_",
		.namelen = @{[length]},
	},
	// .cfa = $cfa{$_},
	.data = {@{[join ', ',@{$data{$_}}]}},
};
EOT
	$last=$ct{$_};
}
print $fh "static struct primitive *latest = &${last}_def;\n";
close $fh;
