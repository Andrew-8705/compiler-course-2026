// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/baldin_a_lab4_MLIR%shlibext --pass-pipeline="builtin.module(affine-trip-count)" %s | FileCheck %s

// CHECK-LABEL: func.func @test_simple_loop
func.func @test_simple_loop() {
  // CHECK: affine.for %arg0 = 0 to 10 {
  // CHECK: trip_count = 10 : i64
  affine.for %i = 0 to 10 {
    %0 = arith.constant 1 : i32
  }
  return
}

// CHECK-LABEL: func.func @test_offset_loop
func.func @test_offset_loop() {
  // CHECK: affine.for %arg0 = 5 to 15 {
  // CHECK: trip_count = 10 : i64
  affine.for %i = 5 to 15 {
    %0 = arith.constant 1 : i32
  }
  return
}

// CHECK-LABEL: func.func @test_step_loop
func.func @test_step_loop() {
  // CHECK: affine.for %arg0 = 0 to 10 step 2 {
  // CHECK: trip_count = 5 : i64
  affine.for %i = 0 to 10 step 2 {
    %0 = arith.constant 1 : i32
  }
  return
}

// CHECK-LABEL: func.func @test_complex_math_loop
func.func @test_complex_math_loop() {
  // CHECK: affine.for %arg0 = 2 to 20 step 3 {
  // CHECK: trip_count = 6 : i64
  affine.for %i = 2 to 20 step 3 {
    %0 = arith.constant 1 : i32
  }
  return
}

// CHECK-LABEL: func.func @test_dynamic_upper_bound
func.func @test_dynamic_upper_bound(%N : index) {
  // CHECK: affine.for %arg1 = 0 to %arg0 {
  // CHECK-NOT: trip_count
  affine.for %i = 0 to %N {
    %0 = arith.constant 1 : i32
  }
  return
}

// CHECK-LABEL: func.func @test_dynamic_lower_bound
func.func @test_dynamic_lower_bound(%start : index) {
  // CHECK: affine.for %arg1 = %arg0 to 100 {
  // CHECK-NOT: trip_count
  affine.for %i = %start to 100 {
    %0 = arith.constant 1 : i32
  }
  return
}

// CHECK-LABEL: func.func @test_zero_trip_loop
func.func @test_zero_trip_loop() {
  // CHECK: affine.for %arg0 = 10 to 5 {
  // CHECK: trip_count = 0 : i64
  affine.for %i = 10 to 5 {
    %0 = arith.constant 1 : i32
  }
  return
}