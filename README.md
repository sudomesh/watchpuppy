
WORK IN PROGRESS. NOTHING WORKS YET.

This is a utility to activate, configure and deactivate hardware watchdogs on e.g. ar91xx chipsets. This only works on watchdogs supporting the ["Magic Close" feature](https://www.kernel.org/doc/Documentation/watchdog/watchdog-api.txt)

Instead of having the entire watchdog daemon written in C (making it less flexible) this utility let's you write your watchdog daemon as e.g. a shell or lua script and then simply call the watchpuppy at the biginning of the script to initialize the watchdog.

# Usage

```
watchpuppy start|stop|status|pat 
  -d device: Device file to open (default: /dev/watchdog)
  -t timeout: Reboot after this many seconds with no pat (default: 30)
```

The pat argument resets the watchdog reboot counter. You can also pat the watchdog by simply writing to the watchdog device like so:

```
echo "p" > /dev/watchdog
```

Just don't write a V character as that will deactivate the watchdog.

# How to write a daemon

See example.sh for a daemon that uses watchpuppy.

# Wishlist

* A way to check if last reboot was triggered by the watchdog

# License and Copyright

Copyright 2015 Marc Juul <marc@juul.io>

License: GPLv3
