yubiswitch-for-linux
====================

Simple program to enable/disable Yubikeys on Linux

To build and install:
---------------------

$ gcc -g -Wall -o yubiswitch main.c
# install -o root  -g root -m 4755  yubiswitch /usr/local/bin/

(yes, unfortunately it needs to be setuid root)


Usage
-----

$ yubiswitch
Usage: yubiswitch {on|off|list|debug}

$ yubiswitch list
Found Yubikey at 1-2:1.0
Yubikey is not bound to usbhid

$ yubiswitch on
binding Yubikey to usbhid driver at 1-2:1.0

$ yubiswitch off
Unbinding Yubikey from usbhid driver at 1-2:1.0


Todo
----

Perhaps an option to turn the yubikey on for a fixed length of time
then off when that time expires. 

Or to watch the yubikey and turn it off immediately after it's used?

Perhaps a deamon option to automatically turn it on when a message is
received and off after a timeout?

