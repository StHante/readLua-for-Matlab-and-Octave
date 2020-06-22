# readLua for Matlab and Octave

A MEX function that executes a lua file and then gets variables
from it.

Everything was only tested on a 64bit Ubuntu and Windows 10 in Matlab 2016.
Help for testing on other platforms and machines is appreciated.

Read more about lua here: http://www.lua.org/

## Get binaries

I added a subdirectory `bin`, where there are subdirectories for Platform and then Matlab/Octave-Versions which I compiled. If you have a newer Matlab Version, those might still work, but I can't say for sure.

I have used Lua 5.2 for builing.

I decided to do this, because it looks that is has become a pain to install the correct compiler in Matlab.

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
   Nesting is possible.
   
   For example: `{a=1, b='foo'}`, `{a={a=1}}`
   
   Not possible: `{["2asd"] = 7}`
 * _nil:_
   
   A `nil` in lua becomes an empty `1x0` matrix in Matlab/Octave. Note
   that every undefined variable in lua also has the value `nil`, wich
   means that there will be no error, if you try to fetch a variable from
   a lua file that does not exist.
   
   If you try to read a table, where one key is assigned to be `nil`, 
   e.g. `{a=3, b=nil}`, the key `b` will _not_ appear in the struct,
   because `b` is treated as undefined.


## Future

I might:

 * Include more binaries in this repo
 * Implement support for reading matrices
 * Fix possible bugs
 * Improve error handling
