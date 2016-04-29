#include "mex.h"
/*#include "matrix.h"*/

/* Needed for memcpy */
#include <string.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

/* ************************************************************************
 * *** Some nice lua function wrappers ************************************
 * ***********************************************************************/

double my_lua_get_and_pop_number(lua_State* L) {
   if (!lua_isnumber(L, -1)) {
      lua_close(L);
      mexErrMsgIdAndTxt("readLua:LuaError",
              "Top of lua stack should be a number, but is not.");
   }
   double val = lua_tonumber(L, -1);
   lua_pop(L, 1);
   return val;
}

const char* my_lua_get_and_pop_string(lua_State* L, size_t* string_len) {
   if (!lua_isstring(L, -1)) {
      lua_close(L);
      mexErrMsgIdAndTxt("readLua:LuaError",
              "Top of lua stack should be a string, but is not.");
   }

   /* Get string and its length */
   const char* myval = lua_tolstring(L, -1, string_len);

   /* Allocate memory */
   char* val = (char*) mxCalloc(*string_len+1, sizeof(char));

   /* Copy string */
   memcpy(val, myval, (*string_len+1) * sizeof(char));

   lua_pop(L, 1);
   return (const char*) val;
}

double* my_lua_get_and_pop_vector(lua_State* L, size_t* array_len) {
   if (!lua_istable(L, -1)) {
      lua_close(L);
      mexErrMsgIdAndTxt("readLua:LuaError",
              "Top of lua stack should be a table, but is not.");
   }
   /* Get size */
   lua_len(L, -1);
   *array_len = (size_t) my_lua_get_and_pop_number(L);
   /* Allocate memory */
   double* val = (double*) mxCalloc((mwSize) *array_len, sizeof(double));

   /* Loop over elements */
   int i;
   for (i = 0; i < *array_len; i++) {
      /* Push index */
      lua_pushnumber(L, i+1);
      /* Get number */
      lua_gettable(L, -2);
      val[i] = my_lua_get_and_pop_number(L);
   }

   /* Make stack clean */
   lua_pop(L, 1);

   return val;
}

/* ************************************************************************
 * *** Some nice mex function wrappers ************************************
 * ***********************************************************************/

void checkForValidVariableName(const char* name) {
   mwSize i = 0;
   while (name[i] != '\0') {
      if ( (    ( 0x30 <= name[i] && name[i] <= 0x39 && i>0) /* Numbers */
             || (                    name[i] == 0x5F && i>0) /* Underscore */
             || ( 0x41 <= name[i] && name[i] <= 0x5A       ) /* Capital letters */
             || ( 0x61 <= name[i] && name[i] <= 0x7A       ) /* Small letters */
                   ) && (i <= mxMAXNAM) ) {
         i++;
      } else {
         mexErrMsgIdAndTxt( "readLua:invalidVariableName", /* TODO */
            "The name %s is not a valid Matlab variable Name.", name);
      }
   }

   /* Name is a valid */
   return;
}

void addFieldToOut(mxArray* out,
                   const char* fieldname) {
   /* Check the fieldname */
   checkForValidVariableName(fieldname);

   /* Add field to struct out */
   if (mxAddField(out, fieldname) == -1) {
      mexErrMsgIdAndTxt( "readLua:errorAddingField", /* TODO */
            "Error adding field to struct.");
   }

   return;
}

void addMatrixFieldToOut(mxArray* out,
                         const char* fieldname,
                         size_t mrows,
                         size_t ncols,
                         double* Matrix) {
   mxArray* myMatrix;

   /* Add field to struct out */
   addFieldToOut(out, fieldname);

   /* Create myMatrix */
   myMatrix = mxCreateDoubleMatrix((mwSize) mrows, (mwSize) ncols, mxREAL);

   /* Assign Matrix to myMatrix */
   mxSetPr(myMatrix, Matrix);

   /* Assign myMatrix to out struct */
   mxSetField(out, (mwIndex) 0, fieldname, myMatrix);

   return;
}

