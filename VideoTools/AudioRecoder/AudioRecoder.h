#pragma once
#include "../Common.h"
#include <Audioclient.h>
#include <mmdeviceapi.h>

#pragma comment(lib, "Avrt.lib")

class CAudioRecoder
{
public:
	CAudioRecoder(int CaptureType);
	~CAudioRecoder();

	enum { SOUNDCARD, MICROPHONE };

	int get_sample(unsigned char*& buffer, int& size, int64_t& timestamp); // ��ȡ������
	int get_nb_channels();		// ��ȡ����������������ֵ��2,1��
	int get_nb_bits_sample();	// ��ȡ����ʱÿһ��������Ĵ洢������������ֵ��16,8,24��
	int get_sample_rate();		// ��ȡ�����ʣ�����ֵ��44100,48000��
	int get_nb_samples();		// ��ȡ����ʱ������һ֡��Ƶ�Ĳ���������������ֵ��44100->1024,48000->1056��

private:
	bool reInit();
	bool Init();
	void Release();

	IMMDevice* GetDefaultDevice();
	bool AdjustFormat(WAVEFORMATEX* pwfx);

private:
	int m_CaptureType = 0;
	bool m_Inited = false;
	IMMDevice* m_pDevice = nullptr;
	WAVEFORMATEX* m_pWaveFmt = nullptr;
	IAudioClient* m_pAudioClient = nullptr;
	IAudioCaptureClient* m_pAudioCaptureClient = nullptr;
	HANDLE m_hTask = nullptr;

	UINT m_nb_samples = 0; // ÿһ֡��Ƶ�Ĳ���������
};

