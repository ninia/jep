#!/bin/sh
automake -a
aclocal && autoconf && automake && ./config.status --recheck
