#!/usr/bin/perl
use strict;
my @sp;
my %dict = (
	'+' => sub {
		my $a = pop @sp;
		my $b = pop @sp;
		push @sp, $a + $b;
	},
	'.' => sub {
		print(pop @sp, " ")
	},
);

INTERP:
while (<>) {
	for (split /\s/) {
		push @sp, int $_ and next if /^-?\d+$/;
		eval {
			$dict{uc $_}();
		} or do {
			print uc $_, "?\n";
			@sp = ();
			next INTERP;
		}
	}
	print "ok\n";
}
