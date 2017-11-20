//gsoap ns service name:	moteur_service
//gsoap ns service style:	document
//gsoap ns service encoding:	literal
// //gsoap ns service namespace:
//gsoap ns service location:	http://localhost:18000

//gsoap ns schema namespace: moteur_service
#ifndef MOTEUR_SERVICE_H
#define MOTEUR_SERVICE_H

int ns__workflowSubmit(std::string scuflDocument, std::string inputDocument, std::string proxy, std::string settings, std::string & workflowID);
int ns__getVersion(std::string & versionID);
int ns__getWorkflowStatus(std::string workflowID, std::string & workflowStatus);
int ns__killWorkflow(std::string workflowID,void);
#endif

