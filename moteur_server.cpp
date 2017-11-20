/***********************************************
 * file:  moteur_server.cpp
 * date:  ?
 * authors: Tristan (Creatis) glatard@creatis.insa-lyon.fr
 * 			Martin (AMC) m.stam@amc.uva.nl
 *
 * project website:
 *	http://amc-app1.amc.sara.nl/twiki/bin/view/EBioScience/Admin/MoteurServiceDev#MOTEUR_web_service_moteur_server
 *
 * description:
 *  Point of entry for the MOTEUR webservice. This binary:
 *   - starts a new session
 *   - stores a passed certificate to the /tmp directory.
 *   - generates a workflow directory
 *   - Initializes the workflow run structure and config files in the mentioned directory
 *
 * history:
 *  -
 *  - 29 september: Header added.
 *  - 30 september: Fixed bugs and errors in 'getVersion' and SERVER_PORT retrieval function
 *  - 5 October: Merged with moteur code ; added proxy lifetime checking
 *  - March 2011: added the getworkflowstatus for the shiwa project
 * parameters:
 *  -
 **********************************************/

#include "soapH.h"
#include "moteur_service.nsmap"
#include <iostream.h>
#include <string.h>
#include <wait.h>
#include <unistd.h>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>
#include <sys/wait.h>
#include <stdlib.h>

using namespace std;

string serviceVersion = "0.3";

int main()
{
  soap_serve(soap_new()); // use the remote method request dispatcher
  return 0;
}

void writeLog(string s){
  ofstream logFile("./moteur_service.log",ios::app);
  time_t t;
  time(&t);
  char * date = ctime(&t);
  date[strlen(date)-1]='\0';
  logFile<<"["<<date<<"] "<<s<<endl;
  logFile.close();
}

int ns__getVersion(struct soap * soap, std::string & versionID)
{
  versionID = serviceVersion;
  return SOAP_OK;
}

int ns__killWorkflow(struct soap * soap, std::string workflowId){
  string command="./killWorkflow.sh ";
  command+=workflowId;
  command+=" >> ./moteur_service.log";
  system(command.c_str());
  return SOAP_OK;
}

int ns__getWorkflowStatus(struct soap * soap, std::string workflowId, std::string & workflowStatus)
{
  string message="getting status of workflow ";
  message+=workflowId;
  writeLog(message);

  
  string command="cat /proc/*/cmdline > /tmp/tmpgrep ";
  system(command.c_str());
  
  command = "grep ";
  command+=workflowId;
  command+="/workflow.xml /tmp/tmpgrep &>/dev/null" ;

  writeLog(command.c_str());

  int  status = system(command.c_str());
  

  system("rm -f /tmp/tmpgrep");

  if(status == -1){
    char s [126];
    sprintf(s, "[moteur_server] cannot get workflow status");
    writeLog(s);
    return soap_sender_fault(soap, s, s);    
  }
  int r =  WEXITSTATUS(status);
  if(r == 0)
    workflowStatus="RUNNING";
  else
    {
      //workflow is terminated or complete
      command="grep \"completed execution of workflow\" ";
      char temp[1024];
      strcpy(temp,getenv("DOCUMENT_ROOT"));
      strcat(temp,"/workflows/");
      strcat(temp,workflowId.c_str());
      strcat(temp,"/workflow.out");
      command+=temp;
      writeLog(command);
      int stat = system(command.c_str());
      if(stat == -1){
	char s [126];
	sprintf(s, "[moteur_server] cannot grep workflow status");
	writeLog(s);
	return soap_sender_fault(soap, s, s);    	  
      }
      int ret = WEXITSTATUS(stat);
      if(ret==0){
	workflowStatus="COMPLETE";
      }
      else
	if(ret == 1)
	  workflowStatus="TERMINATED";
	else
	  workflowStatus="UNKNOWN WORKFLOW";
    }
  
  writeLog("Status of workflow "+workflowId+" is "+workflowStatus);
  return SOAP_OK;
}


