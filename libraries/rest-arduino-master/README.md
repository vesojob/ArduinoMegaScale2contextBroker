# CloudPlugs REST Client for Arduino

CloudPlugs REST Client for Arduino is a library to make it easier to interact with CloudPlugs servers from Arduino.

Official repository https://github.com/cloudplugs/rest-arduino

Documentation at https://docs.cloudplugs.com/rest-arduino/html

See the LICENSE file for more details.

Copyright 2014 CloudPlugs Inc. http://www.cloudplugs.com

## Dependencies

- Requires the new Ethernet library API (with DHCP and DNS) which is in Arduino 1.0 and later

## Installation

1. Download the latest version of the library from https://github.com/cloudplugs/arduino/releases and save the file somewhere
1. In the Arduino IDE, go to the Sketch -> Import Library -> Add Library... menu option
1. Find the zip file that you saved in the first step, and choose that
1. Check that it has been successfully added by opening the Sketch -> Import Library menu.  You should now see CloudPlugs listed among the available libraries.

## Usage

In normal usage, handles the outgoing request and Host header.  The returned status code is parsed for you, as is the Content-Length header (if present).

Because it expects an object of type Client, you can use it with any of the networking classes that derive from that.  Which means it will work with EthernetClient, WiFiClient and GSMClient.

See the examples for more detail on how the library is used.

