xHacking
========

xHacking is a hacking oriented library. 
It provides useful functions for cracking, reverse engineering and hacking.

A brief schematic of the library is:

Utilities
---------
Some useful functions to be used on target processes

> +CreateConsole

> +GetModuleInfo

> +FindPattern

> +UnlinkModuleFromPEB

> +RelinkModuleToPEB



Loader
------
Easily loads or waits for a DLL to be loaded. Asynchronous callbacks fire up when the DLL is available.

> +Wait

> +Load



Detour
------
Allows to create detours, codecaves and memory exceptions (PAGE_GUARD breakpoints/codecaves) in the target memory.

> +Wait (Depends on *Loader*)

> +Load (Depends on *Loader*)

> +operator()(Args...), operator()(), <T>operator()()

> +Length

> +WithTrampoline

> +Type

> +State

> +Commit

> +Restore



API
---
Allows to indirectly call APIs. Can also fake calls return addresses.

> +operator()(Args...)

> +Fake

> +LoaderMode

> +AutoFakeRange (Depends on *Utilities*)

> +AutoFakeWithinModule (Depends on *Utilities*)

> +AutoFake (Depends on *Utilities, Loader*)

> +AutoLoad (Depends on *Utilities, Loader*)



*NOTE: Only x86 support yet!*
