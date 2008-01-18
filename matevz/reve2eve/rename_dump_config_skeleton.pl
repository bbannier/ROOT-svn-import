#!/usr/bin/perl

die "No LinkDef.h" unless -e "LinkDef.h";

################################################################################

%files = ();

opendir D, ".";

while ($_ = readdir D) {

  next if /^\./ or not -f "$_";

  print $_."\n";

  my $file = $_;
  my $stem = $file;
  $stem =~ s/\..+//;

  $files{$stem} = "AliEve${stem}";
}

closedir D;

################################################################################

%classes = ();

open F, "cpp LinkDef.h|";

while ($_ = <F>) {

  next if /^\s+$/;

  if ( /\#pragma link C\+\+ class ([\w_:]+)[+!]*;/ ) {
    my $fqn = $1;
    my $cls = $fqn;
    $cls =~ s/.*:://;

    $classes{$cls} = "AliEve${cls}";
  }

}

close F;

################################################################################

use Data::Dumper;

$outfile = "rename-config";

$Data::Dumper::Indent = 1;
$Data::Dumper::Purity = 1;

open  O, ">$outfile" or die "can't open $outfile";
print O Data::Dumper->Dump([\%files, \%classes], [qw/*files *classes/]);
close O;
