#!/bin/bash

socat TCP-LISTEN:1337,fork,reuseaddr,bind=0.0.0.0 EXEC:"./challenge.py",pty,ctty,echo=0
