# Purpose
WinMSR is a Windows 64-bits driver which provides access to the cpuid instruction and the msr registers in the processor ring 0. 

This example returns the temperature of the Intel i7 Processor Cores. 

# Prerequisites

## Open Source IDE

1- [GCC MinGW](http://sourceforge.net/projects/tdm-gcc) 64-bit compiler  
2- [Code::Blocks](http://www.codeblocks.org) _standalone version_  
3- [DebugView](http://technet.microsoft.com/en-us/sysinternals/bb896647)  

# Source Code

* driver.c _The kernel driver source code_  
* WinMSR.cbp _The Code::Blocks project file_  
* WinMSR.reg _The registry setup_  

# Installation
* Once built, copy the device driver into the Windows drivers directory
```
copy WinMSR.sys C:\Windows\System32\drivers\
```

## Registry
* Load the WinMSR.reg file into the registry

## Debug
* Load the "Debug Print Filter.reg" file into the registry to enable ```DbgPrint()```

## Debug Print Filter.reg
 Windows Registry Editor Version 5.00
 
[HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Session Manager\Debug Print Filter]
 "DEFAULT"=dword:00000008
 

# Reboot
## F8 Key
* When booting Windows press the F8 key then choose the option to disable the driver signature verification

## Execute WinMSR

1- Start DebugView : _choose the option [Capture Kernel]_  

2- Run a command prompt  

* Start the service
```
net start WinMSR
```  

* Stop the service
```
net stop WinMSR
```
![](http://cyring.free.fr/images/WinMSR-CoreTemp.JPG)
