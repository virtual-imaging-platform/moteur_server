#include "soapH.h"
#include "moteur_service.nsmap"
#include <iostream>
#include <string>
#include <fstream>

using namespace std;

void usage(){
  cout<<"usage : moteur_client [server] [submit|status|kill] [workflowId|workflow.json input.xml]"<<endl; exit(-1);
}

int main(int argc, char **argv) {

  if(argc<4)
    usage();

  string serveur = argv[1];
  string command = argv[2];

  if(command == "status" || command == "kill"){
    string workflow_id = argv[3];

    struct soap soap;
    soap_ssl_init();
    if (soap_ssl_client_context(&soap,SOAP_SSL_SKIP_HOST_CHECK, NULL, NULL, NULL, NULL, NULL))
      {
        soap_print_fault(&soap, stderr);
        exit(1);
      }
    string result;
    if(command == "status")
      soap_call_ns__getWorkflowStatus(&soap, serveur.c_str(), "", workflow_id.c_str(), result);
    if(command == "kill")
      soap_send_ns__killWorkflow(&soap, serveur.c_str(), "", workflow_id.c_str());
    if (soap.error)
      soap_print_fault(&soap, stderr);
    else
      {
        cout<<result<<endl;
      }
  }
  else
    if(command == "submit"){
      if(argc!=5)
        usage();

      string workflow_file = argv[3];
      string input_file = argv[4];
      string settings = "";
      ifstream wf((char *)workflow_file.c_str());
      if(!wf){
        cerr<<"Cannot open file "<<workflow_file<<endl;
        exit(1);
      }
      char buff[2048];
      string wFile="";
      while(wf.getline(buff,2048,'\n'))
        {
          wFile+=buff;
          wFile+="\n";
        }
      wf.close();

      ifstream inp((char *) input_file.c_str());
      if(!inp)
        {
          cerr<<"Cannot open file "<<input_file<<endl;
          exit(1);
        }
      string inFile="";
      while(inp.getline(buff,2048,'\n'))
        {
          inFile+=buff;
          inFile+=+"\n";
        }
      inp.close();

      if(!getenv("X509_USER_PROXY"))
        {
          cerr<<"X509_USER_PROXY is not set"<<endl;
          exit(1);
        }
      ifstream proxy(getenv("X509_USER_PROXY"));
      if(!proxy)
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

    }
    else
      cerr<<"Unknown command "<<command<<endl;
  return 0;
}
