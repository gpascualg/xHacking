xHacking
========

xHacking is a hacking oriented library. 
It provides useful functions for cracking, reverse engineering and hacking.

A brief schematic of the library is:

Utilities
---------
> CreateConsole
> GetModuleInfo
> FindPattern
> UnlinkModuleFromPEB
> RelinkModuleToPEB

Loader
------
> Wait
> Load

Detour
------
> Wait (Depends on *Loader*)
> Load (Depends on *Loader*)
> operator()(Args...), operator()(), <T>operator()()
> Length
> WithTrampoline
> Type
> State
> Commit
> Restore

API
---
> operator()(Args...)
> Fake
> LoaderMode
> AutoFakeRange
> AutoFakeWithinModule
> AutoFake
> AutoLoad

*NOTE: Only x86 support yet!*
