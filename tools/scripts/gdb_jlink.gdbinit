## set remote target to JLinkGDBServer on localhost port 2331
target remote localhost:2331

## log gdb process
set logging file ./gdb.log
set logging on

## no confirm when quit
#set confirm off

## configure some print formats
set print pretty on
set print array on

## save back trace
define save-bt
    if $argc != 1
        help save-bt
    else
        set logging file $arg0
        set logging on
        set logging off
    end
end
document save-bt
Usage: save-bt ./gdb_bt.log
end

## save break pointers
define save-bp
    if $argc != 1
        help save-bp
    else
        save breakpoints $arg0
    end
end
document save-bp
Usage: save-bp ./gdb_bp.rec
end

## reload break pointer
define reload-bp
    if $argc != 1
        help reload-bp
    else
        source $arg0
    end
end
document reload-bp
Usage: source ./gdb_bp.rec
end

## print ARM core registers in all mode
monitor regs

monitor speed 5000
monitor endian little
monitor reset

## load sections from elf file
load

## show memory regions
info mem

## set break pointer
# b Reset_Handler
# b main

## free run until a break pointer
c
