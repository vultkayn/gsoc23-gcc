/* { dg-additional-options "-fdiagnostics-plain-output -fdiagnostics-path-format=inline-events" } */
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
    +--> 'std::__shared_ptr_access<_Tp, _Lp, <anonymous>, <anonymous> >::element_type* std::__shared_ptr_access<_Tp, _Lp, <anonymous>, <anonymous> >::operator->() const [with _Tp = A; __gnu_cxx::_Lock_policy _Lp = __gnu_cxx::_S_atomic; bool <anonymous> = false; bool <anonymous> = false]': event 3
           |
           |
    <------+
    |
  'int main()': events 4-5
    |
    |
   { dg-end-multiline-output "" } */