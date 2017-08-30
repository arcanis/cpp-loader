# cpp-loader

> A C++ loader for Webpack

## Installation

```
$> yarn add cpp-loader
```

## Requirements

- You will need a version of both Clang and the Libclang with a decent C++17 support to install the loader.
- At runtime, the `em++` toolchain must be located inside your `PATH` for this loader to work properly ([instructions](https://github.com/juj/emsdk#installation-instructions)).

Of note: if you've just installed emscripten for the first time, the first compilation will be VERY slow (because the standard library will be built at the same time). Subsequent compilations will be much faster.

## Usage

**webpack.config.js:**

```javascript
module.exports = {

    // ...,

    module: {
        rules: [ {
            test: /\.cc$/,
            loader: 'cpp-loader'
        } ]
    }

};
```

**index.js:**

```javascript
import { addition, range } from './lib.cc';

console.log(addition(2, 2));
console.log(range(10, 100));
```

**lib.cc:**

```c++
#include <vector>

int addition(int a, int b)
{
    return a + b;
}

std::vector<int> range(int from, int to)
{
    std::vector<int> r;

    for (auto t = from; t < to; ++t)
        r.emplace_back(t);

    return r;
}
```

## How does it work?

- First we parse the source file using the libclang. We extract all the exported symbols from the main file (which means that any function and class defined in an header file will be ignored, as will be the static functions defined in the main file).
- For each of these functions and classes, we add a new [embind](https://kripken.github.io/emscripten-site/docs/porting/connecting_cpp_and_javascript/embind.html) entry at the bottom of the original source file, in order to automatically expose it to Javascript.
- In order to add support for the STL data types out-of-the box, we also wrap all the function (and class function) pointers into the [emmagic](https://github.com/manaflair/emmagic) wrappers, which automatically deal with this for us.
- Finally, the resulting file is compiled via emscripten into asm.js, and webpack can bundle it like any other javascript package.

## Limitations

The main limitation is the implementation; most of the following issues could be solved by improving the loader:

- Not all STL containers are supported.

  **Possible fix:** They just have to be implemented in emmagic.

- Currently only works on Windows because I'm lazy.

  **Possible fix:** I need to be less lazy. Also, the build script shouldn't use bashisms.

- The compilation output needs to be asm.js rather than WebAssembly.

  **Possible fix:** An easy fix would be to generate an extra file instead of javascript, and tell Webpack not to bundle it inside the final output. I'm not entirely sure how to make this happen with Webpack, so if someone can investigate this, it would be really helpful.

- A separate compilation is done for each file to compile. It's quite inefficient, because it means that if two files depend on the same common code, this code will be duplicated inside each asm.js module, and will be stored twice in the resulting bundle. It's also harmful, because instances of a class created in one file can't be used in the others.

  **Possible fix:** Fixing this would require to process all the dependencies from all the files at once, and then to process all the C++ files all at once (effectively linking them together).

- The code has to be compiled with the C++17 standard.

  **Possible fix:** Unfortunately there's no easy fix for this one; emmagic does its magic through a lot of metaprogramming hacks that have been greatly simplified with the latest standards, and I can't imagine doing without it. Fortunately the C++ standards are mostly backward-compatible, and the compilation only needs to run a single computer (cause after that it becomes pure Javascript!), so it shouldn't be a big issue.
