# SwipSwapMe
This little app can be used to swap the TV and DRC screen.

## Usage
Start SwipSwapMe with the [Homebrew Launcher](https://github.com/dimok789/homebrew_launcher).  
When starting the app, you have the option to set your own button combo. Just press the button combo you want for 2 seconds.  
Otherwise it will use the default settings (TV button).
  
Press the TV button  (or your button combo) on the Gamepad to swap the screens. Thats all.

# Building
In order to build this application, you need the custom liboGC and portlibs modified/created by dimok. You can find them on the loadiine_gx2 repo (https://github.com/dimok789/loadiine_gx2/releases/tag/v0.2). Simply put the files in your devkit folder and run the Makefile. 

# Credits
OatmealDome / Yahya14 - for the initial idea.
Chadderz - made the first tools including the first version of the "function patcher". Still using his "magic" defines ;)
A big thanks goes out to <b>dimok</b> for creating the HBL, the dynamic libs, kernel stuff and every other stuff he made. He helped me a lot when I tried to patch dynamic functions. Many parts of the code are based on his code.

