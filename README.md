Hydra
=====

Distributed under the GNU General Public License, Version 2.0 - see accompanying file `LICENSE.txt` or copy at `http://www.gnu.org/licenses/gpl-2.0.txt`.

This daemon is designed to multiplex between different HTTP servers distinguished by the `Host:` header.

Usage
-----

```
$ hydra [config file]
```

With no configuration file it expects it to be in `/etc/hydra.conf`.

Building
--------

```
$ cd build
$ cmake ..
$ make
$ make install
```

At this point, there is a copy of the `hydra` daemon - `/usr/local/sbin/hydra`.

Developing
----------

Suggested build environment is Debian/Ubuntu Linux.

For Debian:

* `libboost-dev`
* `libboost-thread-dev`
* `libboost-system-dev`
* `cmake`
* `git`
* `libssl-dev`
