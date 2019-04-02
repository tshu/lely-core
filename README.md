![logo](doc/logo.png)

Lely core libraries
===================

Introduction
------------

### C11 and POSIX compatibility library (liblely-libc)

liblely-libc makes a selection of C11 features available in C99 (and C++98) and implements certain C99 and POSIX features that may be missing on some platforms.
The library is intended to provide platform independence to C99- and
C11-compliant users.

See [doc/libc/overview.md](@ref md_doc_libc_overview) for an overview of the provided functionality.

### Test Anything Protocol (TAP) library (liblely-tap)

liblely-tap provides an implementation of the [Test Anything Protocol].

### Utilities library (liblely-util)

liblely-util provides a variety of functions and data structures used by, and
shared between, other Lely libraries.

See [doc/util/overview.md](@ref md_doc_util_overview) for an overview of the provided functionality.

### CAN library (lely-can)

lely-can provides generic CAN frame definitions, conversion functions for
different drivers and an interface designed to simplify CAN frame handling.

See [doc/can/overview.md](@ref md_doc_can_overview) for an overview of the
provided functionality.

### I/O library (liblely-io)

liblely-io provides uniform, platform-independent I/O interface for CAN devices,
regular files, serial I/O devices and network sockets. Depending on the
platform, some or all of these devices can be polled for events, allowing the
use of the reactor pattern.

See [doc/io/overview.md](@ref md_doc_io_overview) for an overview of the
provided functionality.

### Event library (liblely-ev)

liblely-ev provides an event loop and asynchronous promises and futures.

### Asynchronous I/O library (liblely-io2)

liblely-io2 provides asynchronous I/O. Currently only timers, signal handlers
and CAN devices are supported.

### CANopen library (liblely-co)

liblely-co is a CANopen implementation for both masters and slaves. Most of the
functionality of [CiA] 301, 302, 305, 306, 309 and 315 is provided. The
implementation is passive; it does not perform any I/O or timing by itself.
Instead it relies on the user to send and received CAN frames and update the
clock. This allows the library to be integrated into a wide variety of
frameworks and communication patterns. The library uses the platform-independent
CAN frame definition from the Lely CAN library (liblely-can) and can be used
with the Lely I/O library (liblely-io).

Additionally, several tools are provided:
- can2udp - CAN to UDP forwarding tool
- cocat/cocatd - CANopen cat tools
- coctl - CANopen control tool
- dcf2c - CANopen EDS/DCF to C conversion tool

See [doc/co/overview.md](@ref md_doc_co_overview) for an overview of the
provided functionality.

### C++ CANopen application library (liblely-coapp)

liblely-coapp provides a high-level application interface in C++ for liblely-co.
It is intended to simplify the development of CANopen master applications (and,
to a lesser extent, slaves) by providing a driver model and combining it with an
event loop (from liblely-ev).

Download
--------

lely-core is hosted on [GitLab]. You can clone the repository with

    $ git clone https://gitlab.com/lely_industries/lely-core.git

