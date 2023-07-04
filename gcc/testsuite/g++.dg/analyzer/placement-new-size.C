/* { dg-additional-options "-Wno-placement-new -Wno-analyzer-use-of-uninitialized-value" } */

#include <new>
#include "../../gcc.dg/analyzer/analyzer-decls.h"

extern int get_buf_size ();

void var_too_short ()
{
  short s;
  long *lp = new (&s) long; /* { dg-warning "stack-based buffer overflow" } */
  /* { dg-warning "allocated buffer size is not a multiple of the pointee's size" "" { target *-*-* } .-1 } */
}

void static_buffer_too_short ()
{
  int n = 16;
  int buf[n];
  int *p = new (buf) int[n + 1]; /* { dg-warning "stack-based buffer overflow" } */
}

void symbolic_buffer_too_short ()
{
  int n = get_buf_size ();
  char buf[n];
  char *p = new (buf) char[n + 10]; /* { dg-warning "stack-based buffer overflow" } */
}
