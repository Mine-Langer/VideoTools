#pragma once

#include <Windows.h>
#include <cstdio>
#include <tchar.h>
#include <cinttypes>
#include <dsound.h>
#include <mmsystem.h>
#include <queue>
#include <thread>
#include <mutex>

#pragma comment (lib, "winmm.lib")
#pragma comment (lib, "dsound.lib")
#pragma comment (lib, "dxguid.lib")



class DSPlayer
{
public:
	DSPlayer();
	~DSPlayer();

	bool Start();

	void PushPCMBuf(uint8_t* pBuf, int nSize);

private:
	bool Init();

	void PlayThread();

private:
	LPDIRECTSOUND		m_device = nullptr;
	LPDIRECTSOUNDBUFFER m_primaryBuf = nullptr;
	LPDIRECTSOUNDBUFFER m_secondaryBuf = nullptr;

	DWORD				m_nBufNum = 10;
	DWORD				m_bytesPerNotifySize = 0;
	uint8_t*			m_bytesNotifyPtr = nullptr;
	HANDLE*				m_phEvents = nullptr;

	int m_offset = 0;

	bool				m_bRun = false;
	std::thread			m_threadPlay;

	typedef struct { uint8_t* buf; int size; } PCM_BUF;
	std::queue<PCM_BUF> m_pcmQueue;
	std::mutex			m_mux;
};



//////////////////////////////////////////////////////////////////////
typedef struct PLAYER {
	PLAYER() {
		device = NULL;
		primaryBuffer = NULL;
		secondaryBuffer = NULL;
		started = false;
		bytes_per_notif_ptr = NULL;

		fp = NULL;

	}
	LPDIRECTSOUND device;
	LPDIRECTSOUNDBUFFER primaryBuffer;
	LPDIRECTSOUNDBUFFER secondaryBuffer;
	HANDLE notifEvents[20];
	bool started;
	size_t bytes_per_notif_size;
	uint8_t* bytes_per_notif_ptr;
	HANDLE tid[2];

	FILE* fp;

} Player;

/*播放准备*/
int prepare(Player* ds);
/*开始播放*/
int startPlayer(Player* ds);
/*挂起播放*/
int suspendPlayer(Player* ds);
/*唤醒播放*/
int resumePlayer(Player* ds);
/*停止播放*/
int stopPlayer(Player* ds);
/*释放内存资源*/
int unprepare(Player* ds);

DWORD WINAPI playerThreadImpl(LPVOID params);
int openFile(Player* ds);
int closeFile(Player* ds);