or download the latest release
([v1.9.2](https://gitlab.com/lely_industries/lely-core/tags/v1.9.2)).

Build and install
-----------------

lely-core uses the GNU Build System (`configure`, `make`, `make install`) and
has native support for POSIX platforms (Debian-based Linux distributions in
particular) and Windows (Vista and higher) through [Cygwin] or [MinGW-w64].
[GCC ARM Embedded] and Microsoft Visual Studio (2013 and higher) are also
supported, but project files will have to be created by hand.

The release archive includes the build system, but if you want to build a
checkout of the repository, you need to install the autotools (autoconf,
automake and libtool). The build system can then be generated by running

    $ autoreconf -i

in the root directory of the repository.

If you want to generate HTML documentation from the source code, you need to
install [doxygen] and [Graphviz]. Building the [Python] bindings requires a
Python 2.x and/or Python 3.x interpreter as well as a [Cython] compiler.

### Debian packages

For Debian-based Linux distributions, the preferred installation method is to
generate and install Debian packages. Install the Debian package build system
with

    $ sudo apt-get install cdbs devscripts

The packages can now be built by running

    $ debuild -uc -us -b

from the root directory of the project. Once the packages are generated, you can
clean up the project directory with

    $ debuild clean

debuild creates the packages in the parent directory of the project. Install
them with

    $ cd ..
    $ sudo dpkg -i liblely-*.deb

The Python bindings can be installed with

    $ sudo dpkg -i python-lely-can*.deb

and/or

    $ sudo dpkg -i python3-lely-can*.deb

depending on which Python version is available.

### configure, make, make install

It is also possible to build and install the library by hand. First, configure
the build system by running

    $ ./configure

from the root directory of the project.

The `configure` script supports many options. The full list can be shown with

    $ ./configure --help

Once the build system is configured, the library can be built with

    $ make

The test suite can be run with

    $ make check

Run

    $ make html

to generate the HTML documentation from the source code. The binaries, headers
and documentation can be installed by running

    # make install

as root.

### Options

liblely-libc provides an implementation of the C11 multithreading support
library. Multithreading support can be disabled by providing the
`--disable-threads` option to `configure` or by defining the `LELY_NO_THREADS`
preprocessor macro. The implementation is based on pthreads. When building the
library without the autotools build system, define the `LELY_HAVE_PTHREAD`
preprocessor macro if pthreads is available.

On POSIX and Windows platforms, liblely-util provides functions to run a process
in the background as a daemon/service. This functionality can be disabled with
the `--disable-daemon` option to `configure` or by defining the `LELY_NO_DAEMON`
preprocessor macro.

The CAN library provides a generic CAN message type suitable for both CAN and
CAN FD frames. If CAN FD support is not required, the message size can be
reduced by specifying the `--disable-canfd` option to `configure` or by defining
the `LELY_NO_CANFD` preprocessor macro.

Much of the functionality of CANopen is optional and can be disabled to save
space on embedded devices. liblely-co supports the following `configure` options
(or preprocessor macros) to disable certain features:

* `--disable-dcf` (`LELY_NO_CO_DCF`):
  disable EDS/DCF support.
* `--disable-obj-file` (`LELY_NO_CO_OBJ_FILE`):
  disable UploadFile/DownloadFile support for the object dictionary.
* `--disable-obj-limits` (`LELY_NO_CO_OBJ_LIMITS`):
  disable minimum/maximum values in the object dictionary.
* `--disable-obj-name` (`LELY_NO_CO_OBJ_NAME`):
  disable names in the object dictionary.
* `--disable-sdev` (`LELY_NO_CO_SDEV`):
  disable static device description support.
* `--disable-csdo` (`LELY_NO_CO_CSDO`):
  disable Client-SDO support (also disables master support).
* `--disable-rpdo` (`LELY_NO_CO_RPDO`):
  disable Receive-PDO support.
* `--disable-tpdo` (`LELY_NO_CO_TPDO`):
  disable Transmit-PDO support.
* `--disable-sync` (`LELY_NO_CO_SYNC`):
  disable synchronization (SYNC) object support.
* `--disable-time` (`LELY_NO_CO_TIME`):
  disable time stamp (TIME) object support.
* `--disable-emcy` (`LELY_NO_CO_EMCY`):
  disable emergency (EMCY) object support.
* `--disable-lss` (`LELY_NO_CO_LSS`):
  disable Layer Setting Services (LSS) and protocols support.
* `--disable-wtm` (`LELY_NO_CO_WTM`):
  disable Wireless Transmission Media (WTM) support.
* `--disable-master` (`LELY_NO_CO_MASTER`):
  disable master support.
* `--disable-gw` (`LELY_NO_CO_GW`):
  disable gateway support.
* `--disable-gw-txt` (`LELY_NO_CO_GW_TXT`):
  disable ASCII gateway support.

Additionally, master and/or slave support for the C++ CANopen application
library can be disabled with the following `configure` options (or preprocessor
macros):

* `--disable-coapp-master` (`LELY_NO_COAPP_MASTER`):
  disable C++ CANopen application master support.
* `--disable-coapp-slave` (`LELY_NO_COAPP_SLAVE`):
  disable C++ CANopen application slave support.

The following preprocessor macros can be defined to change the default timeouts
used by an NMT master when booting a slave:

* `LELY_CO_NMT_TIMEOUT`: the default timeout (in milliseconds) for SDO requests
  issued by an NMT master (default: 100). The actual timeout can be changed at
  runtime with co_nmt_set_timeout().
* `LELY_CO_NMT_BOOT_WAIT_TIMEOUT`: the timeout (in milliseconds) before an NMT
  master tries to boot a slave on error status B (default: 1000, see Fig. 4 in
  CiA 302-2 version 4.1.0).
* `LELY_CO_NMT_BOOT_SDO_RETRY`: the number of times an NMT master retries an SDO
  request on timeout (default: 3). This is used for SDO requests that may occur
  right after a slave receives a reset node or reset communication command,
  which might cause it to miss the request.
* `LELY_CO_NMT_BOOT_RTR_TIMEOUT`: the timeout (in milliseconds) after an NMT
  master sends a node guarding RTR to check the NMT state of a slave during
  booting (default: 100, see Fig. 9 in CiA 302-2 version 4.1.0).
* `LELY_CO_NMT_BOOT_CHECK_TIMEOUT`: the time (in milliseconds) between
  successive checks of the flash status indication (1F57:01) or program control
  (1F51:01) sub-object of a slave during booting (default: 100, see Fig. 3 in
  CiA 302-3 version 4.1.0).

Although the Lely core libraries are written in C, C++ interfaces are provided
for a subset of the functionality. These interfaces can be disabled with the
`--disable-cxx` option to `configure` or by defining the `LELY_NO_CXX`
preprocessor macro. This also disables the entire C++ CANopen application
library.

The Python bindings can be disabled by specifying the `--disable-python` option
to `configure`.

Documentation
-------------

Doxygen documentation for the latest development version can be found at
http://lely_industries.gitlab.io/lely-core/doxygen/.

Licensing
---------

Copyright 2013-2019 [Lely Industries N.V.]

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

[Cygwin]: https://www.cygwin.com/
[Cython]: http://cython.org/
[doxygen]: http://www.doxygen.org/
[GCC ARM Embedded]: https://launchpad.net/gcc-arm-embedded
[GitLab]: https://gitlab.com/lely_industries/libc
[Graphviz]: http://www.graphviz.org/
[Lely Industries N.V.]: http://www.lely.com
[MinGW-w64]: http://mingw-w64.org/
[Python]: https://www.python.org/
[Test Anything Protocol]: https://testanything.org
