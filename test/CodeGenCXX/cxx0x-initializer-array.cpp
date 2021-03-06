// RUN: %clang_cc1 -triple i386-unknown-unknown -std=c++11 -S -emit-llvm -o - %s -Wno-address-of-temporary | FileCheck %s

// CHECK: @[[THREE_NULL_MEMPTRS:.*]] = private constant [3 x i32] [i32 -1, i32 -1, i32 -1]

struct A { int a[1]; };
typedef A x[];
int f() {
  x{{{1}}};
  // CHECK: define i32 @_Z1fv
  // CHECK: store i32 1
  // (It's okay if the output changes here, as long as we don't crash.)
  return 0;
}

namespace ValueInitArrayOfMemPtr {
  struct S {};
  typedef int (S::*p);
  typedef p a[3];
  void f(const a &);

  struct Agg1 {
    int n;
    p x;
  };

  struct Agg2 {
    int n;
    a x;
  };

  struct S1 {
    p x;
    S1();
  };

  // CHECK: define void @_ZN22ValueInitArrayOfMemPtr1fEi
  void f(int n) {
    Agg1 a = { n };
    // CHECK: store i32 -1,

    Agg2 b = { n };
    // CHECK: call void @llvm.memcpy.p0i8.p0i8.i32(i8* %{{.*}}, i8* bitcast ([3 x i32]* @[[THREE_NULL_MEMPTRS]] to i8*), i32 12, i32 4, i1 false)
  }

  // CHECK: define void @_ZN22ValueInitArrayOfMemPtr1gEv
  void g() {
    // CHECK: store i32 -1,
    f(a{});
  }
}

namespace array_dtor {
  struct S { S(); ~S(); };
  using T = S[3];
  void f(const T &);
  void f(T *);
  // CHECK: define void @_ZN10array_dtor1gEv(
  void g() {
    // CHECK: %[[ARRAY:.*]] = alloca [3 x
    // CHECK: br

    // Construct loop.
    // CHECK: call void @_ZN10array_dtor1SC1Ev(
    // CHECK: br i1

    // CHECK: call void @_ZN10array_dtor1fERA3_KNS_1SE(
    // CHECK: br

    // Destruct loop.
    // CHECK: call void @_ZN10array_dtor1SD1Ev(
    // CHECK: br i1
    f(T{});

    // CHECK: ret void
  }
  // CHECK: define void @_ZN10array_dtor1hEv(
  void h() {
    // CHECK: %[[ARRAY:.*]] = alloca [3 x
    // CHECK: br

    // CHECK: call void @_ZN10array_dtor1SC1Ev(
    // CHECK: br i1
    T &&t = T{};

    // CHECK: call void @_ZN10array_dtor1fERA3_KNS_1SE(
    // CHECK: br
    f(t);

    // CHECK: call void @_ZN10array_dtor1SD1Ev(
    // CHECK: br i1

    // CHECK: ret void
  }
  // CHECK: define void @_ZN10array_dtor1iEv(
  void i() {
    // CHECK: %[[ARRAY:.*]] = alloca [3 x
    // CHECK: br

    // CHECK: call void @_ZN10array_dtor1SC1Ev(
    // CHECK: br i1

    // CHECK: call void @_ZN10array_dtor1fEPA3_NS_1SE(
    // CHECK: br

    // CHECK: call void @_ZN10array_dtor1SD1Ev(
    // CHECK: br i1
    f(&T{});

    // CHECK: ret void
  }
}
