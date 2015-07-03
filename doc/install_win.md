
#Installation instructions for Windows

##Dependencies

First, we need to install some software:

* Download and install [Qt 5.4.1 (32bit, MinGW)](http://www.qt.io/download-open-source/#section-2) to the default location.
* Download and install [MySQL Community Server 5.5 (32-bit)](http://dev.mysql.com/downloads/mysql/5.5.html)  to the default location.  
  It is needed to make the Qt MYSQL plugin work.  
  After the installation, copy `libmysql.dll` from `C:\PROGRA~2\MySQL\MYSQLS~1.5\lib` to `C:\Qt\Qt5.4.1\5.4\mingw491_32\lib`
* Download and install [Git](https://git-scm.com/download/win)  
  It is needed to extract the repository version during the build process.  
* Download and install [CMake](http://www.cmake.org/cmake/resources/software.html).
  It is needed to build BamTools. Make sure to add the CMake folder to the path during the installation.
* __Optional:__ Install [Python](https://www.python.org/download/windows/) and the [matplotlib](http://matplotlib.org/) extension.  
   It is needed to create plots only.  
   Make sure to add the python folder to the path during the installation. 
	
##Build process
Now, we can build the third-party libraries, and then ngs-bits:

* Build the BamTools library by executing the script `bamtools.bat`.  
  [BamTools](http://sourceforge.net/projects/bamtools/) is needed by ngs-bits to access BAM files.
* Finally, build the ngs-bits tools using the QtCreator project file `src\tools.pro`.  
  After a successful build, the tools can be found in the `bin\` folder.