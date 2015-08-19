#include <Windows.h>
#include <stdio.h>
#include <process.h>
//#include <winsock.h>
#include "GP3.h"
#include "BKBRepErr.h"
#include "Internat.h"

// Прототип callback-функции из OnGazeData.cpp
void on_gaze_data(const tobiigaze_gaze_data* gazedata, void *user_data);

bool BKBGP3::initialized(false);

static SOCKET GP3Socket;

static volatile bool flag_ShutDownThreads=false; // flag to inform the threads to terminate

uintptr_t handler_Reader=0; // thread handlers

//=========================================================
// Reader thread - receiving messages from the server here
//=========================================================
static unsigned __stdcall ReaderThread(void *p)
{
   // big enough buffers...
   char buffer[4096];

   int bytesReceived;
   int num_scanned=0;

   float x_bpog,y_bpog;
   int v_bpog;
      
   while(!flag_ShutDownThreads)
   {
      bytesReceived = recv(GP3Socket, buffer, 4095, 0);
      if(0>=bytesReceived) return 0; // Connection closed
      buffer[bytesReceived]=0; // Make a string zero-terminated
      
	  //printf("received: %s\n",buffer);
	  num_scanned=sscanf_s(buffer,"<REC BPOGX=\"%f\" BPOGY=\"%f\" BPOGV=\"%d\" />",&x_bpog,&y_bpog,&v_bpog);
	
	  if(3==num_scanned)
	  {
		  tobiigaze_gaze_data gd;

		  if(1==v_bpog) gd.tracking_status = TOBIIGAZE_TRACKING_STATUS_BOTH_EYES_TRACKED;
		  else gd.tracking_status = TOBIIGAZE_TRACKING_STATUS_NO_EYES_TRACKED;
		  gd.left.gaze_point_on_display_normalized.x=x_bpog;
		  gd.left.gaze_point_on_display_normalized.y=y_bpog;

		  gd.right.gaze_point_on_display_normalized.x=gd.left.gaze_point_on_display_normalized.x;
		  gd.right.gaze_point_on_display_normalized.y=gd.left.gaze_point_on_display_normalized.y;

		  gd.timestamp=1000UL*timeGetTime(); // Используется скроллом

		  on_gaze_data(&gd, NULL);
	  }

   }
   return 0;
}

//=========================================================
// Init TCP connection (socket)
//=========================================================
int GP3connect()
{
   WORD sockVersion;
   WSADATA wsaData;
   int nret;
   TCHAR *err_string;
   
   SOCKADDR_IN GP3server;

   // Initialize Winsock
   sockVersion=MAKEWORD(2, 2);         

   nret=WSAStartup(sockVersion, &wsaData);
   if(nret)
   {
      err_string=Internat::Message(20,L"Не удалось инициализировать Winsock\r\n");
      goto ws_error;
   }

   // Create a socket
   GP3Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);      
   if (INVALID_SOCKET == GP3Socket)
   {
      err_string=Internat::Message(21,L"Не удалось создать socket\r\n");
      goto ws_error;
   }

   // Local server 
   GP3server.sin_family = AF_INET;
   GP3server.sin_addr.S_un.S_un_b.s_b1=127; // 127.0.0.1
   GP3server.sin_addr.S_un.S_un_b.s_b2=0;
   GP3server.sin_addr.S_un.S_un_b.s_b3=0;
   GP3server.sin_addr.S_un.S_un_b.s_b4=1;
   GP3server.sin_port = htons(4242);

   // Connect the local server
   nret = connect(GP3Socket, (LPSOCKADDR)&GP3server, sizeof(struct sockaddr));
   if (SOCKET_ERROR == nret)
   {
      err_string=L"Cannot connect to GP3 server\n"; // !!! переделать под Internat
      goto ws_error;
   }

   // Connected!!!
   //fprintf(stderr,"Connected to server!\n");
   return 0;

ws_error:
   // Print error and shut down winsock
   //fprintf(stderr,"%s. Press 'Enter' to close the program.\n", err_string);
   BKBReportError(err_string);
   WSACleanup();
   return 1;
}

//=====================================================================================
// Начало работы с устройством
//=====================================================================================
static char *GP3_enable_send_data="<SET ID=\"ENABLE_SEND_DATA\" STATE=\"1\" />\r\n";
static char *GP3_enable_pog_best="<SET ID=\"ENABLE_SEND_POG_BEST\" STATE=\"1\" />\r\n";
int BKBGP3::Init()
{
	if(initialized) return 1; // уже инициализировали
	initialized=true;


	// 1. Initialize TCP connection
    if(GP3connect()) goto cleanup;

    // 2. Начать получать данные
    send(GP3Socket,GP3_enable_send_data,strlen(GP3_enable_send_data),0);
    send(GP3Socket,GP3_enable_pog_best,strlen(GP3_enable_pog_best),0);

    // 3. Start Reader thread
	handler_Reader=_beginthreadex(NULL,0,ReaderThread,0,0,NULL);
	if(1>handler_Reader) goto cleanup; // this will never happen... but...

	return 0;

cleanup:
	Halt();
	return 1;
}

//=====================================================================================
// Завершение работы с устройством
//=====================================================================================
int BKBGP3::Halt()
{
	if(!initialized) return 1; // уже завершили работу
	initialized=false;

	// Shut down winsock
	WSACleanup();

	// Wait for threads to terminate
	flag_ShutDownThreads=true; // inform the threads to terminate
	if(handler_Reader) WaitForSingleObject((HANDLE)handler_Reader,0);

	return 0;
}