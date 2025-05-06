pmcputemp
====

a poor man's cpu temperature gtk status icon

Usage
-----

Just run `pmcputemp` from the command line

Options
-------

- (int integer) from 1 to 10 to override the default refresh of 5 seconds.
- d	makes the widget a dark grey with light text
- l	(lower case 'L') makes the widget a light grey with dark text
- (char * module_name) the name of a kernel module, mistakes are ignored

_NOTE_: do not prepend with a dash, just use the plain character

Example:

```
pmcputemp 3
```

The 3 parameter will override the default 5 seconds. 

or

```
pmcputemp d 7
```

The 'd' will show a dark icon and the delay will be 7 seconds

or

```
pmcputemp l w83627hf 2
```

The 'l' will show a light icon and the delay will be 2 seconds and the kernel
module 'w83627hf' will attempt to be loaded. The 'module' parameter is only
useful on first run of the program and otherwise is ignored.

The options can be used in any order, but only 3 options allowed or else the 
default coloured icon shows with the 5 second delay.

It will attempt to generate a config directory
and temporary storage at `$HOME/.config/pmcputemp` at first
run. See the script `pmcputemp-sh`. In this directory, `pmcputemprc` should be 
generated with the path to the cpu temperature file. In the case of multicore
processors, the first core is the one which is sourced. This is a basic limitation
of the program, after all, poor men and women may find it difficult to procure
a multicore processor machine. The icons with the readouts are stored in this 
directory and is written to approxiamately every 5 (or the switch value) seconds.

If your temperature reading seems wrong (compared to lm_sensors) you can manually
edit the `pmcputemprc` to point to a different sensor data file. You can find
these by running as root:


`find` `/sys` `-type f -name 'temp*_input'` 


or


`find` `/sys` `-type f -name 'temp'`


`pmcputemp` can be started when X starts. Please consult your distribution's
documentation to enable that feature, which will vary between window managers
and desktop environments. 

Build
-----

_run:_

```
./configure [options]
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

DESTDIR is supported.

Depends
-------

- Cairo
- Gtk+-2.0 or Gtk+-3.0

Optional
- [http://www.lm-sensors.org/ lm_sensors]
- [https://chiselapp.com/user/jamesbond/repository/mdview3/home mdview]



Bugs
----
- Fahrenheit is unsupported.
- The script may not find your temperature file. If you know your sensor works,
you can enter the path manually, but make sure there is **no** carriage return.

Please report all bugs here for now or to 01micko AT gmx DOT com.

Translations welcome.
