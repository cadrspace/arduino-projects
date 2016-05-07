# Smart lights

Remotely controlled LED lamp.  The project makes use of Arduino Mega
2560 and Wi-Fi shield (Adafruit CC3000).

## Building and uploading

```
$ MONITOR_PORT=/dev/ttyACM0 make -e upload
```

## Port monitor

GNU Screen is one of the tools that can be used to connect to an
Arduino port:

```
$ screen /dev/ttyACM0 115200
```

## License

`coap.cpp` and `coap.h` is taken from
[microcoap](https://github.com/1248/microcoap) project; `endpoint.cpp`
and `endpoint.h` are written using the code from microcoap as well.
See `LICENSE.txt` file for the license on those files.

The rest of the code is distributed under terms of GNU GPL v3 or later
version.  See `COPYING` file for the license text.
