struct A
{
  int a;
  int b;
};

struct B
{
  int c;
  int d;
  ~B() {}
};

void test ()
{
  A *a = new A ();
  delete a;
  delete a; /* { dg-line delete_a } */
  /* { dg-bogus "-Wanalyzer-use-after-free" "" { xfail *-*-* } delete_a } */
  /* { dg-warning "-Wanalyzer-double-delete" "" { xfail *-*-* } delete_a } */
}

void test_destructor ()
{
  B* b = new B ();
  delete b;
  delete b; /* { dg-line delete_b } */
  /* { dg-bogus "-Wanalyzer-use-after-free" "" { target *-*-* } delete_b } */
  /* { dg-warning "-Wanalyzer-double-delete" "" { target *-*-* } delete_b } */
}