// from PR107534

#include <utility>

struct S {
  S() { p = new int(); }
  ~S() { delete p; }

  int* p = nullptr;
};

void test_move ()
{
  S s;
  S ss = std::move (s);
  int* p = s.p; /* { dg-warning "use-after-move" "-Wanalyzer-use-after-move" { xfail *-*-* } }  */
}