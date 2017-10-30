# function patcher example
This repo contains a example for patching/hooking functions on the WiiU.  

- It requires kernel access.
- Static and dynamic functions are supported. It will try to guess the type, keep an eye on the [udp log](https://github.com/dimok789/loadiine_gx2/tree/master/udp_debug_reader)!  

This version has splitted the actual patcher and the functions that will be patched in different files. 
The actual function patcher can be splitted into mutiple files. Check out the "patcher" folder for an example.

If you have any question, feel free to open an issue.

# Building
In order to build this application you need serval libs:

- [dynamic_libs](https://github.com/Maschell/dynamic_libs/tree/lib) for access to the functions.
- [libutils](https://github.com/Maschell/libutils) for common functions.

Install them (in this order) according to their README's. Don't forget the dependencies of the libs itself.

# Credits
Chadderz - made the first tools including the first version of the "function patcher". Still using his "magic" defines ;)
A big thanks goes out to <b>dimok</b> for creating the HBL, the dynamic libs, kernel stuff and every other stuff he made. He helped me a lot when I tried to patch dynamic functions. Many parts of the code are based on his code.

