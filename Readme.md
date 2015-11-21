Noumenon
========
Noumenon is a dynamic, strongly typed script language. It is simple, yet
powerful and extensible. In contrast to many other languages, there is no
automatic type conversion in Noumenon.

Noumenon is "the meant" in contrary to Phenomenon, "the observed". Why the
name? Heck, you know as much as I do.


Compiling
---------
Run `make noumenon`. If you do not have a make command at your disposal, simply compiling every cpp-file togehter should do the trick as well.

Noumenon is developed under Linux but *should* work on different platforms as well. If you experience any issues, please report.


Getting started
---------------
There is a directory "examples" which includes some demo scripts. Just run `noumenon FILE`.


Build-in functions
------------------
* `typeof(argument)`: Returns the type of the first argument as a string.
* `print(argument, ...)`: Prints all arguments to stdout.
* `println(argument, ...)`: Does the same as `print` -- but appends a newline.
* `range(from, to)`: Creates an array with all integer values from `from` to `to`.
* `length(argument)`: Returns the length of an array, number of mappings in an object, or null for all other values.
* `require(filename)`: Executes the given file and return its returnvalue or `null` if no `return` statement was found.

In interactive mode, there is one more function available:
* `list()`: Lists all variables.


To do
-----
* Verbose mode that warns on non-boolean types in `if`-conditions etc.
* Implement `==` and `!=` for arrays and function.
* Explicit type casting.
* Character-to-Int and Int-to-Character functions like `asc` and `chr`.
* Tutorial on how to embed Noumenon.
* File I/O. Something like:
```
var file = IO.open("output.txt", "w");
file.writeln("hello world");
file.writeln(1, "\t", 2.0, {});
file.close();

var dir = IO.dir(".");
for(var subdir : dir.subdirs()) {
    println(subdir);
}
```
* Native Code. Similar to "require", but being able to wrap a native library, i.e. ".so" or ".dll" files.

License
-------
(c) 2015 Tim Wiederhake, licensed under GPLv3 or leater
