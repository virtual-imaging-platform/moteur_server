#!/bin/bash -l

# Functions

function checkOverrideVal
{
    for i in `cat ${MOTEUR_HOME}/conf/override.conf`
      do
      currVal=`echo ${i} | awk -F'=' '{print $1}'`
      if [ ${currVal} == $1 ]
	  then
	  echo ${i} | awk -F'=' '{print $2}'
	  break
      fi
    done
}

# returns the value of a given key from override.txt
function getOverrideVal
{
    overrideVal=`grep "$1=" ${MOTEUR_HOME}/conf/override.conf`
    echo "${overrideVal}"

}

function log
{
    echo -n "[`date`"
    echo "] $*"
}

function error {
    local MESSAGE=$*
    log ${MESSAGE}
    mkdir -p ${DIR}/html/
    echo "<html><head><title>ERROR</title><meta http-equiv=\"refresh\" content=\"1\"></head><body>Could not submit workflow.<br>${MESSAGE} <br></body></html>" > ${DIR}/html/`basename ${DIR}`.html
    exit 1
}

###

# Environment
source env.sh
###

# Input parameters
WORKFLOW_DOC=$1
INPUT_DOC=$2
###

log "Launching workflow ${WORKFLOW_DOC} on data ${INPUT_DOC}"

# Workflow execution directory, e.g. /var/www/html/workflows/workflow-abcde
cd `dirname ${INPUT_DOC}`

#############################################################
# Process the settings file, checking for missing settings and add default settings.
#

export GRIDCONF="conf/settings.conf"
export GRIDCONF_BUFFER="conf/settings2.conf"

log "Implementing default settings for missing parameters:"

ifs=${IFS}
IFS=$'\n'
for val in `cat ${MOTEUR_HOME}/conf/default.conf`
  do
  currVal=`echo ${val} | awk -F'=' '{print $1}'`
  buffer=`awk -F '=' -v v=${currVal} '$1==v {print $2}' ${GRIDCONF}`
  if [ -z ${buffer} ]
      then
      echo ${val} >> ${GRIDCONF}
  fi
done

#
# End of default settings.
###############################################################

#############################################################
# Process the settings file, checking for settings to override.
#

log "Overriding given settings..."
# run through the settings that were passed by the user.
for val in `cat ${GRIDCONF}`
  do
  valValue=`echo ${val} | awk -F'=' '{print $2}'`
  valValueLength=`echo ${valValue} | wc -m`

  # Default value overwriting might have left empty values: Drop them if found
  if [ ${valValueLength} -lt 2 ]
      then
    # The line contained a key, but no value. Skip it.
      continue
  fi

  currVal=`echo ${val} | awk -F'=' '{print $1}'`

  overrideVal=`checkOverrideVal ${currVal}`
  oValLength=`echo ${overrideVal} | wc -c`
  if [ `echo ${overrideVal} | wc -c` -gt 1 ]
      then
    # override setting:
      outputVal=`getOverrideVal ${currVal}`
      echo ${outputVal} >> ${GRIDCONF_BUFFER}
  else
      echo ${val} >> ${GRIDCONF_BUFFER}
  fi
done
IFS=${ifs}
rm ${GRIDCONF}
cp ${GRIDCONF_BUFFER} ${GRIDCONF}
rm ${GRIDCONF_BUFFER}

#
# End of override settings.
#############################################################

# See Redmine ticket #2781
cp ${INPUT_DOC} input-m2.xml

# Loading jar dependencies in CLASSPATH
export CLASSPATH=${MOTEUR_HOME}/moteur2.jar
for i in `ls ${MOTEUR_HOME}/{libs,plugins}/*.jar`
do 
    export CLASSPATH=${CLASSPATH}:$i
done

# Launching workflow
echo "---- LAUNCHING WORKFLOW: nohup nice -19 $JAVA_HOME/bin/java -Xmx950M -XX:-UseGCOverheadLimit -Duser.home=\"${HOME}/prod\" -DX509_USER_PROXY=${X509_USER_PROXY} fr.cnrs.i3s.moteur2.client.Main --config ${MOTEUR_HOME}/.moteur2 -ng -p `basename ${PWD}` ${WORKFLOW_DOC} ${INPUT_DOC} 1>./workflow.out 2>./workflow.err &"
nohup nice -19 $JAVA_HOME/bin/java -Xmx950M -XX:-UseGCOverheadLimit -Duser.home="${HOME}/prod" -DX509_USER_PROXY=${X509_USER_PROXY} fr.cnrs.i3s.moteur2.client.Main --config ${MOTEUR_HOME}/.moteur2 -ng -p `basename ${PWD}` ${WORKFLOW_DOC} ${INPUT_DOC} 1>./workflow.out 2>./workflow.err &



    
