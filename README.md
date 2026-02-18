# BirdSharp
**BirdSharp is a high-performance, mid-level systems language that is the perfect balance between low-level control and a high-level developer experience.**


## BirdSharp Examples
### 1. Hello World (using the Standard Library)
This simple Hello World program can already show BirdSharp's versaltility. This code uses the Standard Library, a simple library that converts high-level instructions into syscalls.

```c
#!include "std.bsh"

int main(long argc, char **argv) {
    print("Hello, BirdSharp!\n")
    exit(0)
    return 0
}
```

### 2. Fibonacci Function
In particular, a Fibonnaci function can say whether a programming language is incompetent or powerful. Again, BirdSharp excels with its easy syntax

```c
int fibonacci(int n) {
    if (n < 2) {
        return n
    }
    
    int a = 0;
    int b = 1;
    int i = 2;
    int temp = 0;
    
    while (i <= n) {
        temp = a + b;
        a = b;
        b = temp;
        i = i + 1;
    }
    return b;
}
```
* Note that there are no for loops; they aren't implemented yet *

### 3. Arrays
BirdSharp's arrays are very much similar to C's. However, array initializations are entirely before the variable (*int[10]* a instead of *int a[10]*)
```c
#!include "code/std.bsh"

int main(long argc, char **argv) {
    int[10] a
    a[0] = 5
    a[1] = 10
    a[2] = 15
    print("Result: ")
    print(int_to_string(a[0] + a[1] * a[2]))
    print("\n")
    exit(0)
    return 0;
}
```

### 4. Structs
Similar to arrays, BirdSharp's structures are close to C's. The following code is a demonstration of creating a structure and reading and writing memory from it.
```c
#!include "std.bsh"

struct A {
    long x
    long y
}



int main(long argc, char **argv) {
    struct A string = cast(struct A, 0)
    string.x = 10
    string.y = 20
    exit(cast(int, string.x + string.y))
    return 0;
}
```

## BirdSharp's Core Ideas:
* **No excessive magic:** The code should execute exactly as the coder intends it: some optimizations are free to occur, but the user's main purpose will not be overriden.
* **Readability:** The coder, given code, should be able to understand it's exact intent without having to rely on external help.
* **Directives:** The coder, at any point in the code, can change specific features about BirdSharp. This helps the language become more diverse, although these directives are limited.

## The BirdSharp Toolchain
- Frontend: Processes BirdSharp through the tokenizer, parser, and typechecker to create refined ASTs (Abstract Syntax Trees).
- IR layer: Lowers the ASTs into a custom Intermediate Representation. The IR is supposed to be cross-platform and help structure logic before worrying about hardware.
- A backend layer: Currently only targets Assembly (specifically macho64 assembly). It translates the Intermediate Representation into Assembly code, and assembles and links them (using external processes) into an executable that has no bloat.

## Getting Started
- Build the executables: BirdSharp is rather easy to compile, as it is meant to be a developer friendly project. You can simply run *build.sh* to get the bs and irc executables, or even run *run.sh* to build it and run the code in the code/ folder.
- Compile the program: Compiling is also rather easy. If the output path isn't specified, BirdSharp will try to dump its results into a res/ folder, and will throw an error if it can't find one. This is counter-intuitive, and will be patched soon. This is an example of a BirdSharp compiler process that specifies both the input and output files
```sh
./lang/bs code/main.bsh
```
- Running the output: BirdSharp delivers the executable into a res/main file by default. However, if the output file is specified, it will dump its result there instead. Hence, the output can easily be ran as such:
```sh
./main
```

## Possible Improvements
I am open to possible contributions and ways to improve BirdSharp further, because it still has a bunch of problems. These are some examples of things that need to be added or improved.
- [ ] For Loops
- [ ] IR Optimizations: Dead-code elimination and constant-folding
- [ ] Output Patch: Help solve the default res/ folder dependency
