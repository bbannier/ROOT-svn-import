#!/usr/bin/perl

$DUMPINFO = 1;

my ($BINDIR, $BINNAME) = $0 =~ m!^(.*)/(.*)!;

if ($#ARGV != 1) {
  print STDERR <<"fnord";
usage: $BINNAME <module> <class-base>
   eg: $BINNAME base     MyNewClass  # create normal class
       $BINNAME gl       MyNewClass  # create GL renderer
       $BINNAME ged      MyNewClass  # create Editor
       $BINNAME gedsubed MyNewClass  # create Editor with SubEditor
Should be in module (eve/ or gl/).
Note that GL and Editor suffixes are not present in the command!
fnord
  exit 1;
}

my $MODULE = $ARGV[0];
# Flat structure now.
# die "'$MODULE' not a directory" unless -d $MODULE;

%suff = ( 'base' => '', 'gl' => 'GL', 'ged' => 'Editor', 'gedsubed' => 'Editor');

my $STEM  = $ARGV[1];
my $CLASS = $STEM . $suff{$MODULE};

if ($MODULE eq 'gedsubed') {
  $replace_xxclass = 1;
  $XXCLASS = $STEM . 'SubEditor';
}

# Flat structure now.
# my $H_NAME = "$MODULE/$CLASS.h";
# my $C_NAME = "$MODULE/$CLASS.cxx";
my $H_NAME = "$CLASS.h";
my $C_NAME = "$CLASS.cxx";

my $H_FILE = "inc/$H_NAME";
my $C_FILE = "src/$C_NAME";

die "Expect inc/ and src/ directories in working dir"
  unless -d "inc" and -d "src";
die "File '$H_FILE' already exists" if -e $H_FILE;
die "File '$C_FILE' already exists" if -e $C_FILE;

sub find_skel
{
  my ($stem, $dirs) = @_;

  for $d (@$dirs) {
    my $file = "$d/$stem";
    return $file if -e $file;
  }
  die "Skeleton file '$stem' not found in following dirs:\n  '" .
    join("', '", @$dirs) . "'\n";
}

my $SKEL_H_NAME = find_skel("SKEL-$MODULE.h",   [".", "inc", "$BINDIR"]);
my $SKEL_C_NAME = find_skel("SKEL-$MODULE.cxx", [".", "src", "$BINDIR"]);

print "Using skeleton files '$SKEL_H_NAME' and '$SKEL_C_NAME'\n" if ($DUMPINFO);

my ($skel_h, $skel_c);
{
  my $ex_sla = $/; undef $/;
  open H, "$SKEL_H_NAME" or die "can't open $SKEL_H_NAME";
  $skel_h = <H>; close H;
  open C, "$SKEL_C_NAME" or die "can't open $SKEL_C_NAME";
  $skel_c = <C>; close C;
  $/ = $ex_sla;
}

print "Replacing CLASS -> $CLASS, STEM -> $STEM.\n" if ($DUMPINFO);

for $f ($skel_h, $skel_c) {
  $f =~ s/XXCLASS/$XXCLASS/g if $replace_xxclass;
  $f =~ s/CLASS/$CLASS/g;
  $f =~ s/STEM/$STEM/g;
}

print "Writing files '$H_FILE', '$C_FILE'.\n" if ($DUMPINFO);

open H, ">$H_FILE" or die "can't open $H_FILE";
print H $skel_h;
close H;

open H, ">$C_FILE" or die "can't open $C_FILE";
print H $skel_c;
close H;

print "Now you should also edit the link-def file:\n" .
  "pragma link C++ class $CLASS+\n;" if ($DUMPINFO);
