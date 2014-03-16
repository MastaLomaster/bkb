#include <Windows.h>
#include <stdio.h>
#include <process.h>
#include <winsock.h>
#include "TET.h"
#include "BKBRepErr.h"

// Прототип callback-функции из TobiiREX.cpp
void on_gaze_data(const tobiigaze_gaze_data* gazedata, void *user_data);

bool BKBTET::initialized(false);

extern int screenX, screenY;

extern double screen_scale;

// The only two messages that we send to the server
char *JSON_heart_beat="{\"category\":\"heartbeat\"}";
char *JSON_set_push="{\"category\":\"tracker\",\"request\":\"set\",\"values\":{\"push\":true,\"version\":1}}";
// Scanf template for the frame (containing "fix:false")
char *JSON_frame_false="{\"category\":\"tracker\",\"request\":\"get\",\"statuscode\":200,\"values\":{\"frame\":{"\
				 "\"avg\":{\"x\":%f,\"y\":%f},\"fix\":false,\"lefteye\":{\"avg\":{\"x\":%f,\"y\""\
				 ":%f},\"pcenter\":{\"x\":%f,\"y\":%f},\"psize\":%f,\"raw\":{\"x\":%f"\
				 ",\"y\":%f}},\"raw\":{\"x\":%f,\"y\":%f},\"righteye\":{\"avg\":{\"x\":%f"\
				 ",\"y\":%f},\"pcenter\":{\"x\":%f,\"y\":%f},\"psize\":%f,\"raw\":{\"x\":%f"\
				 ",\"y\":%f}},\"state\":%d,\"time\":%d}}}";
// Scanf template for the frame (containing "fix:true")
char *JSON_frame_true="{\"category\":\"tracker\",\"request\":\"get\",\"statuscode\":200,\"values\":{\"frame\":{"\
				 "\"avg\":{\"x\":%f,\"y\":%f},\"fix\":true,\"lefteye\":{\"avg\":{\"x\":%f,\"y\""\
				 ":%f},\"pcenter\":{\"x\":%f,\"y\":%f},\"psize\":%f,\"raw\":{\"x\":%f"\
				 ",\"y\":%f}},\"raw\":{\"x\":%f,\"y\":%f},\"righteye\":{\"avg\":{\"x\":%f"\
				 ",\"y\":%f},\"pcenter\":{\"x\":%f,\"y\":%f},\"psize\":%f,\"raw\":{\"x\":%f"\
				 ",\"y\":%f}},\"state\":%d,\"time\":%d}}}";

SOCKET TETSocket;

volatile bool flag_ShutDownThreads=false; // flag to inform the threads to terminate

uintptr_t handler_HeartBeat=0, handler_Reader=0; // thread handlers

//==================================================
// HeartBeat thread - sends a message once a second
//==================================================
unsigned __stdcall HeartBeatThread(void *p)
{
	while(!flag_ShutDownThreads)
	{
		if(SOCKET_ERROR==send(TETSocket,JSON_heart_beat,strlen(JSON_heart_beat),0)) return 0; // Connection closed
		Sleep(1000);
	}
	return 0;
}

