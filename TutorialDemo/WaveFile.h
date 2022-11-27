#pragma once
#include <dsound.h>
#include <mmsystem.h>

#pragma comment (lib, "winmm.lib")
#pragma comment (lib, "dsound.lib")
#pragma comment (lib, "dxguid.lib")

#define WAVEFILE_READ	1
#define WAVEFILE_WRITE	2

#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p)  { if(p) { delete (p);     (p)=NULL; } }

class CWaveFile
{
public: 
	CWaveFile();
	~CWaveFile();

	BOOL Open(TCHAR* szFilename, WAVEFORMATEX* pwfx, DWORD dwFlags);

	BOOL Close();

	BOOL Read(BYTE* pBuffer, DWORD dwSizeToRead, DWORD* pdwSizeRead);

	BOOL Write(DWORD dwSizeToWrite, BYTE* pbData, DWORD* pdwSizeWrite);

	DWORD Size();

	BOOL Reset();

private:
	bool ReadMMIO();
	bool WriteMMIO(WAVEFORMATEX* pwfx);

private:
	WAVEFORMATEX* m_pwfx;        // Pointer to WAVEFORMATEX structure
	HMMIO         m_hmmio;       // MM I/O handle for the WAVE
	MMCKINFO      m_ck;          // Multimedia RIFF chunk
	MMCKINFO      m_ckRiff;      // Use in opening a WAVE file
	DWORD         m_dwSize;      // The size of the wave file
	MMIOINFO      m_mmioinfoOut;
	DWORD         m_dwFlags;
	BOOL          m_bIsReadingFromMemory;
	BYTE* m_pbData;
	BYTE* m_pbDataCur;
	ULONG         m_ulDataSize;
	TCHAR* m_pResourceBuffer;
};

