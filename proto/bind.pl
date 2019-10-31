#!/usr/bin/perl
use strict;
my $prev = "NULL";
my %dict;
for (<>) {
	if (m{/\* : (.*?) => (.*?) \*/}) {
	print <<"EOT";
static struct primitive $2_def = {
	.link = {
		.prev = (struct link *)$prev,
		.name = "$1",
		.namelen = @{[length $1]},
	},
	.cfa = &&$2_code,
};
EOT
		$prev = "&$2_def";
		$dict{$1} = "&$2_def.xt";
	}
}
print "static struct primitive *latest=$prev;";
