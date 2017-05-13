# logging hooks

hooks into all kind of logging methods and log them via the UDP logger.  
Caution: Before you start or end an application, press the right stick to disable the logging until a new app it started. Otherwise the console will softlock

#Building#
In order to build this application, you need the custom liboGC and portlibs modified/created by dimok. You can find them on the loadiine_gx2 repo (https://github.com/dimok789/loadiine_gx2/releases/tag/v0.2). Simply put the files in your devkit folder and run the Makefile. 

#Credits#
Chadderz - made the first tools including the first version of the "function patcher". Still using his "magic" defines ;)  
A big thanks goes out to <b>dimok</b> for creating the HBL, the dynamic libs, kernel stuff and every other stuff he made. He helped me a lot when I tried to patch dynamic functions. Many parts of the code are based on his code.

