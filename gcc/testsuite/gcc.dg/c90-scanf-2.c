/* Test for scanf formats.  Formats using C99 features should be rejected
   outside of C99 mode.
*/
/* Origin: Joseph Myers <jsm28@cam.ac.uk> */
/* { dg-do compile } */
/* { dg-options "-std=iso9899:1990 -pedantic -Wformat" } */

typedef __SIZE_TYPE__ size_t;
typedef __PTRDIFF_TYPE__ ptrdiff_t;

__extension__ typedef long long int llong;

/* This next definition is a kludge.  When GCC has a <stdint.h> it
   should be used.
*/
#include <limits.h>
#if INT_MAX == __LONG_LONG_MAX__
typedef int intmax_t;
#elif LONG_MAX == __LONG_LONG_MAX__
typedef long intmax_t;
#else
__extension__ typedef long long intmax_t;
#endif

extern int scanf (const char *, ...);

void
foo (signed char *hhp, float *fp, llong *llp, intmax_t *jp,
     size_t *zp, ptrdiff_t *tp)
{
  /* Some tests already in c90-scanf-1.c.  */
  /* The widths hh, ll, j, z, t are new.  */
  scanf ("%hhd", hhp); /* { dg-warning "length|C" "%hh in C90" } */
  scanf ("%lld", llp); /* { dg-warning "length|C" "%ll in C90" } */
  scanf ("%jd", jp); /* { dg-warning "length|C" "%j in C90" } */
  scanf ("%zu", zp); /* { dg-warning "length|C" "%z in C90" } */
  scanf ("%td", tp); /* { dg-warning "length|C" "%t in C90" } */
  /* The formats F, a, A are new.  */
  scanf ("%F", fp); /* { dg-warning "C" "%F in C90" } */
  scanf ("%a", fp); /* { dg-warning "C" "%a in C90" } */
  scanf ("%A", fp); /* { dg-warning "C" "%A in C90" } */
}
