; ModuleID = '../test-cases/nest_while_2.cpp'
source_filename = "../test-cases/nest_while_2.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: mustprogress noinline norecurse nounwind optnone sspstrong uwtable
define dso_local noundef i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  store i32 0, ptr %1, align 4
  store i32 0, ptr %2, align 4
  store i32 0, ptr %3, align 4
  store i32 0, ptr %4, align 4
  br label %5

5:                                                ; preds = %22, %0
  %6 = load i32, ptr %2, align 4
  %7 = icmp slt i32 %6, 5
  br i1 %7, label %8, label %25

8:                                                ; preds = %5
  br label %9

9:                                                ; preds = %19, %8
  %10 = load i32, ptr %3, align 4
  %11 = icmp slt i32 %10, 5
  br i1 %11, label %12, label %22

12:                                               ; preds = %9
  br label %13

13:                                               ; preds = %16, %12
  %14 = load i32, ptr %4, align 4
  %15 = icmp slt i32 %14, 5
  br i1 %15, label %16, label %19

16:                                               ; preds = %13
  %17 = load i32, ptr %4, align 4
  %18 = add nsw i32 %17, 1
  store i32 %18, ptr %4, align 4
  br label %13, !llvm.loop !6

19:                                               ; preds = %13
  %20 = load i32, ptr %3, align 4
  %21 = add nsw i32 %20, 1
  store i32 %21, ptr %3, align 4
  br label %9, !llvm.loop !8

22:                                               ; preds = %9
  %23 = load i32, ptr %2, align 4
  %24 = add nsw i32 %23, 1
  store i32 %24, ptr %2, align 4
  br label %5, !llvm.loop !9

25:                                               ; preds = %5
  %26 = load i32, ptr %2, align 4
  %27 = load i32, ptr %3, align 4
  %28 = add nsw i32 %26, %27
  %29 = load i32, ptr %4, align 4
  %30 = add nsw i32 %28, %29
  ret i32 %30
}

attributes #0 = { mustprogress noinline norecurse nounwind optnone sspstrong uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"clang version 19.1.7"}
!6 = distinct !{!6, !7}
!7 = !{!"llvm.loop.mustprogress"}
!8 = distinct !{!8, !7}
!9 = distinct !{!9, !7}
