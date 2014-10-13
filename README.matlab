
Matlab interface to pfstools can be found in src/matlab. It contains
both .m and mex functions, which need to be compiled before can be
used. Once pfstools is properly installed, you can browse help:

doc pfstools_matlab
or
cd <dir_with_m_and_mex_files>
doc ./Contents.m

If you have problems running some functions, you can execute (from matlab):

pfs_test_shell

to diagnose for common problems.

Follow the instructions below to install matlab interface to pfstools.

Linux installation
===================

* Run './configure' then 'make install' as for ordinary installation

'configure' script may fail to detect matlab support. The most common
problem is missing or wrong matlab "mex" compiler in the PATH. On some
Linux distributions matlab "mex" command can be replaced with octave
mex command . In such case, you have to explicitly specify mex path by
passing --with-mex=<path-to-matlab-mex> to the 'configure' script.

* Add directory INSTALLDIR/share/pfstools/pfstools_matlab to matlab path
  (File->Set Path).

Windows installation
===================

Under Windows you have to invoke NMAKE file manually. From ordinary DOS
shell (not cygwin), cd to src/matlab, then execute:

(path to your matlab instalation)/matlab/R2006b/bin/mex.bat -setup

mex.bat will ask to choose a configuration. Choose the one compatible
with your Visual Studio installation.

then execute:

NMAKE -f Makefile.win32

We tested compilation on Win32 with VS C++ compiler. cygwin and
pfstools must be installed. Then include this directory in matlab's
path (File/SetPath in maltab IDE menu). You may need to modify
pfs_shell() function that should return the command line for executing
'bash' from DOS shell.

Many matlab pfs_* functions need to execute shell functions, for
example pfsin. To make sure that they can be executed from matlab, all
environmental variables must be set. Currently this is done by the
pfs_shell function, which extends command line so that pfs* commands
are executed from bash (assuming that bash sets all necessary
environmental variables in .bashrc). If bash is not your default
shell, you may need to change this.

Good luck.


If no good-luck, then below is a loosely written trouble shooting:

1. From matlab, execute 'pfs_test_shell'. The function will perform a few tests
for the most common pfstools/matlab setup problems and will suggest the most likely
solution.

2. You can select a compiler for mex files using "mex -setup"

If you have more than one compiler, for instance you owe a Visual
Studio, then try various compilers. Sometimes things work with VC 8.0,
but not with VC 6.0, and sometimes the other way around.

3. If you want to compile with Visual Studio, use the "Visual Studio
200X Command Prompt" DOS shell.

Especially if "nmake" doesn't seem to be on your system. 

4. All supplied Mex files do not depend on any library. So if you have
a problem within Matlab which looks like "The specified module could
not be found." it's not related to shared libraries. But it maybe
related to incompatible Visual Studio libraries, see point 1.

5. If you got everything running and type "pfsview(my_image)" and nothing shows up:

Check if X-Win32 or other X server is running, type "pfs_shell" in
Matlab and check if path is correct (and possibly adjust the path in
pfs_shell.m). Add a line to $HOME/.bash_login containing

"export DISPLAY=127.0.0.1:0"

6. Under Windows, if your Cygwin installation is not in c:\cygwin set Windows global variable:

CYGWIN_HOME='c:\MyDirectory\cygwin'

Of course, c:\MyDirectory should be replaced with a right path.


Known problems:

* Under Windows shell window flashes each time pfsput / pfsget command
  is executed

* pfs channel tags are not written to pfs-streams. No channel
  specific tags are written properly.

* Some file handles may not be closed properly.
