# nn::nex traffic research tool

What this tool currently does:  
  
- log the (encrypted) sendto/recvfrom traffic  
- hook into the RC4 encryption/decryption and log the buffer.  

 The hook into the RC4 function, give you the input/out of the decryption/encrypting used by the game for network payload data.  

Currently the following games are supported:  
    - Mario Kart 8 (EUR,v64)  
    - Donkey Kong Tropical Freeze (EUR,v17)  

Caution:  
I threw this together in a short time, the code is targeted for devs.  
Many hard coded values and a a unpolished code for a quick PoC.  

# Usage
Replace the IP in the main.cpp with Ip of your Computer and compile it.  
Load the .elf via the [Homebrew Launcher](https://github.com/dimok789/homebrew_launcher).  
The data will be logged to UDP Port 4405. You can use the [UDP Debug Reader](https://github.com/dimok789/loadiine_gx2/tree/master/udp_debug_reader)

# Credits
Based on the function-patcher.  
Chadderz - made the first tools including the first version of the "function patcher". Still using his "magic" defines ;)  
A big thanks goes out to <b>dimok</b> for creating the HBL, the dynamic libs, kernel stuff and every other stuff he made. He helped me a lot when I tried to patch dynamic functions. Many parts of the code are based on his code.
