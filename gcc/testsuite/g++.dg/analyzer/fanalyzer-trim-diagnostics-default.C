/* { dg-additional-options "-O0 -fdiagnostics-plain-output -fdiagnostics-path-format=inline-events" } */
/* { dg-skip-if "" { c++98_only }  } */

// Must behave like -fanalyzer-trim-diagnostics=system

#include <memory>

struct A {int x; int y;};

int main () {
  std::shared_ptr<A> a; 
  a->x = 4; /* { dg-line deref_a } */ 
  /* { dg-warning "dereference of NULL" "" { target *-*-* } deref_a } */

  return 0;
}
/* { dg-begin-multiline-output "" }
  'int main()': events 1-2
    |
    |
   { dg-end-multiline-output "" } */