/* { dg-additional-options "-fdiagnostics-plain-output -fdiagnostics-path-format=inline-events -fanalyzer-show-events-in-system-headers" } */
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
    +--> 'std::__shared_ptr_access<_Tp, _Lp, <anonymous>, <anonymous> >::element_type* std::__shared_ptr_access<_Tp, _Lp, <anonymous>, <anonymous> >::operator->() const [with _Tp = A; __gnu_cxx::_Lock_policy _Lp = __gnu_cxx::_S_atomic; bool <anonymous> = false; bool <anonymous> = false]': events 3-4
           |
           |
           +--> 'std::__shared_ptr_access<_Tp, _Lp, <anonymous>, <anonymous> >::element_type* std::__shared_ptr_access<_Tp, _Lp, <anonymous>, <anonymous> >::_M_get() const [with _Tp = A; __gnu_cxx::_Lock_policy _Lp = __gnu_cxx::_S_atomic; bool <anonymous> = false; bool <anonymous> = false]': events 5-6
                  |
                  |
                  +--> 'std::__shared_ptr<_Tp, _Lp>::element_type* std::__shared_ptr<_Tp, _Lp>::get() const [with _Tp = A; __gnu_cxx::_Lock_policy _Lp = __gnu_cxx::_S_atomic]': events 7-8
                         |
                         |
                  <------+
                  |
                'std::__shared_ptr_access<_Tp, _Lp, <anonymous>, <anonymous> >::element_type* std::__shared_ptr_access<_Tp, _Lp, <anonymous>, <anonymous> >::_M_get() const [with _Tp = A; __gnu_cxx::_Lock_policy _Lp = __gnu_cxx::_S_atomic; bool <anonymous> = false; bool <anonymous> = false]': event 9
                  |
                  |
           <------+
           |
         'std::__shared_ptr_access<_Tp, _Lp, <anonymous>, <anonymous> >::element_type* std::__shared_ptr_access<_Tp, _Lp, <anonymous>, <anonymous> >::operator->() const [with _Tp = A; __gnu_cxx::_Lock_policy _Lp = __gnu_cxx::_S_atomic; bool <anonymous> = false; bool <anonymous> = false]': event 10
           |
           |
    <------+
    |
  'int main()': events 11-12
    |
    |
   { dg-end-multiline-output "" } */