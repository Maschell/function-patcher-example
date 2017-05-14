# Mario Kart 8 traffic research tool

What this tool currently does:  
  
- log the (encrypted) sendto/recvfrom traffic  
- hook into the RC4 encryption/decryption function of Mario Kart 8.  
  
The hook into the Mario Kart 8 RC4 function, give you some decrypted traffic of the game. Other parts are probably encrypted/decrypted somewhere else.  

Caution:  
I throw this together in a short time, the code is targeted for devs.  
Many hard coded values, work with the EUR Version of Mario Kart 8 with the newst update installed (v64)  
Unpolished code for a quick PoC.  

TODO:  
- Find a way to decrypt the P2P traffic.  

# Usage
Replace the IP in the main.cpp with Ip of your Computer and compile it.  
Load the .elf via the [Homebrew Launcher](https://github.com/dimok789/homebrew_launcher).  
The data will be logged to UDP Port 4405. You can use the [UDP Debug Reader](https://github.com/dimok789/loadiine_gx2/tree/master/udp_debug_reader)

# Credits
Based on the function-patcher.  
Chadderz - made the first tools including the first version of the "function patcher". Still using his "magic" defines ;)  
A big thanks goes out to <b>dimok</b> for creating the HBL, the dynamic libs, kernel stuff and every other stuff he made. He helped me a lot when I tried to patch dynamic functions. Many parts of the code are based on his code.
