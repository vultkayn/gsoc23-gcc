/* { dg-additional-options "-fdiagnostics-plain-output -fdiagnostics-path-format=inline-events -fno-analyzer-show-events-in-system-headers" } */
/* { dg-skip-if "" { c++98_only }  } */

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