# ffmpeg-tests
Test codes based on `ffmpeg`(i.e., `libav*`).       

## Build

### Windows    
Latest [ninja](https://ninja-build.org/), [cmake](https://cmake.org/) are mandantory in your `PATH`.      
Then Install Latest [Microsoft Visual Studio](https://visualstudio.microsoft.com/) with `llvm/clang` enabled.      
After that, set env `$ENV:VS20<XX>_INSTALL_PATH`, for instance,              

```PowerShell
# add $ENV:VS2019_INSTALL_PATH into your windows environment variables, 
# possible value maybe "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools" or "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community"
PS C:\> $ENV:VS2019_INSTALL_PATH
```    

- Build via `ninja`       

```PowerShell
# Enable vs dev shell env for building
PS C:\> Import-Module ($ENV:VS2019_INSTALL_PATH + "\Common7\Tools\Microsoft.VisualStudio.DevShell.dll")
PS C:\> Enter-VsDevShell -VsInstallPath $ENV:VS2019_INSTALL_PATH -SkipAutomaticLocation -DevCmdArguments "-arch=x64 -host_arch=x64"

# use Ninja
$ mkdir -p build && cd build
$ cmake .. -GNinja -DVCPKG_TARGET_TRIPLET=x64-windows-static -DCMAKE_TOOLCHAIN_FILE:STRING=$ENV:VCPKG_CMAKE_PATH
$ cmake --build . -j
```    

- Build via `MSVC/ClangCL`       

```PowerShell
# Enable vs dev shell env for building
PS C:\> Import-Module ($ENV:VS2019_INSTALL_PATH + "\Common7\Tools\Microsoft.VisualStudio.DevShell.dll")
PS C:\> Enter-VsDevShell -VsInstallPath $ENV:VS2019_INSTALL_PATH -SkipAutomaticLocation -DevCmdArguments "-arch=x64 -host_arch=x64"

# use cmake to generate MSVC .sln, then open it
$ mkdir -p build && cd build
$ cmake .. -G "Visual Studio 16 2019" -A x64 -DVCPKG_TARGET_TRIPLET=x64-windows-static -DCMAKE_TOOLCHAIN_FILE:STRING=$ENV:VCPKG_CMAKE_PATH

# [Important] Then manually swtich to `llvm/clang` in Visual Studio
# In theory, `cmake .. -G "Visual Studio 16 2019" -A x64 -T ClangCL` should select `llvm/clang` automatically, but it doesn't work in my testing.     
```


## References
- [An ffmpeg and SDL Tutorial - How to Write a Video Player in Less Than 1000 Lines](http://dranger.com/ffmpeg/ffmpeg.html)    
- [TwinklebearDev SDL 2.0 Tutorial Index](https://www.willusher.io/pages/sdl2/)

