# Dependencies

* Installation of development environment on CentOS

  `sudo yum install gcc gcc-c++ flex bison libxml2-devel openssl-devel`

* [gSOAP Toolkit](https://sourceforge.net/projects/gsoap2)

  The makefile of `moteur_server` needs that all the used files
  (sources, lib, bin, include) are in the same directory.  To do this,
  we set the prefix for the compilation.

  ```shell
  unzip -q gsoap_2.8.55.zip
  cd gsoap-2.8
  ./configure --exec-prefix=$PWD/gsoap --prefix=$PWD/gsoap
  make
  make install
  ```

# Compilation

* Edit the first line of `Makefile` to make variable `GSOAP` point to
  your gSOAP installation:
  ```
  GSOAP=<PATH>/gsoap-2.8/gsoap
  ```

* Run the command
  ```shell
  make
  ```
