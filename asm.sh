/home/aschrein/dev/llvm/build/install/bin/llc -relocation-model=pic --x86-asm-syntax=intel out.ll &&\
    gcc -fPIC out.s &&\
    objdump -M intel -d a.out > disasm.s &&\
    ./a.out &&\
    echo "SUCCESS"
