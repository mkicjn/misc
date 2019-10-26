#!/usr/bin/perl
use strict;
my @stack;
my %dict = (
	'+' => sub {
		my $a = pop @stack;
		my $b = pop @stack;
		push @stack, ($a + $b);
	},
	'.' => sub {
		print(pop @stack, " ")
	},
);

while (<>) {
	for (split /\s/) {
		push @stack, $_ and next if /^-?\d+$/;
		$dict{$_}();
	}
}
