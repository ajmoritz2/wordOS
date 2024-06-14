#!/bin/sh
./clean.sh
echo --------------------------- CLEANED UP SYSTEM ---------------------------------
make
./iso.sh
./clean.sh
echo ---------------------------- SYSTEM STARTING -----------------------------------
bochs
