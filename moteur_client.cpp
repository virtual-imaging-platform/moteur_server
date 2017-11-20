#include "soapH.h"
#include "moteur_service.nsmap"
#include <iostream>
#include <string>
#include <fstream>

using namespace std;

int main(int argc, char **argv) {

  if(argc!=4) {cout<<"usage : moteur_client server workflow_file input_file"<<endl; exit(-1);}

  string serveur = argv[1];
  string workflow_file = argv[2];
  string input_file = argv[3];
  string settings = "";

  ifstream wf((char *)workflow_file.c_str());
  if(wf==NULL){
    cerr<<"Cannot open file "<<workflow_file<<endl;
    exit(1);
  }
  char buff[2048];
  string wFile="";
  while(wf.getline(buff,2048,'\n'))
    {
      wFile+=buff;
    }
  wf.close();

  ifstream inp((char *) input_file.c_str());
  if(inp == NULL)
    {
      cerr<<"Cannot open file "<<input_file<<endl;
      exit(1);
    }
  string inFile="";
  while(inp.getline(buff,2048,'\n'))
    {
      inFile+=buff;
    }
  inp.close();

  if(!getenv("X509_USER_PROXY"))
    {
      cerr<<"X509_USER_PROXY is not set"<<endl;
      exit(1);
    }
  ifstream proxy(getenv("X509_USER_PROXY"));
  if(proxy == NULL)
    {
      cerr<<"Cannot open file "<<getenv("X509_USER_PROXY")<<endl;
      exit(1);
    }
  string prox="";
  while(proxy.getline(buff,2048,'\n'))
    {
      prox+=buff;
      prox+="\n";
    }
  proxy.close();


  cout<<prox<<endl;

  struct soap soap;


  soap_ssl_init();
  if (soap_ssl_client_context(&soap,
			      SOAP_SSL_SKIP_HOST_CHECK, NULL, NULL, NULL, NULL, NULL
			      
			      ))
    {
      soap_print_fault(&soap, stderr);
      exit(1);
    } 
  
//  soap_init(&soap);

  string result;
  soap_call_ns__workflowSubmit(&soap, serveur.c_str(), "", wFile.c_str(), inFile.c_str(),prox, settings, result);
  if (soap.error)
    soap_print_fault(&soap, stderr);
  else
    {
      cout<<result<<endl;
 }
  return 0;
}
