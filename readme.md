
# libytil

A library containing various utilities.


## Modules

Group   | Description
--------|------------
con     | container modules
def     | convenient macros and definitions
enc     | encoding modules
ext     | extensions of system headers
gen     | generic modules
sys     | system modules


Group   | Module    | Description
--------|-----------|------------
con     | art       | adaptive radix tree
con     | list      | doubly linked list
con     | ring      | ring buffer
con     | vec       | vector
def     | bits      | bit manipulation macros
def     | cast      | cast macros
def     | color     | color escape sequences
def     | gen       | general purpose macros
def     | magic     | magic numbers for classes
def     | os        | os specific macros
def     | rc        | return convenience macros
def     | simd      | SIMD commands
enc     | base64    | base64 encoding
enc     | base85    | base85 encoding
enc     | pctenc    | percent encoding
enc     | qpenc     | qouted-printable encoding
gen     | error     | error handling
gen     | log       | logging
gen     | path      | path management
gen     | str       | dynamic strings
sys     | env       | environment utilities
sys     | path      | system path utilities
sys     | proc      | process utilities
sys     | service   | windows service utilities
-       | test      | testsuite


## How to compile

### generate config files

```
make config
```

This starts the interactive config dialog.
Just quit with 'q' and keep the default values or choose to modify them.


### debug build

```
make debug
```

This builds the library in 'libytil/build/debug/libytil.a'.


### release build

```
make release
```

This builds the library in 'libytil/build/release/libytil.a'.


### tests

```
make test
```

This builds the testsuite in 'libytil/build/test/ytil'.



## How to use

``` c

#include <ytil/gen/log.h>


int main(int argc, char *argv[])
{
    size_t unit, target;

    unit    = log_unit_add(LIT("main"));
    target  = log_target_add_stdout(LOG_COLOR_AUTO);

    log_sink_set_level(unit, target, LOG_INFO);

    log_info(unit, "Hello World");

    log_free();

    return 0;
}

```

Example Usage with GCC

```
gcc -o my_app my_app.c -I./libytil/include -L./libytil/build/debug -lytil
```


## Documentation

```
make doc
make devdoc
```

Generates the documentation with Doxygen in 'libytil/doc'.
The 'doc' documentation includes only documentation included in header files.
The 'devdoc' documentation includes documentation in source files as well.
