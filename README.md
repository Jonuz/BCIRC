BCIRC
=====

Basic IRC bot with plugin (and experimental python3) support, written with C as training project.  
Feel free to fork and ask for support from #bcirc @ Freenode.  

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
>URL title announce  
>Weather announcer  
>Multiserver support  
>C plugin support  
>Logging  


##Requiments
>gcc5 (Might work with gcc4)  
>libcurl-dev  
>libconfig-dev  
>POSIX compatible compiler  

If you are using python branch, then this too.  
>python3-dev  
