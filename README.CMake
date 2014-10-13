Is is strongly recommended to compile into out-of-source directory:

mkdir build
cd build
cmake ../
make 

If you encounter any problems during compilation, run the make again with the option:

make VERBOSE=1

or, if you use nmake on Windows:

nmake /S

and report the problem with the complete error message to the google discussion group:

https://groups.google.com/forum/#!forum/pfstools

======= Compiling with QT ==========

If Qt library is not found, you need to specify the path to qmake exacutable

cmake -D "QT_QMAKE_EXECUTABLE=/usr/bin/qmake" ../

