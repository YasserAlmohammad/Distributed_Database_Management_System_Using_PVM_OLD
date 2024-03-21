you must copy dbCluster.exe, dbEngine.exe to PVM_ROOT/bin/Win32 directory to make it available
to PVM, or any directory PVM can detect executables in.


to run the client first you mush run switcher.exe and activate a cluster or any way you
can activate clusters in.


if you run dbCluster.exe as stand alone, you must use PVM Console to terminate it.


if you exited a cluster while a client is using it, the client program will lock.


use switcher.exe to check cluster status if anything went wrong.


to simualte network cluster comunication run simulateNodes.exe and define a temorary directory
as input like:( C:\ )
a valid database file must already exist in the same directory as simultorNodes.exe exists
with the name : source.txt, this file will be used to duplicate several databases neccessary
for the siulation.


the source code here is portable, can run on any c++ compiler, on a system runing PVM program
you need to link with( wsock32.lib, libpvm3.lib)
