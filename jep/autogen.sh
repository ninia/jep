#!/bin/sh
aclocal && autoconf && automake && ./config.status --recheck