int ns__workflowSubmit(struct soap * soap, std::string scuflDocument, std::string inputDocument, std::string proxy, std::string settings, std::string & workflowID)
{
  writeLog("Running moteur_server: ");
  writeLog("Starting workflowSubmit");
  writeLog("Scufl doc is "+scuflDocument);
  writeLog("Input doc is "+inputDocument);
  writeLog("Settings received: " + settings);
  //CREATES UNIQUE DIRECTORY IN $MOTEURROOT
  writeLog("Creating directory");

  if(!getenv("DOCUMENT_ROOT")){
    char s [126];
    sprintf(s, "DOCUMENT_ROOT is not defined");
    writeLog(s);
    return soap_sender_fault(soap, "DOCUMENT_ROOT is not defined", s);
  }

  char temp[1024];
  strcpy(temp,getenv("DOCUMENT_ROOT"));
  strcat(temp,"/workflows");
  if(access(temp,F_OK)){
    if(mkdir(temp,0755)){
      writeLog(strerror(errno));
      char s [126];
      sprintf(s, "Unable to create directory");
      writeLog(s);
    }
    else{
      char ttemp[1024];
      strcpy(ttemp,temp);
      strcat(ttemp,"/.htaccess");
      ofstream htaccess(ttemp);
      htaccess<<"AddType application/vlemed/jobids .vljids"<<endl<<"AddType application/dirmapper .rdf"<<endl<<"AddType text/plain .err"<<endl<<"AddType text/plain .out"<<endl<<"AddType text/plain .xml"<<endl;
      htaccess.close();
      chmod(ttemp,S_IRUSR);
    }
  }

  
  strcat(temp,"/workflow-XXXXXX");
  char * tempdir_c = mkdtemp(temp);
  if(tempdir_c == 0){
    writeLog(strerror(errno));
    char s [126];
    sprintf(s, "[moteur_server] Unable to create directory %s",tempdir_c);
    writeLog(s);
    return soap_sender_fault(soap, s, s);
  }
  
  string tempdir = tempdir_c;
  writeLog("Dir "+tempdir+" created.");
  writeLog("Chmoding dir to 755.");
  if(chmod(temp,S_IRUSR | S_IWUSR  | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)){
    char s [126];
    sprintf(s, "[moteur_server] Cannot change permission of dir %s to 755",tempdir.c_str());
    writeLog(s);
    return soap_sender_fault(soap, s, s);
  }
  writeLog("Dir chmoded.");

  // Create a conf directory so we can store the settings that we received.
  string confdir = tempdir;
  confdir += "/conf";
  writeLog("Creating conf subdirectory");

  if(mkdir(confdir.c_str(), 0755)) {
    writeLog(strerror(errno));
    char s [200];
    sprintf(s, "[moteur_server] Unable to create conf-directory: %s",confdir.c_str());
    writeLog(s);
    return soap_sender_fault(soap, s, s);
  }
  writeLog("Confdir created.");

  writeLog("Writing workflow and input files");
  //WRITES THE FILES
  string dummy="";
  ofstream workflow((tempdir+"/workflow.xml").c_str(),ios::trunc);
  if(workflow==NULL){
    char s [126];
    sprintf(s, "[moteur_server] Cannot write in dir %s",tempdir.c_str());
    writeLog(s);
    return soap_sender_fault(soap, s, s);
  }
  workflow<<scuflDocument<<endl;
  workflow.close();
  writeLog("Workflow file successfully written");

  ofstream inputs((tempdir+"/inputs.xml").c_str(),ios::trunc);
  if(inputs==NULL){
    char s [126];
    writeLog(s);
    sprintf(s, "Cannot write in dir %s",tempdir.c_str());
    return soap_sender_fault(soap, s, s);
  }
  inputs<<inputDocument<<endl;
  inputs.close();
  writeLog("Input file successfully written");

  string conffile = confdir + "/settings.conf";


  ofstream configFile(conffile.c_str(), ios::trunc);
  if(configFile==NULL){
    char s [126];
    sprintf(s, "Cannot write in settings.conf");
    writeLog(s);
    return soap_sender_fault(soap, "Job execution error", s);
  }

  // Write settings to file, if any settings have been passed to the service.
  configFile<<settings<<endl;
  configFile.close();
  writeLog("Config file successfully written");

  //WRITES THE PROXY
  string proxyfile="/tmp/";
  proxyfile+=basename(tempdir.c_str());
  proxyfile+="-proxy";
  ofstream proxy_file(proxyfile.c_str(),ios::trunc);
  if(proxy_file==NULL){
    char s [126];
    sprintf(s, "Cannot write in file %s",proxyfile.c_str());
    writeLog(s);
    return soap_sender_fault(soap, s, s);
  }
  proxy_file<<proxy;
  proxy_file.close();

  chmod(proxyfile.c_str(),S_IRUSR);
  writeLog("Proxy file successfully written");

  //tests proxy lifetime
  string lifetimeCommand="X509_USER_PROXY=";
  lifetimeCommand+=proxyfile;
  lifetimeCommand+="/opt/glite/bin/voms-proxy-info --timeleft";
  char buffer[1024];
  strcpy(buffer,"");
  int stat = 0;
  FILE * f = popen(lifetimeCommand.c_str(),"r");
  if(f){
    while(f && !feof(f)){
      fgets(buffer,1024,f);
    }
    stat = pclose(f);
  }
  else
    return soap_sender_fault(soap, "Cannot read your proxy", "Cannot read your proxy");

  //ignore the result if there was an issue executing the command
  if(stat == -1){
    writeLog("Warning: there was an issue executing the voms-proxy-info command: couldn't check proxy lifetime");
  }
  else{
    int timeleft = atoi(buffer);
    if(timeleft < 18000 && stat == 0){
      char s [126];
      sprintf(s, "Your proxy lifetime is only %s seconds: please reinitialize it",buffer);
      writeLog(s);
      return soap_sender_fault(soap, s,s);
    }
  }

  //LAUNCHES MOTEUR
  string command = "X509_USER_PROXY=";
  command+=proxyfile;
  command+=" ./submitWorkflow.sh  ";
  command+=tempdir;
  command+="/workflow.xml ";
  command+=tempdir;
  command+="/inputs.xml ";
  command+=proxyfile;
  command+=">> ./moteur_service.log";

  writeLog("Launching MOTEUR: "+command);

  system(command.c_str());

  char hostname[128];
  gethostname(&hostname[0],sizeof(hostname));
  char port[10];
  strcpy(port, getenv("SERVER_PORT"));
  workflowID=dummy+"https://"+hostname+":"+port+"/workflows/"+basename(tempdir.c_str())+"/html/"+basename(tempdir.c_str())+".html";
  writeLog(workflowID);
  writeLog("Bye.");
  return SOAP_OK;
}
