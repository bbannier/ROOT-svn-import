#!/usr/bin/perl -w

while ( my $arg = shift @ARGV ) {

  my $FHIN = *STDIN;
  open($FHIN, "<", $arg) or die "Couldn't open file $arg for input\n";

  print "Methods without comment in file \033[1;32m$arg\033[0m:\n";

  while (<$FHIN>) {
    if( not /TMVA::[A-Za-z]*::/ ) { next; }
    if( /;/ ) { next; }
    my $method = $_;
    while (<$FHIN>) {
      if( /;/ and not /^\/\// ) { last; }
      if( not /{/ ) { next; }
      $_ = <$FHIN>;
      if( not /^\ *\/\//) { print "  $method"; }
      last;
    }
  }

}
