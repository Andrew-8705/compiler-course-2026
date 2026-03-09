// RUN: %clang_cc1 -load %llvmshlibdir/baldin_a_lab1_ClangAST%pluginext -plugin const_plugin -fsyntax-only %s 2>&1 | FileCheck %s

// CHECK-LABEL: test_full_const
// CHECK: const int* const p1 {{=}}
void test_full_const() {
    int a = 10;
    int* p1 = &a;
    int b = *p1;
}

// CHECK-LABEL: test_const_data
// CHECK: const int* p2 {{=}}
void test_const_data() {
    int a = 10;
    int* p2 = &a;
    p2++; 
}

// CHECK-LABEL: test_const_pointer
// CHECK: int* const p3 {{=}}
void test_const_pointer() {
    int a = 10;
    int* p3 = &a;
    *p3 = 20; 
}

// CHECK-LABEL: test_reference
// CHECK: const int& ref {{=}}
void test_reference() {
    int a = 10;
    int& ref = a;
    int c = ref;
}

// CHECK-LABEL: test_no_const
// CHECK: int* p4 {{=}}
// CHECK-NOT: const int * p4
void test_no_const() {
    int a = 10;
    int* p4 = &a;
    p4++;
    *p4 = 30;
}

// CHECK-LABEL: test_already_const
// CHECK: const int* const p5 {{=}}
void test_already_const() {
    int a = 10;
    const int* const p5 = &a;
    int b = *p5;
}

// CHECK-LABEL: test_arguments
// CHECK: void test_arguments(const int* const arg1, const int& arg2)
void test_arguments(int* arg1, int& arg2) {
    int val = *arg1 + arg2;
}

// CHECK-LABEL: test_array_mutation
// CHECK: int* const p6 {{=}}
void test_array_mutation() {
    int arr[5] = {1, 2, 3, 4, 5};
    int* p6 = arr;
    p6[2] = 10;
}