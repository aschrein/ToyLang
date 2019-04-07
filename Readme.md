---
# A Toy Language built using flex+bison+llvm
---
## Build and Run
```console
git clone https://github.com/aschrein/ToyLang.git
cd ToyLang
mkdir build
cd build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=install -DCMAKE_EXPORT_COMPILE_COMMANDS=YES
ninja && ./llvmTest ../foobar.toy && sh ../asm.sh

```
---
### The app translates this:
```js
defun getDiff(a, b) {if (a<b) {b-a} else {a-b}};
// Should be 1
def gd : getDiff(1, 2);
print(gd);
// Should be 11
def gd : getDiff(22, 11);
print(gd);
```
### Into this
```js
; ModuleID = 'VLisp'
source_filename = "VLisp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

@g_const_0 = internal constant [4 x i8] c"%i\0A\00"

declare void @printf(i8*, ...)

define i32 @main(i32) {
entry:
  %1 = call i32 @getDiff(i32 1, i32 2)
  %2 = alloca i32
  store i32 %1, i32* %2
  %3 = load i32, i32* %2
  call void (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @g_const_0, i32 0, i32 0), i32 %3)
  %4 = call i32 @getDiff(i32 22, i32 11)
  %5 = alloca i32
  store i32 %4, i32* %5
  %6 = load i32, i32* %5
  call void (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @g_const_0, i32 0, i32 0), i32 %6)
  ret i32 0
}

define i32 @getDiff(i32, i32) {
entry:
  %2 = alloca i32
  store i32 %0, i32* %2
  %3 = alloca i32
  store i32 %1, i32* %3
  %4 = load i32, i32* %3
  %5 = load i32, i32* %2
  %6 = icmp slt i32 %5, %4
  br i1 %6, label %then, label %else

then:                                             ; preds = %entry
  %7 = load i32, i32* %2
  %8 = load i32, i32* %3
  %9 = sub i32 %8, %7
  br label %merge

else:                                             ; preds = %entry
  %10 = load i32, i32* %3
  %11 = load i32, i32* %2
  %12 = sub i32 %11, %10
  br label %merge

merge:                                            ; preds = %else, %then
  %13 = phi i32 [ %9, %then ], [ %12, %else ]
  ret i32 %13

; uselistorder directives
  uselistorder i32 1, { 3, 4, 0, 1, 2 }
}

```
---
