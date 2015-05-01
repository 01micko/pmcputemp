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
- Cairo
- Gtk+ >= 2.0 (broken in gtk3 for now)
- Xlib

Usage
-----

Just run `pmcputemp` from the command line, it does accept an integer 
switch (1 - 10) to adjust the default delay (5). It will attempt 
to generate a config directory and temporary storage at `$HOME/.config/pmcputemp` at first
run. See the script `pmcputemp.sh`. In this directory, `pmcputemprc` should be 
generated with the path to the cpu temperature file. In the case of multicore
processors, the first core is the one which is sourced. This is a basic limitation
of the program, after all, poor men and women may find it difficult to procure
a multicore processor machine. The icons with the readouts are stored in this 
directory and is written to approxiamately every 5 (or the switch value) seconds. 

Bugs
----
- Fahrenheit is unsupported.
- The script may not find your temperature file. If you know your sensor works,
you can enter the path manually, but make sure there is **no** carriage return.

Please report all bugs here for now.
Translations welcome.
