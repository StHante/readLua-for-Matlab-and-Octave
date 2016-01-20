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
This can be done by building it or getting precompiled libraries.
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
    
    important[1] = crucial

By calling

    s = readLua('config.lua', {'important', 'crucial'});

we would get the follwing struct
    
    s = 
        important: [3 2 3]
          crucial: 3


## Restrictions

There is no restrictions on what lua files can be used, apart from the
compatibility of the version of the library you use.

There is, however, the restriction that you can only read the following
type of variables at the moment:

 * Numbers
 * Vectors, that means tables that have numeric keys from 1 up to the
   number of keys in the table.
 * Strings


## Future

I will hopefully implement the functionality to support lua tables, that
work like Matlab/Octave structs and I will maybe implement support for
reading matrices.
