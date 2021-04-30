# 2021-04-30: Threads and flow control

Right now, we have a handful of primary modules that make telephony work.

* phony: Provides application layer interface to telephone services
* phony_hid: USB HID state transitions with firmware
* stitch: Audio device tunneling (e.g., Host mic to Telphone output and
  Telephone input to Host speakers)

This design is intended to make it easy to bring up a telephone client and
manage / track it's changing state over time.

There are some questions that are arising from this implementation.

## Who manages threads, when and why?

The phony module creates a thread for it's HID client, but the stitch
service creates it's own thread for audio devices.

I'm under the impression that a library shouldn't really get into the
business of creating and managing threads because various applications 
manage threads with different schemes (e.g., GTK apps use gthread, Posix 
uses pthread and Windows has it's own thing.

It's not clear to me how the phony module could be expected to work at all 
without initiating at least 3 threads.

1) HID communications
2) Stitch outbound pipe
3) Stitch inbound pipe

## How to stub/mock services for testing?
I'm also struggling to understand how to mock the external services for a C 
application. My tests are able to cover some basic logical flows, but they 
are nowhere near comprehensive enough because I cannot yet fake the libusb 
HID connections or the soundio audio device interfaces.

I believe it's just some work I need to to, in order to create fake versions 
of these interfaces.
