/* Derived from example code at https://en.cppreference.com/w/cpp/language/dynamic_cast.  */

struct V
{
  virtual void f() {} // must be polymorphic to use runtime-checked dynamic_cast
};

struct A : virtual V
{
};

struct B : virtual V
{
  int b;
  B(V *v, A *a)
  {
    // casts during construction (see the call in the constructor of D below)
    // well-defined: v of type V*, V base of B, results in B*
    dynamic_cast<B *>(v);
    // undefined behavior: a has type A*, A not a base of B
    dynamic_cast<B *>(a); /* { dg-warning "-Wanalyzer-undefined-cast" "" { xfail *-*-* } } */

    // undefined behavior: target-type is A*, A not a base of B
    dynamic_cast<A *>(this); /* { dg-warning "-Wanalyzer-undefined-cast" "" { xfail *-*-* } } */
  }
};

struct D : A, B
{
  int d;
  D() : B(static_cast<A *>(this), this) {}
};

struct E : A
{
};

/*
Class hierarchy is as such:
          V
        /   \
       A     B
       |\   /
       | \ /
       E  D

  Syntax: dynamic_cast< target-type >( expression )
*/

void test1 ()
{
  D d;
  A &a = d;

  D &new_d = dynamic_cast<D &>(a); // downcast
  /* { dg-bogus "dereference of NULL" "" { target *-*-* } .-1 }  */
  B &new_b = dynamic_cast<B &>(a); // sidecast
  /* { dg-bogus "dereference of NULL" "" { target *-*-* } .-1 }  */
}

void test2 ()
{
  E e;
  A *a = &e;

  /* Invalid cast.
   D cannot be derived from the most derived object which is of type E, so downcast impossible.
   D is not a base of E, so a sidecast is impossible.  */
  D *new_d = dynamic_cast<D *>(a);
  new_d->d = 7; /* { dg-warning "dereference of NULL" "" { xfail *-*-* } }  */
}

void test3 ()
{
  E e;
  A *a = &e;

  /* Invalid cast.
   B is not a base of E, so a sidecast is impossible.  */
  B *new_b = dynamic_cast<B *>(a);
  new_b->b = 7; /* { dg-warning "dereference of NULL" "" { xfail *-*-* } }  */
}
