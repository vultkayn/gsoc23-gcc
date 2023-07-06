/* Derived from example code at https://en.cppreference.com/w/cpp/language/dynamic_cast.  */
 
struct Base
{
  virtual ~Base() {}
};

struct Derived : Base
{
  virtual void name () {}
};

struct DerivedTwice : Derived
{
  virtual void name () {}
};

void test1 ()
{
  Base *b = new Base;
  Derived *d = dynamic_cast<Derived *>(b);
  d->name(); /* { dg-warning "dereference of NULL" "" { xfail *-*-* } }  */
  delete b;
}

void test2()
{
  Base *b = new Derived;
  Derived *d2 = dynamic_cast<Derived *>(b);
  d2->name(); // safe to call
  /* { dg-bogus "dereference of NULL" "" { target *-*-* } .-1 }  */
  delete b;
}

void test_null ()
{
  Base *b = nullptr;
  dynamic_cast<Derived *>(b)->name();
  /* { dg-warning "dereference of NULL" "" { xfail *-*-* } .-1 }  */
}
