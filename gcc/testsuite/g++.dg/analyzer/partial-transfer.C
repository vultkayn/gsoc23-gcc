/* When a user-defined move/copy constructor was provided,
   check that every resource owned are effectively
   moved/copied, not just a subset of them.  */

struct A
{
  // Ownership through New
  A (int x, int y) : ptr1 (new int (x)), ptr2 (new int (y)) {}

  A (A const &other)
  {
    delete ptr1;
    ptr1 = new int(*other.ptr1);
    // shallow here
    ptr2 = other.ptr2;
  } /* { dg-warning "partial copy of instance of 'A' missing true copy of 'ptr2'" "-Wanalyzer-shallow-ownership-transfer"  { xfail *-*-* } }  */

  A &operator= (A const &other)
  {
    delete ptr1;
    ptr1 = new int (*other.ptr1);
    // shallow here
    ptr2 = other.ptr2;
    return *this;
  } /* { dg-warning "partial copy of instance of 'A' missing true copy of 'ptr2'" "-Wanalyzer-shallow-ownership-transfer"  { xfail *-*-* } }  */

  A (A &&other)
  {
    delete ptr1;
    ptr1 = std::move (*other.ptr1);
    // no mention of ptr2
  } /* { dg-warning "partial move of instance of 'A' missing true move of 'ptr2'" "-Wanalyzer-shallow-ownership-transfer"  { xfail *-*-* } }  */

  A& operator= (A && other)
  {
    delete ptr1;
    ptr1 = *other.ptr1; // in this context, it is a valid move.
    // no mention of ptr2
  } /* { dg-warning "partial move of instance of 'A' missing true move of 'ptr2'" "-Wanalyzer-shallow-ownership-transfer"  { xfail *-*-* } }  */

  int *ptr1;
  int *ptr2;
}