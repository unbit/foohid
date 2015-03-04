# foohid
OSX IOKit driver for implementing virtual HID devices from userspace

Quick start
===========

Install https://github.com/unbit/foohid/releases/download/0.1/foohid.pkg and reboot your system.

The kext will expose a IOUserClient allowing you to create and manage virtual HID devices.

Use the official python2 wrapper (available at https://github.com/unbit/foohid-py) to start playing with virtual hid devices:

```sh
pip install foohid
```

(will install the foohid extension).

Now clone the foohid-py repository

```sh
git clone https://github.com/unbit/foohid-py
```

where 2 tests are available:

test_mouse.py will create a virtual mouse. Just run it, and every second your mouse pointer will move to a random position

test_list.py will show the listing feature (a bunch of virtual devices will be created, listed and destroyed)

Note: python3 support should be ready soon


The IOUserClient api
====================

4 Methods are exposed:

* create (selector 0)
* destroy (selector 1)
* send (selector 2)
* list (selector 3)
