#!/bin/bash

# Homes
export JAVA_HOME=/usr/lib/jvm/jre-1.7.0-openjdk.x86_64
export HOME=/var/www # HOME is not set in CGI scripts

# UI
export LFC_HOST=lfc-biomed.in2p3.fr
export LANG=us_US.UTF-8
export LCG_GFAL_INFOSYS=cclcgtopbdii01.in2p3.fr:2170

# MOTEUR
export MOTEUR_HOME=${PWD}

# Proxy
export ROBOT_PROXY=${PATH_TO_PROXY}
export X509_USER_PROXY=${ROBOT_PROXY} 

# DIRAC
source ${MOTEUR_HOME}/dirac-client/bashrc &>/tmp/dirac-env1.err # DIRAC configuration
