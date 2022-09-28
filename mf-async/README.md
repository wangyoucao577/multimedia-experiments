# mf-async      

Refer to Microsoft's [Writing an Asynchronous Method - square root example](https://learn.microsoft.com/en-us/windows/win32/medfound/writing-an-asynchronous-method) to implement the async method for async calling, which will be dispatched to work queue for real calculation internally.        
See more details in    
- [Calling Asynchronous Methods](https://learn.microsoft.com/en-us/windows/win32/medfound/calling-asynchronous-methods)
- [Implementing the Asynchronous Callback](https://learn.microsoft.com/en-us/windows/win32/medfound/implementing-the-asynchronous-callback)
- [Writing an Asynchronous Method](https://learn.microsoft.com/en-us/windows/win32/medfound/writing-an-asynchronous-method)

## Build        
```PowerShell
$ mkdir -p build && cd build
$ cmake .. -A x64

# optionlly open Visual Studio to build and run
$ cmake --build . 
```

