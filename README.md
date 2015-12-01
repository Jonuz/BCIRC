BCIRC
=====

Basic IRC bot with plugin support, written with C as training project.


##How to build
In BCIRC directory:

>mkdir plugins/bin  
>source exports.sh  
>sh build.sh  

If you wanna recompile plugin, go to plugins/src/\<plugin\> and do make clean and make.
Remember that BCIRC's enviroment variables has to be set, to do it run source exports.sh in BCIRC's root directory.


##Requiments
>gcc5 (Might work with gcc4.x, never tested thought)  
>libcurl-dev  
>libconfig-dev  
>POSIX compatible OS