void addStringFieldToOut(mxArray* out,
                         const char* fieldname,
                         size_t stringLen,
                         const char* string) {
   mxArray* myString;

   /* Add field to struct out */
   addFieldToOut(out, fieldname);

   /* Create myString */
   myString = mxCreateString(string);

   /* Assign myMatrix to out struct */
   mxSetField(out, (mwIndex) 0, fieldname, myString);

   return;
}

/* Declare function whose body is defined later */
void myLuaGetTopAndAddFieldToOut(lua_State* L,
                                 mxArray* out,
                                 const char* key);


void addStructFieldToOut(lua_State* L,
                         mxArray* out,
                         const char* key) {
   /* Create struct array */
   mxArray* strct;
   strct = mxCreateStructMatrix((mwSize) 1, (mwSize) 1, 0, NULL);

   /* Add field to struct out */
   addFieldToOut(out, key);

   /* Assign strct to out struct */
   mxSetField(out, (mwIndex) 0, key, strct);

   /* Iterate over elements of the table */
   lua_pushnil(L);

   while (lua_next(L, -2) != 0) {
      /* Key is now at -2 and value at -1 (previous key was popped) */
      /* Check if the key is a string */
      if (!lua_isstring(L, -2)) {
         lua_close(L);
         mexErrMsgIdAndTxt("readLua:nonStringKeyInTable",
            "Lua table %s (not treated as vector) has a non-string key.");
      }
      /* Add the key-value-pair to the strct variable */
      myLuaGetTopAndAddFieldToOut(L, strct, lua_tostring(L, -2));
   }

   return;
}

void myLuaGetTopAndAddFieldToOut(lua_State* L,
                                 mxArray* out,
                                 const char* key) {
   int ltype;
   double* array;
   size_t arrayLen;
   const char* string;
   size_t stringLen;

   /* Get the type of the variable on top of stack */
   ltype = lua_type(L, -1);

   /* Decide what to do */
   if (ltype == LUA_TNUMBER) {
      /* Allocate space */
      array = (double*) mxCalloc((mwSize) 1, sizeof(double));
      /* Get a number */
      array[0] = my_lua_get_and_pop_number(L);
      /* A number is a matrix of format 1x1 */
      addMatrixFieldToOut(out, key, 1, 1, array);
   } else if (ltype == LUA_TTABLE) {
      /* If the table has the numeric key 1 with a numeric value,
       * we treat it as a vector */
      /* Push index */
      lua_pushnumber(L, 1);
      /* Get element */
      lua_gettable(L, -2);
      /* Test if vector */
      if (lua_isnumber(L, -1)) {
         /* Pop value (key was popped by lua_gettable) */
         lua_pop(L, 1);
         /* We are dealing with a vector */
         array = my_lua_get_and_pop_vector(L, &arrayLen);
         /* A vector is a matrix of format 1xarrayLen */
         addMatrixFieldToOut(out, key, 1, arrayLen, array);
      } else {
         /* Pop value (key was popped by lua_gettable) */
         lua_pop(L, 1);

         /* Table is treated as a record */
         addStructFieldToOut(L, out, key);

         /* Pop table */
         lua_pop(L, 1);
      }
   } else if (ltype == LUA_TSTRING) {
      /* Get a string and its length */
      string = my_lua_get_and_pop_string(L, &stringLen);
      /* Add it to out */
      addStringFieldToOut(out, key, stringLen, string);
   } else if (ltype == LUA_TNIL) {
      /* Given variable does not exist or is given the value nil,
       * we treat this as an empty array */
      addMatrixFieldToOut(out, key, 1, 0, NULL);
   } else {
      lua_close(L);
      mexErrMsgIdAndTxt("readLua:UnsupportedType",
              "Lua variable %s has unsupported type.", key);
   }

   return;
}

void myLuaGetByKeyAndAddFieldToOut(lua_State* L,
                                   mxArray* out,
                                   const char* key) {
   /* Put lua variable on top of stack */
   lua_getglobal(L, key);

   /* Add the top of stack to out */
   myLuaGetTopAndAddFieldToOut(L, out, key);

   return;
}

/* ************************************************************************
 * *** Actual mex entry function ******************************************
 * ***********************************************************************/

