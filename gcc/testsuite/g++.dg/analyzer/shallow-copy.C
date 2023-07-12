struct DoesntOwn
{
  DoesntOwn (int *p) : ptr{p} {}
  int *ptr = nullptr; // not owner of this
};

struct GNUOwner
{
  GNUOwner (int *p) : ptr {p} {}
  int *ptr [[gnu::owner]] = nullptr;
};

struct GNUOwnerCtor
{
  [[gnu::owner]]
  GNUOwnerCtor (int *init) : ptr(init) {}
  int *ptr = nullptr;
};

struct InferOwnerFromDtor
{
  InferOwnerFromDtor (int *init) : ptr(init){}
  ~InferOwnerFromDtor ()
  {
    delete ptr; /* { dg-bogus "release resource not owned" "-Wanalyzer-ownership" { target *-*-* } } */
  }
  int *ptr = nullptr;
};

struct InferOwnerFromNew
{
  InferOwnerFromNew (int x) : ptr {new int(x)} {}
  void release ()
  { 
    delete ptr; /* { dg-bogus "release resource not owned" "-Wanalyzer-ownership" { target *-*-* } } */
  }

  int *ptr = nullptr;
};

void test1 ()
{
  int *p = new int;
  DoesntOwn s (p);
  DoesntOwn ss = s; /* { dg-bogus "shallow copy" "-Wanalyzer-shallow-ownership-transfer" { target *-*-* } } */
  delete p;
}

void test2 ()
{
  GNUOwner g (new int);
  GNUOwner gg = g; /* { dg-warning "shallow copy" "-Wanalyzer-shallow-ownership-transfer" { xfail *-*-* } } */
}

void test3 ()
{
  GNUOwnerCtor g(new int);
  GNUOwnerCtor gg = g; /* { dg-warning "shallow copy" "-Wanalyzer-shallow-ownership-transfer" { xfail *-*-* } } */
}

/* Test that shallow copy is retroactive, if it's later discovered
   that we default copied a resource.  */

void test4 ()
{
  InferOwnerFromDtor g (new int); // ownership not known at this point.
  // nor at the copy
  InferOwnerFromDtor gg = g; /* { dg-warning "shallow copy" "-Wanalyzer-shallow-ownership-transfer" { xfail *-*-* } } */
  // however, deletion of g carries the information.
}