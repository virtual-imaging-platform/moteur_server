SERVICE_NAME=moteur
GSOAP=./gsoap-2.8/install

all: 	stubs ${SERVICE_NAME}_server ${SERVICE_NAME}_client ${SERVICE_NAME}_client-status
soapServer.o:
	g++ -g -DWITH_OPENSSL -fPIC -Wno-deprecated -Wall -I ${GSOAP}/include -c soapServer.cpp

soapClient.o:
	g++ -g -DWITH_OPENSSL -fPIC -Wno-deprecated -Wall -I ${GSOAP}/include -c soapClient.cpp

soapC.o:
	g++ -g -DWITH_OPENSSL -fPIC -Wno-deprecated -Wall -I ${GSOAP}/include -c soapC.cpp

stdsoap2.o: ${GSOAP}/stdsoap2.cpp
	g++ -g -DWITH_OPENSSL -fPIC -Wno-deprecated -Wall -c ${GSOAP}/stdsoap2.cpp

${SERVICE_NAME}_server.o: soapC.o soapServer.o ${SERVICE_NAME}_server.cpp
	g++ -g -DWITH_OPENSSL -fPIC -Wno-deprecated -Wall -I /usr/include/httpd -I ${GSOAP}/include `xml2-config --cflags` -c ${SERVICE_NAME}_server.cpp

${SERVICE_NAME}_client.o: soapC.o soapClient.o ${SERVICE_NAME}_client.cpp
	g++ -g -DWITH_OPENSSL -fPIC -I ${GSOAP}/include -c ${SERVICE_NAME}_client.cpp

${SERVICE_NAME}_client-status.o: soapC.o soapClient.o ${SERVICE_NAME}_client-status.cpp
	g++ -g -DWITH_OPENSSL -fPIC -I ${GSOAP}/include -c ${SERVICE_NAME}_client-status.cpp


stubs :
	${GSOAP}/bin/soapcpp2 ${SERVICE_NAME}_service.h			

${SERVICE_NAME}_server: ${SERVICE_NAME}_server.o soapServer.o soapC.o
	g++ -g -lssl -lcrypto  -DWITH_OPENSSL -fPIC -Wno-deprecated -Wall ${SERVICE_NAME}_server.o soapC.o -lgsoapssl++ soapServer.o -L ${GSOAP}/lib -lpthread `xml2-config --libs`  -Wl,-rpath,. -o ${SERVICE_NAME}_server

${SERVICE_NAME}_client-status: ${SERVICE_NAME}_client-status.o soapClient.o soapC.o
	g++   -g -DWITH_OPENSSL -lssl -lcrypto  -fPIC ${SERVICE_NAME}_client-status.o soapC.o  -lgsoapssl++ -L${GSOAP}/lib soapClient.o -o ${SERVICE_NAME}_client-status

${SERVICE_NAME}_client: ${SERVICE_NAME}_client.o soapClient.o soapC.o
	g++   -g -DWITH_OPENSSL -lssl -lcrypto  -fPIC ${SERVICE_NAME}_client.o soapC.o soapClient.o -lgsoapssl++ -L${GSOAP}/lib -o ${SERVICE_NAME}_client


install: stubs ${SERVICE_NAME}_server ${SERVICE_NAME}_client
	cp ${SERVICE_NAME}_client ${SERVICE_NAME}_server ${MOTEUR}/bin

clean:
	/bin/rm -f  ${SERVICE_NAME}_server ${SERVICE_NAME}_client  *.log *.o *~ soap* *.re*.xml *.xsd *.wsdl *.nsmap

