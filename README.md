# readLua for Matlab and Octave

A MEX function that executes a lua file and then gets variables
from it.

Everything was only tested on a 64bit Ubuntu 14.04 machine in Matlab
and Octave.
Help for testing on other platforms and machines is appreciated.

Read more about lua here: http://www.lua.org/

## Building

You need a recent Matlab or Octave version, a C compiler, that is
supported by Matlab (e.g. gcc, Microsoft Visual C++) or Octave.

The first step is to get lua and its header files.
This can be done by getting the sources and building it or
getting precompiled libraries.
If you get precompiled libraries, make sure that they are compatible with
your C compiler.

Now you start your Matlab/Octave and configure your mex compiler to
compile C with a C compiler which is compatible to
Matlab/Octave and your lua libraries.

Now you edit `make.m`. You need to specify paths to the lua header files
and the lua library, as well as the lua library name. Then you run `make.m`
and it will compile everything giving you a `readLua.mexa64` or something
similar, depending on you platform. It will also run a small test and
build a help function `readLua.m`.

Now you can copy `readLua.m` and `readLua.mexa64` (or similar extension)
to a place, where it can be found by Matlab/Octave. Running `clean.m`
will make the folder clean again.


## Usage

Assuming `config.lua` is a lua file on your path, where you want to
read the variables `important` and `crucial`, then you run

    s = readLua('config.lua', {'important', 'crucial'});

This will give you a Matlab/Octave struct `s` with fields `important` and
`crucial` which contain the values from the lua file `config.lua`.

### Example

`config.lua` could look like this:

    important = {1, 2, 3}
    crucial = 3
    need = {look = 'at this'}
    
    important[1] = crucial
    need["answer"] = 42

By calling

    s = readLua('config.lua', {'important', 'crucial', 'need'});

we would get the follwing struct
    
    s =
        important: [3 2 3]
          crucial: 3
             need: [1x1 struct] 
    
    s.need = 
          look: 'at this'
        answer: 42

## Restrictions

There is no restrictions on what lua files can be used, apart from the
compatibility of the version of the library you use.

There is, however, the restriction that you can only read the following
type of variables at the moment:

 * _Numbers:_
   
   For example: `2`, `3.0`, `2.3e-8`
 * _Vectors:_ That means tables that are sequences
   (see [lua manual](http://www.lua.org/manual/5.2/manual.html#3.4.6)),
   where the values have to be numeric.
   
   For example: `{1, 2.0, 3e7}`, `{9, 8, foo='bar'}` (`foo` would be dropped)
   
   Not possible: `{1, 2, nil, 4}`, `{'baz', 2, 3}`

   Please, use only numeric keys that are integer and positive.
 * _Strings:_
   
   For example: `''`, `'bar'`
 * _Tables:_
   
   Where all keys are strings that may be Matlab/Octave struct fields.
   (Note that if you have keys that can *not* be a struct field, you will
    most likely crash Matlab/Octave)
   Nesting is possible.
   
   For example: `{a=1, b='foo'}`, `{a={a=1}}`
   
   Not possible: `{["1asd"] = 7}`


## Future

I might:

 * Include binaries in this repo
 * Implement support for reading matrices
 * Fix possible bugs
 * Improve error handling
