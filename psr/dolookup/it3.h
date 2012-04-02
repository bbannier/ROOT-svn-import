int x;

enum my_enum {
  Trailways,
  Greyhound
};

class A {
public:
  class B {
  public:
    class C {
    public:
      class D {};
    public:
      void f() {}
    public:
      int c;
    };
  public:
    int b;
  };
public:
  int a;
public:
  int a_f(int p1,    int p2) { return 0; }
  int a_f(int p1, double p2) { return 0; }
};

int f(int p1, double p2);

