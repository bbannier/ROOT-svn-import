#!/usr/bin/perl

@infiles = @ARGV;

$allstars   = "/"  . '*' x 78  . "/";
$alluscores = "//" . '_' x 78;

for $infile (@infiles)
{
  my ($path, $stem, $ext) = $infile =~ m!^(.*/)?([^/]+)\.(\w+)$!;
  $path =~ s!^\./!!;

  my $outfile     = $infile;

  # Read file into single scalar $text
  {
    local $/; undef $/;
    open FILE, $infile;
    $text = <FILE>;
    close FILE;
  }

  ### Text manipulation begin

  # Fix length of /****/ constructs to 80
  #$text =~ s!^/\*{60,77}/$!$allstars!ogm;

  # Fix length of //_____ to 80
  #$text =~ s!^//_{60,77}$!$alluscores!ogm;


  # Place /____ in front of funcs that don't have it

  # Does not work for ctors
  # $text =~ s/\n\n(^[^\s][^\(\n]+\([^\)]*\)\s*(?:const)?\n\{(?:\}|\n))/\n\n$alluscores\n$1/ogms;

  # Works somewhat for ctors, ClassImps get the line ... see next regexp
  # $text =~ s!\n\n(^[^\s/][^\(\n]+\(.*?\)\s*(?:const)?\n\{(?:\}|\n))!\n\n$alluscores\n$1!ogms;

  # Fix classimps that swallow/preempt ctor's //___________
  # $text =~ s!$alluscores\n(ClassImp\(\w+\))\n\n([^/])!$1\n\n$alluscores\n$2!ogms;

  ### Text manipulation end

  # Write file, possibly removing the old one.
  { 
    open  FILE, ">$outfile";
    print FILE $text;
    close FILE;
  }

  printf "%-30s -> %-30s\n", $infile, $outfile;

}
