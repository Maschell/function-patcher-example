# Running a thread in background
This repo contains a example for always running in a thread in background.

- It requires kernel access.
- The thread is defined in main.cpp "mainThread"

This version has splitted the actual patcher and the functions that will be patched in different files. 
The actual function patcher can be splitted into mutiple files. Check out the "patcher" folder for an example.

If you have any question, feel free to open an issue.

# Building
In order to build this application, you need the custom liboGC and portlibs modified/created by dimok. You can find them on the loadiine_gx2 repo (https://github.com/dimok789/loadiine_gx2/releases/tag/v0.2). Simply put the files in your devkit folder and run the Makefile. 

# Credits
Chadderz - made the first tools including the first version of the "function patcher". Still using his "magic" defines ;)  
A big thanks goes out to <b>dimok</b> for creating the HBL, the dynamic libs, kernel stuff and every other stuff he made. He helped me a lot when I tried to patch dynamic functions. Many parts of the code are based on his code.

