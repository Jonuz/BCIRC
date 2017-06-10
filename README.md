BCIRC
=====

Basic IRC bot with plugin (and experimental python3) support, written with C as training project.  

##How to build
In BCIRC directory:

>mkdir plugins/bin  
>source exports.sh  
>git submodule init  
>git submodule update  
>sh build.sh  

If you wanna recompile plugin, go to plugins/src/\<plugin\> and do make clean and make.
Remember that BCIRC's enviroment variables has to be set, to do it run source exports.sh in BCIRC's root directory.


##Features
>URL title announcing  
>Weather announcer  
>Multiserver support  
>C plugin support  
> Python script support (disabled by default & might be buggy)
>Logging  


##Requiments
>gcc5 (Might work with gcc4)  
>libcurl-dev  
>libconfig-dev  

If you are using python branch, then this too.  
>python3-dev  
