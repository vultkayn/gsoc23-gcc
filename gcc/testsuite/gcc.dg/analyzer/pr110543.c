#include <memory>

struct A {int x; int y;};

int main () {
  std::shared_ptr<A> a; 
  a->x = 4; /* Diagnostic path should stop here rather than going to shared_ptr_base.h */
  return 0;
}