void mexFunction( int nlhs, mxArray *plhs[],
                  int nrhs, const mxArray *prhs[] )
{
   /* *******************************************************************
    * *** Checking i/o, preparing ***************************************
    * ******************************************************************/

   size_t mrows, ncols;
   char* cfgName;
   size_t nSymbols;
   mxArray* out;

   /* Check for proper number of arguments. */
   if(nrhs!=2) {
      mexErrMsgIdAndTxt( "readLua:invalidNumInputs", /* TODO */
                         "Exactly two inputs required: Filename and cell array of symbols to load.");
   } else if (nlhs!=1) {
    mexErrMsgIdAndTxt( "readLua:invalidNumOutputs", /* TODO */
            "Exactly one output required.");
   }

   /* The first input must be a string. */
   mrows = mxGetM(prhs[0]);
   ncols = mxGetN(prhs[0]);
   if (!mxIsChar(prhs[0]) || mrows!=1 || ncols<1) {
      mexErrMsgIdAndTxt( "readLua:invalidInput", /* TODO*/
            "First input must be a nonempty string.");
   }

   /* Get name of the cfg file */
   cfgName = (char*) mxCalloc(ncols+1, sizeof(char));
   if (mxGetString(prhs[0], cfgName, ncols+1) != 0)
      mexErrMsgIdAndTxt( "readLua:couldNotGetString", /* TODO */
              "Could not get string.");

   /* The second input must be a nonempty cell array */
   mrows = mxGetM(prhs[1]);
   ncols = mxGetN(prhs[1]);
   if (!mxIsCell(prhs[1]) || mrows!=1 || ncols<1) {
      mexErrMsgIdAndTxt( "readLua:invalidInput",
              "Second input must be a nonempty cell array.");
   }
   nSymbols = ncols;

   /* Create output struct */
   out = mxCreateStructMatrix((mwSize) 1, (mwSize) 1, 0, NULL);

   /* *******************************************************************
    * *** Opening Lua, reading file *************************************
    * ******************************************************************/

   lua_State* L;

   /* Open Lua */
   L = luaL_newstate();
   /* Open standard libraries */
   luaL_openlibs(L);

   /* Open file */
   if (luaL_loadfile(L, cfgName)) {
      lua_close(L);
      mexErrMsgIdAndTxt("readLua:LuaError",
              "Could not load file %s.", cfgName);
   }

   /* Execute file */
   if (lua_pcall(L, 0, 0, 0)) {
      lua_close(L);
      mexErrMsgIdAndTxt("readLua:LuaError",
              "Could not execute file %s.", cfgName);
   }

   /* *******************************************************************
    * *** Getting parameters ********************************************
    * ******************************************************************/

   size_t i;
   mxArray* mySymbol;
   char* symbol;

   /* Loop over all symbols which should be read */
   for (i=0; i<nSymbols; i++) {
      /* Get the symbol name */
      mySymbol = mxGetCell(prhs[1], i);

      /* Check if symbol name is an actual string */
      mrows = mxGetM(mySymbol);
      ncols = mxGetN(mySymbol);
      if (!mxIsChar(mySymbol) || mrows!=1 || ncols<1) {
         mexErrMsgIdAndTxt( "readLua:invalidInput", /* TODO*/
               "Second input must be a nonempty cell array which contains only nonempty strings.");
      }

      /* Extract string */
      symbol = (char*) mxCalloc(ncols+1, sizeof(char));
      if (mxGetString(mySymbol, symbol, ncols+1) != 0) {
         mexErrMsgIdAndTxt( "readLua:couldNotGetString", /* TODO */
               "Could not get string.");
      }

      /* Get the specified symbol (=key) from the lua config file and
       * add an appropriate field to the out array */
      myLuaGetByKeyAndAddFieldToOut(L, out, symbol);

      /* Deallocate symbol */
      mxFree(symbol);
   }

   /* *******************************************************************
    * *** Finish ********************************************************
    * ******************************************************************/

   /* Close lua */
   lua_close(L);

   /* Assign output */
   plhs[0] = out;

   return;
}
