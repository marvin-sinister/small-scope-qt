# small-scope-qt
GUI for [small-scope](https://github.com/marvin-sinister/small-scope) arduino oscilloscope

![alt tag](http://i.imgur.com/5QLERq9.png)

This is a QtCreator project containing software for [small-scope](https://github.com/marvin-sinister/small-scope) arduino shield.

The software is written and tested on Debian GNU/Linux. To compile on Debian, you will have to install the *libqt5serialport5-dev* package. The Qt is portable, and the serial library should be portable as well, so it should be possible to compile on other platforms.

Some of the features:
 * trigger mode (rising, falling, toggle)
 * free running mode (continuous)
 * trigger level select from software
 * trigger delay select from software
 * voltage refference select (internal, external, internal 1.1V)
 * measure at two positions and cursor location

Some of the features are only available when using with [small-scope shield](https://github.com/marvin-sinister/small-scope-electronics).
