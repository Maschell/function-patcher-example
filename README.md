# Mario Kart 8 decrypted Logging

Hook in the RC4 encryption/decryption that Mario Kart 8 uses for encryption/decryption online traffic. (Acutal UDP game data is not encrypted with RC4, but the matchmaking etc.)  
I throw this together in a short time, for devs only.  
Many hard coded values, only for Mario Kart 8 EUR v64. ugly, unreadable code for a quick PoC.  
The MiiVerse connections can be hooked via NSSLRead/NSSLWrite hooks (not done here because I'm tired).
[...]

# Credits
Chadderz - made the first tools including the first version of the "function patcher". Still using his "magic" defines ;)  
A big thanks goes out to <b>dimok</b> for creating the HBL, the dynamic libs, kernel stuff and every other stuff he made. He helped me a lot when I tried to patch dynamic functions. Many parts of the code are based on his code.
