#!/bin/bash

make prod
python obfuscate.py
make final
