#include "AudioRecoder.h"
#include <avrt.h>
#include <chrono>

static int64_t getCurTimestamp()// 获取毫秒级时间戳（13位）
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

static std::string GetCurrTimeString()
{
	const char* time_fmt = "%Y-%m-%d %H:%M:%S";
	time_t t = time(nullptr);
	char time_str[64];
	strftime(time_str, sizeof(time_str), time_fmt, localtime(&t));

	return time_str;
}

CAudioRecoder::CAudioRecoder(int CaptureType)
{
	m_CaptureType = CaptureType;
	Init();
}

CAudioRecoder::~CAudioRecoder()
{
	Release();
}

int CAudioRecoder::get_sample(unsigned char*& buffer, int& size, int64_t& timestamp)
{
	if (!m_Inited)
		return -1;

	HRESULT hr = 0;
	UINT32 nextPacketSize = 0;
	UINT32 numFramesToRead;
	DWORD  dwFlags;
	unsigned char* pData = nullptr;

	timestamp = getCurTimestamp();
	int i = 0;

	while (true)
	{
		hr = m_pAudioCaptureClient->GetNextPacketSize(&nextPacketSize);
		if (FAILED(hr)) {
			return -3;
		}

		if (nextPacketSize != 0) {
			hr = m_pAudioCaptureClient->GetBuffer(&pData, &numFramesToRead, &dwFlags, nullptr, nullptr);
			if (FAILED(hr)) {
				//录制系统声音时，扬声器突然停止播放触发的错误
				//break;
				this->reInit();
				return -5;
			}


			if (dwFlags & AUDCLNT_BUFFERFLAGS_SILENT)
			{
				pData = nullptr;
			}
			if (!pData) {
				break;
			}

			int pDataLen = numFramesToRead * m_pWaveFmt->nBlockAlign;
			memcpy(buffer + size, pData, pDataLen);
			size += pDataLen;
			++i;
			m_pAudioCaptureClient->ReleaseBuffer(numFramesToRead);
		}
		else {
			break;
		}
	}
	if (0 == size) {
		return -4;
	}

	return 0;
}

int CAudioRecoder::get_nb_channels()
{
	return m_pWaveFmt->nChannels;
}

int CAudioRecoder::get_nb_bits_sample()
{
	return m_pWaveFmt->wBitsPerSample;
}

int CAudioRecoder::get_sample_rate()
{
	return m_pWaveFmt->nSamplesPerSec;
}

int CAudioRecoder::get_nb_samples()
{
	return m_nb_samples;
}

bool CAudioRecoder::reInit()
{
	Release();
	return Init();
}

bool CAudioRecoder::Init()
{
	m_pDevice = GetDefaultDevice();
	if (!m_pDevice)
		return false;

	HRESULT hr = S_FALSE;
	DWORD dwTaskIndex = 0;

	do 
	{
		hr = m_pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, reinterpret_cast<void**>(&m_pAudioClient));
		if (FAILED(hr))
			break;

		hr = m_pAudioClient->GetMixFormat(&m_pWaveFmt);
		if (FAILED(hr))
			break;

		AdjustFormat(m_pWaveFmt);

		if (SOUNDCARD == m_CaptureType)
			hr = m_pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK, 0, 0, m_pWaveFmt, 0);
		else
			hr = m_pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, 0, 0, m_pWaveFmt, 0);
		if(FAILED(hr))
			break;

		if (FAILED(m_pAudioClient->GetBufferSize(&m_nb_samples)))
			break;

		if (FAILED(m_pAudioClient->GetService(__uuidof(IAudioCaptureClient), reinterpret_cast<void**>(&m_pAudioCaptureClient))))
			break;

		if (SOUNDCARD == m_CaptureType)
			m_hTask = AvSetMmThreadCharacteristics(_T("Capture"), &dwTaskIndex);
		else
			m_hTask = AvSetMmThreadCharacteristics(_T("Audio"), &dwTaskIndex);
		if (!m_hTask)
			break;

		if (FAILED(m_pAudioClient->Start()))
			break;

		m_Inited = true;

	} while (false);
	
	return true;
}

void CAudioRecoder::Release()
{
	if (m_hTask)
	{
		AvRevertMmThreadCharacteristics(m_hTask);
		m_hTask = nullptr;
	}

	if (m_pAudioCaptureClient)
	{
		m_pAudioCaptureClient->Release();
		m_pAudioCaptureClient = nullptr;
	}

	if (m_pWaveFmt)
	{
		CoTaskMemFree(m_pWaveFmt);
		m_pWaveFmt = nullptr;
	}

	if (m_pAudioClient)
	{
		if (m_Inited)
			m_pAudioClient->Stop();

		m_pAudioClient->Release();
		m_pAudioClient = nullptr;
	}

	if (m_pDevice)
	{
		m_pDevice->Release();
		m_pDevice = nullptr;
	}

	m_Inited = false;
}

IMMDevice* CAudioRecoder::GetDefaultDevice()
{
	IMMDevice* pDevice = nullptr;
	IMMDeviceEnumerator* pMMDeviceEnum = nullptr;

	HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, 
		CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pMMDeviceEnum);
	if (FAILED(hr))
		return nullptr;

	if (SOUNDCARD == m_CaptureType)
		hr = pMMDeviceEnum->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
	else
		hr = pMMDeviceEnum->GetDefaultAudioEndpoint(eCapture, eConsole, &pDevice);

	if (pMMDeviceEnum)
	{
		pMMDeviceEnum->Release();
		pMMDeviceEnum = nullptr;
	}

	return pDevice;
}

bool CAudioRecoder::AdjustFormat(WAVEFORMATEX* pwfx)
{
	if (pwfx->wFormatTag == WAVE_FORMAT_IEEE_FLOAT) {
		pwfx->wFormatTag = WAVE_FORMAT_PCM;
	}
	else if (pwfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
	{
		PWAVEFORMATEXTENSIBLE pEx = reinterpret_cast<PWAVEFORMATEXTENSIBLE>(pwfx);
		if (IsEqualGUID(KSDATAFORMAT_SUBTYPE_IEEE_FLOAT, pEx->SubFormat))
		{
			pEx->SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
			pEx->Samples.wValidBitsPerSample = 16;
		}
	}

	pwfx->wBitsPerSample = 16;
	pwfx->nBlockAlign = pwfx->nChannels * pwfx->wBitsPerSample / 8;
	pwfx->nAvgBytesPerSec = pwfx->nBlockAlign * pwfx->nSamplesPerSec;

	return true;
}
