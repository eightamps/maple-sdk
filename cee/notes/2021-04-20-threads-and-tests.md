# 2021-04-30: Threads and flow control

Right now, we have a handful of modules related to telephony.

* phony: Provides application layer interface to telephone services
* phony_hid: USB HID state transitions with firmware
* stitch: Audio device tunneling (e.g., Host mic to Telphone output and
  Telephone input to Host speakers)

This design is intended to make it easy to bring up a telephone client and
manage it's state changes.

There are some questions that are arising from this implementation.

## Who manages threads, when and why?

The phony module creates a thread for it's HID client, but the stitch
service creates it's own thread.

I'm under the impression that a library shouldn't really get into the
business of creating an managing threads, but this could be a
misunderstanding. It's not clear to me how the phony module could be
expected to work at all without initiating at least 3 threads.

1) HID communications
2) Stitch outbound pipe
3) Stitch inbound pipe

## How to stub/mock services for testing?
