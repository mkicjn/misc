#!/usr/bin/perl
use strict;
my $last="NULL";
my %dict;
my @lines=(<>);
m{: ([^ ]*) \( ([^ ]*) \)} and $dict{$1}="&$2_def.xt" for @lines;
for (@lines) {
	if (m{/\*: ([^ ]*) \( ([^ ]*) \) ;\*/}) {
	print <<"EOT";
static struct primitive $2_def = {
	.link = {
		.prev = (struct link *)$last,
		.name = "$1",
		.namelen = @{[length $1]},
	},
	.cfa = &&$2_code,
};
EOT
		$last="&$2_def";
	}
}
print "static struct primitive *latest=$last;";
