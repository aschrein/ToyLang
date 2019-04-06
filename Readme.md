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
defun f:(2*arg - 1)/3;
print(f(99));
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
  %1 = call i32 @f(i32 99)
  call void (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @g_const_0, i32 0, i32 0), i32 %1)
  ret i32 0
}

define i32 @f(i32) {
entry:
  %1 = mul i32 2, %0
  %2 = sub i32 %1, 1
  %3 = sdiv i32 %2, 3
  ret i32 %3
}
```
---
