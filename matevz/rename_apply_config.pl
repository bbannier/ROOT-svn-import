#!/usr/bin/perl

# use Data::Dumper;

#-------------------------------------------------------------------------------
# Configuration
#-------------------------------------------------------------------------------

$CFGFILE="rename-config";

# Several config vars can be set in config file, e.g.:
#
#$RENAME_FILES = 1;
#
#$FIX_FRONT_MATTER = 1; # edit below, too
#$FRONT_MATTER=<<'FNORD';
#// @(#)root/reve:$Id$
#// Authors: Matevz Tadel & Alja Mrak-Tadel: 2006, 2007
#
#/*************************************************************************
# * Copyright (C) 1995-2007, Rene Brun and Fons Rademakers.               *
# * All rights reserved.                                                  *
# *                                                                       *
# * For the licensing terms see $ROOTSYS/LICENSE.                         *
# * For the list of contributors see $ROOTSYS/README/CREDITS.             *
# *************************************************************************/
#FNORD


#-------------------------------------------------------------------------------
# Action starts here
#-------------------------------------------------------------------------------

die "Usage: $0 <original-dir> <modified-copy-dir>\n"
     unless -d $ARGV[0] and -d $ARGV[1];

$origdir = $ARGV[0];
$dir     = $ARGV[1];

`rm -rf $dir`;
`cp -a $origdir $dir`;

chdir $dir;


# read config, remove no-change entries in files / classes

do "$CFGFILE" or die "Can't read config file $CFGFILE; run rename_dump_config_skeleton.pl and edit its output first.";

$n = $N = 0;
for $f (keys %files)
{
  if ($f eq $files{$f}) {
    ++$n;
    delete $files{$f};
  }
  ++$N;
}
print "files:   removed $n noop entries, $N all entries.\n";

$n = $N = 0;
for $f (keys %classes)
{
  if ($f eq $classes{$f}) {
    ++$n;
    delete $classes{$f};
  }
  ++$N;
}
print "classes: removed $n noop entries, $N all entries.\n";


# Loop over .h, .cxx and .C files and do the replace magick.

@infiles = split(' ', `find . -type f -name \\*.h -or -name \\*.cxx -or -name \\*.C`);

# for $infile ("RenderElement.h", "RenderElement.cxx")
for $infile (@infiles)
{
  my ($path, $stem, $ext) = $infile =~ m!^(.*/)?([^/]+)\.(\w+)$!;
  $path =~ s!^\./!!;

  my $outfile;
  my $name_change;
  if ($RENAME_FILES && exists $files{$stem}) {
    $outfile     = "$path$files{$stem}.$ext";
    $name_change = 1;
  } else {
    $outfile     = $infile;
    $name_change = 0;
  }

  my $replace_count = 0;

  # Read file into single scalar $text
  {
    local $/; undef $/;
    open FILE, $infile;
    $text = <FILE>;
    close FILE;
    unlink $infile;
  }

  ### Text manipulation begin

  # Front stuff - SVN Id, Author, Copyright
  if ($FIX_FRONT_MATTER)
  {
    if ($text =~ m!^//.*\$(Header|Id)!) {
      $text =~ s/^.*\n//m;
    }
    $text = $FRONT_MATTER . $text;
  }

  # Multiple inclusion protection (h files)
  if ($ext eq "h" && $name_change)
  {
    $text =~ s/^\#ifndef (REVE|ROOT)_\w+(_H)?/\#ifndef ROOT_$stem/m;
    $text =~ s/^\#define (REVE|ROOT)_\w+(_H)?/\#define ROOT_$stem/m;
  }

  # Namespace holding class definition (h files)
  if ($ext eq "h")
  {
    if ($text =~ m/^\s*namespace Reve\s*\{/m)
    {
      $text =~ s/^\s*namespace Reve\s*\{//ms;
      $text =~ s!^[ \t]*\}[ \t]*(//.*)?\s*(?=\#endif)!!m;
    }
  }

  # Remove Declarations like 'using namespace Reve'. (cxx and C files)
  if ($ext eq "cxx" or $ext eq "C")
  {
    $text =~ s/^\s+using namespace Reve;\s*\n+//mg;
  }

  # Remove 'Reve::' and 'Reve/' stuff
  $text =~ s/Reve:://mg;
  $text =~ s!Reve/!!mg;

  # Replace include files whose names have changed
  for $file (keys %files)
  {
    my $repl = $files{$file};
    $text =~ s!^\#include (<|")${file}\.h("|>)!\#include <$repl.h>!mg;
  }

  # Replace class names etc
  for $name (keys %classes)
  {
    my $repl = $classes{$name};
    ### my $n = $text =~ s/(_|[^\w]|^)${name}([^\w]|$)/$1$repl$2/mg;
    my $n = $text =~ s/(_|[^\w]|^)${name}(?=[^\w]|$)/$1$repl/mg;
    $replace_count += $n;
  }

  # Functions in Reve namespace
  #if ($stem ne "Reve" and $stem ne "LinkDef")
  #{
  #  for $name qw(SetupEnvironment CheckMacro AssertMacro Macro LoadMacro ColorFromIdx FindColorVar)
  #  {
  #    my $repl = "TEve::$name";
  #    my $n = $text =~ s/(_|[^\w]|^)${name}([^\w]|$)/$1$repl$2/mg;
  #  }
  #}

  ### Text manipulation end

  # Write file, possibly removing the old one.
  { 
    open  FILE, ">$outfile";
    print FILE $text;
    close FILE;
    unlink $infile if $name_change;
  }

  printf "%-30s -> %-30s (%d)\n", $infile, $outfile, $replace_count;

}
