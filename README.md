# *TelnetServer* - A telnet server implementation for Arduino

*TelnetServer* is an easy to use Telnet server implementation which allows you for instance to attach a command line to it (see [StreamCmd](../StreamCmd)). This way you get a remote terminal to your machine in a minute - at least for your LAN -- WAN use is strongly discouraged if you have security concerns.

*TelnetServer* is currently experimental and available for ESP8266 only. But feel free to try it out and provide us with some feedback if you get it running on different platforms.

Here's an example an actual interface in action:
```
uli@ankerklause:~$ telnet ig-archas.local
Trying 192.168.1.30...
Connected to ig-archas.local.
Escape character is '^]'.
Welcome to the Intelli-Güss telnet interface!
Copyright (c) 2017 Elektronik Workshop
Type "help" for available commands
Please enter password for "ig-archas": ******
ig-archas> hist
----
history clean
ig-archas> c.info 1
on-board circuit [1]:   on
            pump time   30 s
           dry thresh  180
           wet thresh  230
            soak time    5 m
     reservoir thresh  150
----------------------------
   last read humidity  208
accumulated pump time  30 s
                state  idle
           iterations  0
ig-archas>
```

There are some nice [examples](#examples) which will help you to set up your own Telnet-based command line in just a few minutes.

## Design notes
### Todos
* Test on AVR, SAM and SAMD architectures - volunteers?
* Support for minimal negotiation sequences such that the terminal doesn't show the password, when we're in authentication mode

## Installation
### Arduino IDE
1. Download the ZIP file (below) to your machine.
2. In the Arduino IDE, choose Sketch/Include Library/Add Zip Library
3. Navigate to the ZIP file, and click Open

--- or ---

In the Arduino IDE, choose Sketch/Include Library/Manage Libraries.  Click the TelnetServer Library from the list, and click the Install button.

## Compatible Hardware
No hardware dependencies.

## Examples
The library includes several examples to help you get started. These are accessible in the Examples/TelnetServer menu off the File menu in the Arduino IDE.
* **[SimpleTelnetCli](examples/SimpleTelnetCli/SimpleTelnetCli.ino):** A very simple Telnet server implementation which you could just copy past to add your own Telnet command line interface to your project in a few seconds.
* **[TelnetPinManipulator](examples/TelnetPinManipulator/TelnetPinManipulator.ino):** A more complex version which allows you to remotely manipulate on your board via Telnet.

---
Copyright (c) 2017 [Elektronik Workshop](http://elektronikworkshop.ch)
