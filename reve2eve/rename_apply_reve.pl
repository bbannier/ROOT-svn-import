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
#$REPLACE_REVE_GLOBS = 0; # Activate for first runs, avoid in general.
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

if ($ARGV[0] =~ m!^-cfg=([-\w\./]+)$!)
{
  $CFGFILE = $1;
  shift @ARGV;
}

die "Usage: $0 [-cfg=<config-file>] <files>\n" .
    "  This version will NOT rename files, just replace includes.\n"
  if $#ARGV == -1;

# read config, remove no-change entries in files / classes

print "Using config-file '$CFGFILE'.\n";

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


@infiles = @ARGV;

# for $infile ("RenderElement.h", "RenderElement.cxx")
for $infile (@infiles)
{
  my ($path, $stem, $ext) = $infile =~ m!^(.*/)?([^/]+)\.(\w+)$!;
  $path =~ s!^\./!!;

  my $outfile       = $infile;
  my $replace_count = 0;

  # Read file into single scalar $text
  {
    local $/; undef $/;
    open FILE, $infile;
    $text = <FILE>;
    close FILE;
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
  # This is a remnant from transformation of Reve itself.
  # if ($ext eq "h")
  # {
  #   $text =~ s/^\#ifndef (REVE|ROOT)_\w+(_H)?/\#ifndef ROOT_$stem/m;
  #   $text =~ s/^\#define (REVE|ROOT)_\w+(_H)?/\#define ROOT_$stem/m;
  # }

  # Namespace holding class definition (h files)
  if ($STRIP_H_NAMESPACE and $ext eq "h")
  {
    if ($text =~ m/^\s*namespace $STRIP_H_NAMESPACE\s*\{/m)
    {
      $text =~ s/^\s*namespace $STRIP_H_NAMESPACE\s*\{//ms;
      $text =~ s!^[ \t]*\}[ \t]*(//.*)?\s*(?=\#endif)!!m;
    }
  }

  # Remove Declarations like 'using namespace Reve'. (cxx and C files)
  if ($ext eq "cxx" or $ext eq "C")
  {
    $text =~ s/^\s+using namespace Reve;\s*\n+//mg;
  }

  # Remove 'Reve::' and 'Reve/' stuff
  $text =~ s!Reve::!!mg;
  $text =~ s!Reve/!!mg;

  # Replace include files whose names have changed
  for $file (keys %files)
  {
    my $repl = $files{$file};
    $text =~ s!^\#include (<|")${file}\.h("|>)!\#include <$repl.h>!mg;
  }

  # Introduce early replacements
  for $word (keys %prereplace) {
    my $repl = $prereplace{$word};
    my $n = $text =~ s/(_|[^\w]|^)${word}(?=[^\w]|$)/$1$repl/mg;
    $replace_count += $n;
  }

  # Replace class names etc
  for $name (keys %classes)
  {
    my $repl = $classes{$name};
    my $n = $text =~ s/(_|[^\w]|^)${name}(?=[^\w]|$)/$1$repl/mg;
    $replace_count += $n;
  }

  # Functions in Reve namespace; Reve:: is removed before.
  # Macro is removed from the list as it is a common name
  if ($REPLACE_REVE_GLOBS) {
    for $name qw(SetupEnvironment CheckMacro AssertMacro LoadMacro ColorFromIdx FindColorVar)
    {
      my $repl = "TEveUtil::$name";
      my $n = $text =~ s/(_|[^\w]|^)${name}([^\w]|$)/$1$repl$2/mg;
      $replace_count += $n;
    }
  }

  # Introduce late replacements
  for $word (keys %postreplace) {
    my $repl = $postreplace{$word};
    my $n = $text =~ s/(_|[^\w]|^)${word}(?=[^\w]|$)/$1$repl/mg;
    $replace_count += $n;
  }

  ### Text manipulation end

  # Write file.
  { 
    open  FILE, ">$outfile";
    print FILE $text;
    close FILE;
  }

  printf "%-30s -> %-30s (%d)\n", $infile, $outfile, $replace_count;

}
