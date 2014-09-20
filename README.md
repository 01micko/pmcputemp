pmcputemp
====

a poor man's cpu temperature status icon

Build
-----
run:

```
./configure
```

then:

```
make
```

and as root:

```
make install
```

Some basic options to `configure` are provided. Run `configure --help`

Depends
-------
Cairo
Gtk+ >= 2.0 (not yet stested in Gtk+-3.0)
Xlib

Usage
-----
Just run `pmcputemp` from the command line, it needs no options. It will attempt 
to generate a config directory and temprary storage at `$HOME/.tempicon/` at first
run. See the script `pmcputemp.sh`. In this directory, `pmcputemprc` should be 
generated with the path to the cpu temperature file. In the case of multicore
processors, the first core is the one which is sourced. This is a basic limitation
of the program, after all, poor men and women may find it difficult to procure
a multicore processor machine. The icons with the readouts are stored in this 
directory and is written to approxiamately every 5 seconds. 

Bugs
----
- Fahrenheit is unsupported.
- The script may not find your temperature file. If you know your sensor works,
you can enter the path manually, but make sure there is **no** carriage return.

Please report all bugs here for now.
Translations welcome.
