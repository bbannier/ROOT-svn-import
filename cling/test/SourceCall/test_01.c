// RUN: %cling %s

#include <stdio.h>

int test_01(int argc=12, char** argv= "A default argument")
{
  int i;
  for( i = 0; i < 5; ++i )
    printf( "Hello World #%d\n", i );
  return 0;
}
