# set remote target to OpenOCD Server on localhost port 3333
target remote localhost:3333

# config logging behaviors
set logging overwrite on
set logging redirect off
set logging file ./gdb.log
set logging on

# config some print formats
set print pretty on

# load sections from elf file
load

# show memory regions
info mem

#b Reset_Handler
#b main

#c
