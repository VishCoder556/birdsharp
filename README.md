# BirdSharp
**BirdSharp is a high-performance, mid-level systems language that is the perfect balance between low-level control and a high-level developer experience.**

## Current Status: Alpha
BirdSharp is currently in active development. The core compiler pipeline is functional for MacOS (macho64), but many high-level language features are still being drafted.


## BirdSharp Examples
### 1. Hello World (using the Standard Library)

```c
#!include "std.bsh"

int main(long argc, char **argv) {
    print("Hello, BirdSharp!\n")
    exit(0)
    return 0
}
```

### 2. Fibonacci Function

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

### 3. Modes
Modes are probably one of BirdSharp's greatest features, and also one of its most unique ones. Modes are the only features in BirdSharp that don't execute as exactly what is being told of it. There are many types of modes:
    1. Preprocessor Modes (i.e. #!include)
        These change the tokens from the tokenizer, changing the output of all succeeding stages.
    2. Parsing Modes (#!flat or #!autovar)
        These modes change how something is parsed. For example, `autovar on` allows 'a = 5' instead of having to do 'int a = 5'. This is something that happens while everything is being parsed to an AST.
    3. Typechecking / Revising Modes (NONE EXIST YET)
        These modes modify small additional details about ASTs. This includes optimizations. However, optimizations are currently built-in to BirdSharp without any modes to change them.
    4. Compiling (NONE EXIST YET)
        These modes worry about how an object will be compiled into its executable. `#!extern` is a compiling mode because the only thing it changes is the output of the file.

This sample is an example of both preprocessor and parsing modes, the only modes that currently exist in BirdSharp:
```c
#!include "std.bsh"

#!flat
#!autovar on
print("Hello World\n")
return 0
```



## Getting Started
- Build the executables: BirdSharp is rather easy to compile, as it is meant to be a developer friendly project. You can simply run *build.sh* to get the bs and irc executables, or even run *run.sh* to build it and run the code in the code/ folder.
- Compile the program: Compiling is also rather easy. If the output path isn't specified, BirdSharp will try to dump its results into a res/ folder, and will throw an error if it can't find one. This is counter-intuitive, and will be patched soon. This is an example of a BirdSharp compiler process that specifies both the input and output files
```sh
./lang/bs code/main.bsh
```
- Running the output: BirdSharp delivers the executable into a 'main' file by default. However, if the output file is specified, it will dump its result there instead. Hence, the output can easily be ran as such:
```sh
./main
```
- Keep in mind that my project requires dependencies depending on the output format:
    -x86_64: requires YASM and CLANG to compile
    -ARM64: requires just CLANG to compile
- These dependencies should be in $PATH, or else BirdSharp will throw an error

## BirdSharp's Core Ideas:
* **No excessive magic:** The code should execute exactly as the coder intends it: some optimizations are free to occur, but the user's main purpose will not be overriden.
* **Readability:** The coder, given code, should be able to understand it's exact intent without having to rely on external help.
* **Directives:** The coder, at any point in the code, can change specific features about BirdSharp. This helps the language become more diverse, although these directives are limited.

## The BirdSharp Toolchain
- Frontend: Processes BirdSharp through the tokenizer, parser, and typechecker to create refined ASTs (Abstract Syntax Trees).
- IR layer: Lowers the ASTs into a custom Intermediate Representation. The IR is supposed to be cross-platform and help structure logic before worrying about hardware.
- A backend layer: Currently only targets Assembly (specifically macho64 assembly). It translates the Intermediate Representation into Assembly code, and assembles and links them (using external processes) into an executable that has no bloat.


## Possible Improvements
I am open to possible contributions and ways to improve BirdSharp further, because it still has a bunch of problems. These are some examples of things that need to be added or improved.
- [ ] For Loops
- [ ] Support for more platforms
- [ ] Cleaner codebase
- [ ] More modes