//=========================================================
// Reader thread - receiving messages from the server here
//=========================================================
unsigned __stdcall ReaderThread(void *p)
{
	// big enough buffers...
	char buffer[4096];
	char true_false_buffer[128];

	int bytesReceived;
	int num_scanned=0;

	float x_avg,y_avg,
		left_x_avg, left_y_avg, left_pcenter_x, left_pcenter_y, left_psize, left_x_raw, left_y_raw,
		x_raw, y_raw,
		right_x_avg, right_y_avg, right_pcenter_x, right_pcenter_y, right_psize, right_x_raw, right_y_raw;
	int state, tet_time, fix;

	while(!flag_ShutDownThreads)
	{
		bytesReceived = recv(TETSocket, buffer, 4095, 0);
		if(0>=bytesReceived) return 0; // Connection closed
		buffer[bytesReceived]=0; // Make a string zero-terminated
		
		
		// First try. Template string contains "fix:false"
		fix=0;
		num_scanned=sscanf(buffer,JSON_frame_false,
			&x_avg,&y_avg,
			&left_x_avg, &left_y_avg, &left_pcenter_x, &left_pcenter_y, &left_psize, &left_x_raw, &left_y_raw,
			&x_raw, &y_raw, 
			&right_x_avg, &right_y_avg, &right_pcenter_x, &right_pcenter_y, &right_psize, &right_x_raw, &right_y_raw,
			&state, &tet_time
			);
		// Second try. Template string contains "fix:true"
		if(20!=num_scanned)
		{
			fix=1;
			num_scanned=sscanf(buffer,JSON_frame_true,
			&x_avg,&y_avg,
			&left_x_avg, &left_y_avg, &left_pcenter_x, &left_pcenter_y, &left_psize, &left_x_raw, &left_y_raw,
			&x_raw, &y_raw, 
			&right_x_avg, &right_y_avg, &right_pcenter_x, &right_pcenter_y, &right_psize, &right_x_raw, &right_y_raw,
			&state, &tet_time
			);
		}

		if(20==num_scanned)
		{
			//printf("X:%f Y:%f fix:%d state:%d\n",x_avg,y_avg,fix,state);
			
			if(7==state)
			{
				// Содрано из AirMouse.cpp
				tobiigaze_gaze_data gd;
				POINT p;
				p.x=(LONG)(x_avg+0.5f); p.y=(LONG)(y_avg+0.5f);;

				gd.tracking_status = TOBIIGAZE_TRACKING_STATUS_BOTH_EYES_TRACKED;
				gd.left.gaze_point_on_display_normalized.x=p.x/(double)screenX*screen_scale;
				gd.left.gaze_point_on_display_normalized.y=p.y/(double)screenY*screen_scale;
	
				gd.right.gaze_point_on_display_normalized.x=gd.left.gaze_point_on_display_normalized.x;
				gd.right.gaze_point_on_display_normalized.y=gd.left.gaze_point_on_display_normalized.y;

				gd.timestamp=1000UL*timeGetTime(); // Используется скроллом

				on_gaze_data(&gd, NULL);
			}
			
		}
	}
	return 0;
}

//=========================================================
// Init TCP connection (socket)
//=========================================================
int TETconnect()
{
	WORD sockVersion;
	WSADATA wsaData;
	int nret;
	char *err_string;
	
	SOCKADDR_IN TETserver;

	// Initialize Winsock
	sockVersion=MAKEWORD(2, 2);			

	nret=WSAStartup(sockVersion, &wsaData);
	if(nret)
	{
		err_string="Не удалось инициализировать Winsock\n";
		goto ws_error;
	}

	// Create a socket
	TETSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);		
	if (INVALID_SOCKET == TETSocket)
	{
		err_string="Не удалось создать socket\n";
		goto ws_error;
	}

	// Local server 
	TETserver.sin_family = AF_INET;
	TETserver.sin_addr.S_un.S_un_b.s_b1=127; // 127.0.0.1
	TETserver.sin_addr.S_un.S_un_b.s_b2=0;
	TETserver.sin_addr.S_un.S_un_b.s_b3=0;
	TETserver.sin_addr.S_un.S_un_b.s_b4=1;
	
	TETserver.sin_port = htons(6555);

	// Connect the local server
	nret = connect(TETSocket, (LPSOCKADDR)&TETserver, sizeof(struct sockaddr));
	if (SOCKET_ERROR == nret)
	{
		err_string="Не удалось соединиться с сервером\r\nУбедитесь, что запущена программа 'EyeTribe Server'\r\n"\
			"Переход в режим работы с [аэро]мышью.";
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
int BKBTET::Init()
{
	if(initialized) return 1; // уже инициализировали
	initialized=true;

	screenX=GetSystemMetrics(SM_CXSCREEN);
	screenY=GetSystemMetrics(SM_CYSCREEN);

	char buffer[4096]; 

	// 1. Initialize TCP connection
	if(TETconnect()) goto cleanup;

	// 2. Set Push mode
	send(TETSocket,JSON_set_push,strlen(JSON_set_push),0);
	recv(TETSocket, buffer, 4095, 0); // ignore the reply

	// 2. Start heartbeat thread
	handler_HeartBeat=_beginthreadex(NULL,0,HeartBeatThread,0,0,NULL);
	if(1>handler_HeartBeat) goto cleanup; // this will never happen... but...
		
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
int BKBTET::Halt()
{
	if(!initialized) return 1; // уже завершили работу
	initialized=false;

	// Shut down winsock
	WSACleanup();

	// Wait for threads to terminate
	flag_ShutDownThreads=true; // inform the threads to terminate
	if(handler_HeartBeat) WaitForSingleObject((HANDLE)handler_HeartBeat,0);
	if(handler_Reader) WaitForSingleObject((HANDLE)handler_Reader,0);

	return 0;
}