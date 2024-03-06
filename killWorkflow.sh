#!/bin/bash -l

function log
{
    echo -n "[`date`"
    echo "] $1"
}

source ./env.sh

ID=$1

log "killing workflow ${ID}"

OUT=`mktemp /tmp/kill-XXXXX.out`
ERR=`mktemp /tmp/kill-XXXXX.err`

for i in `ps -fu apache | grep moteur2.client.Main | grep ${ID} | awk '{print $2}'`
do
 log "Killing process $i"
 kill -9 $i 
done


cd ${MOTEUR_HOME}/workflow-agent
log "Calling java -jar workflow-agent.jar kill ${DOCUMENT_ROOT}/workflows ${ID} 1>>./workflowagent.out 2>>./workflowagent.err &"
java -jar workflow-agent.jar kill ${DOCUMENT_ROOT}/workflows ${ID} 1>>./workflowagent.out 2>>./workflowagent.err &

\rm -f ${OUT}
\rm -f ${ERR}

exit 0
