struct DoesntOwn
{
  DoesntOwn (int *p) : ptr{p} {}
  int *ptr = nullptr; // not owner of this
};

[[gnu::owner]]
void deallocator (int *ptr);

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


/*
* If an owner is reassigned, it automatically loses ownership of the previous resource.
* and gain ownership of the newly assigned one.
* Not the purpose of this sm to flag dangling resources.
*/

/* If a move constructor is provided by the user, flag that all owned resources
   are passed over to lhs. Otherwise, emit a "partial-move" warning.  */

void test1 ()
{
  int *p = new int;
  deallocator (p);
  delete p; /* { dg-warning "release resource not owned" "-Wanalyzer-ownership" { xfail *-*-* } } */
}

void test2 ()
{
  int *p = new int;
  DoesntOwn d (p);
  delete p; /* { dg-bogus "release resource not owned" "-Wanalyzer-ownership" { target *-*-* } } */
}

void test3 ()
{
  int *p = new int;
  GNUOwner g (p);
  delete p; /* { dg-warning "release resource not owned" "-Wanalyzer-ownership" { xfail *-*-* } } */
}

void test4 ()
{
  int *p = new int;
  GNUOwnerCtor g (p);
  delete p; /* { dg-warning "release resource not owned" "-Wanalyzer-ownership" { xfail *-*-* } } */
}

void test5 ()
{
  InferOwnerFromNew g (5);
  g.release ();
}