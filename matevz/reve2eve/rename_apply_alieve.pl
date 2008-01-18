#!/usr/bin/perl

# use Data::Dumper;

#-------------------------------------------------------------------------------
# Configuration
#-------------------------------------------------------------------------------

$CFGFILE         = "rename-config";
$NEW_DIR_POSTFIX = "new";

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

die "Usage: $0 [-cfg=<config-file>] <files> <dirs>\n" .
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


for $f (@ARGV) {
  $f =~ s!/*$!!o;
  if (-d $f) {
    $origdir = "$f";
    $dir     = "$f-$NEW_DIR_POSTFIX";
    `rm -rf $dir`;
    `cp -a $origdir $dir`;
    push @infiles, split(' ', `find $dir -type f -name \\*.h -or -name \\*.cxx -or -name \\*.C`);    
  } else {
    push @infiles, $f;
  }
}


# Print files to modify. Pays off check this before the real run.
#
# print join "\n", @infiles, "\n";
# exit;


# Loop over .h, .cxx and .C files and do the replace magick.
for $infile (@infiles)
{
  my ($path, $stem, $ext) = $infile =~ m!^(.*/)?([^/]+)\.(\w+)$!;
  $path =~ s!^\./!!;

  my $outfile       = $infile;
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

  # Watch this !!!!!!!!!!!!!!  EDIT BELOW FOR NEW PROJECT !!!!!!!!!!!!!!!!
  # Multiple inclusion protection (h files)
  if ($ext eq "h" && $name_change)
  {
    $text =~ s/^\#ifndef (ALIEVE|REVE|ROOT)_\w+(_H)?/\#ifndef ALIEVE_${stem}_H/m;
    $text =~ s/^\#define (ALIEVE|REVE|ROOT)_\w+(_H)?/\#define ALIEVE_${stem}_H/m;
  }

  # Namespace holding class definition (h files)
  if ($STRIP_H_NAMESPACE and $ext eq "h")
  {
    if ($text =~ m/^\s*namespace $STRIP_H_NAMESPACE\s*\{/m)
    {
      $text =~ s/^\s*namespace $STRIP_H_NAMESPACE\s*\{//ms;
      $text =~ s!^[ \t]*\}[ \t]*(//.*)?\s*(?=\#endif)!!m;
    }
  }

  # Watch this !!!!!!!!!!!!!!  EDIT BELOW FOR NEW PROJECT !!!!!!!!!!!!!!!!
  # Remove Declarations like 'using namespace Reve'. (cxx and C files)
  if ($ext eq "cxx" or $ext eq "C")
  {
    $text =~ s/^\s*using namespace Alieve;$//mg;
  }

  # Watch this !!!!!!!!!!!!!!  EDIT BELOW FOR NEW PROJECT !!!!!!!!!!!!!!!!
  # Remove 'Reve::' and 'Reve/' stuff
  $text =~ s!Alieve::!!mg;
  # $text =~ s!Alieve/!!mg; # Not for aliroot

  # Replace include files whose names have changed
  for $file (keys %files)
  {
    my $repl = $files{$file};
    $text =~ s!^\#include (<|")${file}\.h("|>)!\#include $1$repl.h$2!mg;
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

  # Write file, possibly removing the old one.
  { 
    open  FILE, ">$outfile";
    print FILE $text;
    close FILE;
    unlink $infile if $name_change;
  }

  printf "%-30s -> %-30s (%d)\n", $infile, $outfile, $replace_count;

}
