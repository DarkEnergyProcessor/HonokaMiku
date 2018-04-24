HonokaMiku
==========
Universal Love Live! School Idol Festival game files decrypter.  
This is original implementation of HonokaMiku following C++03 standard. The source code is located in `src` folder.  
The code is designed to be cross-platform as possible, so it should compile under Windows, Linux (incl. Android), Mac OS X, and other platforms.

Basically it allows decryption of LL!SIF game files so that it can be opened without using LL!SIF.

File decryption support
=======================
HonokaMiku supports decryption of SIF EN/WW, JP, TW, and CN game files, from version 1 encryption format to version 4 encryption format.

It is also possible to decrypt Playground game files not from LL!SIF if you have the MD5 prefix key by using the API.

Documentation
=============

Documentation? `doxygen`

Embedding in Application
========================
HonokaMiku is designed as an API at first, so embedding should be easy and straightforward.

Just add `DecrypterContext.h`, `md5.h`, and all `*.cc` (except `HonokaMiku.cc`) files in `src` folder to your project and you're done.

Implementation In Other Languages
=================================
Did you rewrite HonokaMiku to other languages? Send me your work and I'll add it in here.

