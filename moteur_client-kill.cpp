#include "soapH.h"
#include "moteur_service.nsmap"
#include <iostream>
#include <string>
#include <fstream>

using namespace std;

int main(int argc, char **argv) {

  if(argc!=3) {cout<<"usage : moteur_client server workflowId"<<endl; exit(-1);}

  string serveur = argv[1];
  string workflow_id = argv[2];

  struct soap soap;
  soap_ssl_init();
  if (soap_ssl_client_context(&soap,SOAP_SSL_SKIP_HOST_CHECK, NULL, NULL, NULL, NULL, NULL))
    {
      soap_print_fault(&soap, stderr);
      exit(1);
    } 
  
//  soap_init(&soap);

  soap_send_ns__killWorkflow(&soap, serveur.c_str(), "", workflow_id.c_str());
  return 0;
}
