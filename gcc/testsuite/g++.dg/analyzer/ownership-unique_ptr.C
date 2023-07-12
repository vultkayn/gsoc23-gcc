#include <memory>

struct A
{
  int x, y;
};

void test ()
{
  A* a = new A;
  A* b = a;
  std::unique_ptr<A> unq (a);
  delete b; /* { dg-warning "release resource not owned" "-Wanalyzer-ownership" { xfail *-*-* } } */
}

void test2 ()
{
  A* a = new A;
  std::unique_ptr<A> unq (a);
  A* b = unq.release();
  delete b; /* { dg-bogus "release resource not owned" "-Wanalyzer-ownership" { target *-*-* } } */
}
