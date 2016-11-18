#Project HonokaMiku
Universal Love Live! School Idol Festival game files decrypter.  
This is original, implementation of HonokaMiku following C++03 standard. The source code is located in `src` folder.

Basically it allows decryption of LL!SIF game files so that it can be opened without using LL!SIF.

#File decryption support
HonokaMiku supports decryption of these game files and formats.

Game Server | V1   | V2   | V3   
----------- | ---- | ---- | -----
SIF EN/WW   | x    | x    | x
SIF JP      | x    | x    | x
SIF TW      | x    | x    | 
SIF CN      | x    | x    | x

It is also possible to decrypt Playground game files not from LL!SIF if you have the MD5 prefix key.

#Embedding in Application
HonokaMiku is designed as an API at first, so embedding should be easy and straightforward.
