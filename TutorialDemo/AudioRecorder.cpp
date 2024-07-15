#include "RecordTest.h"
#include "AudioRecorder.h"
#include <avrt.h>


int64_t getCurTimestamp()// 获取毫秒级时间戳（13位）
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

static std::string GetCurrTimeString()
{
	const char* time_fmt = "%Y-%m-%d %H:%M:%S";
	time_t t = time(nullptr);
	char time_str[64];
	struct tm out_tm;
	localtime_s(&out_tm, &t);
	strftime(time_str, sizeof(time_str), time_fmt, &out_tm);

	return time_str;
}

CAudioRecorder::CAudioRecorder(int CaptureType)
{
	CoInitialize(nullptr);
}

CAudioRecorder::~CAudioRecorder()
{
	Stop();
	Release();
	CoUninitialize();
}

void CAudioRecorder::Start(IRecordEvent* pEvt)
{
	m_pRecordEvent = pEvt;

	m_bRun = true;
	m_thread = std::thread(&CAudioRecorder::Work, this);
}

void CAudioRecorder::Stop()
{
	m_bRun = false;
	if (m_thread.joinable())
		m_thread.join();
}



int CAudioRecorder::get_nb_channels()
{
	return m_pWaveFmt->nChannels;
}

int CAudioRecorder::get_nb_bits_sample()
{
	return m_pWaveFmt->wBitsPerSample;
}

int CAudioRecorder::get_sample_rate()
{
	return m_pWaveFmt->nSamplesPerSec;
}

int CAudioRecorder::get_nb_samples()
{
	return m_nb_samples;
}

bool CAudioRecorder::reInit()
{
	Release();
	return Init(m_CaptureType);
}

bool CAudioRecorder::Init(int CapType)
{
	m_CaptureType = CapType;

	m_pDevice = GetDefaultDevice();
	if (!m_pDevice)
		return false;

	HRESULT hr = S_FALSE;
	DWORD dwTaskIndex = 0;

	hr = m_pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, reinterpret_cast<void**>(&m_pAudioClient));
	if (FAILED(hr)) return false;

	hr = m_pAudioClient->GetMixFormat(&m_pWaveFmt);
	if (FAILED(hr)) return false;

	AdjustFormat(m_pWaveFmt);

	if (SOUNDCARD == m_CaptureType)
		hr = m_pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK, 1000000, 0, m_pWaveFmt, 0);
	else
		hr = m_pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, 1000000, 0, m_pWaveFmt, 0);
	if (FAILED(hr)) return false;

	if (FAILED(m_pAudioClient->GetBufferSize(&m_nb_samples)))
		return false;

	if (FAILED(m_pAudioClient->GetService(__uuidof(IAudioCaptureClient), reinterpret_cast<void**>(&m_pAudioCaptureClient))))
		return false;

	if (SOUNDCARD == m_CaptureType)
		m_hTask = AvSetMmThreadCharacteristics(_T("Capture"), &dwTaskIndex);
	else
		m_hTask = AvSetMmThreadCharacteristics(_T("Audio"), &dwTaskIndex);
	if (!m_hTask)
	{
		DWORD dwErr = GetLastError();
		printf("AvSetMmThreadCharacteristics failed with error: %d\r\n", dwErr);
		return false;
	}

	if (FAILED(m_pAudioClient->Start()))
		return false;

	m_Inited = true;

	printf("采样声道数=%d\n", m_pWaveFmt->nChannels);
	printf("采样声道存储比特数=%d\n", m_pWaveFmt->wBitsPerSample);
	printf("采样率=%d\n", m_pWaveFmt->nSamplesPerSec);
	printf("一帧音频的采样点数量=%d\n", m_nb_samples);

	return true;
}

void CAudioRecorder::Release()
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

IMMDevice* CAudioRecorder::GetDefaultDevice()
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

bool CAudioRecorder::AdjustFormat(WAVEFORMATEX* pwfx)
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

void CAudioRecorder::Work()
{
	HRESULT hr = 0;
	UINT32 nextPacketSize = 0;
	UINT32 numFramesToRead;
	DWORD  dwFlags;
	uint8_t* pData = nullptr;

	uint8_t buffer[10000] = { 0 };
	int64_t timestamp = getCurTimestamp();
	int i = 0;
	int size = 0;

	while (m_bRun)
	{
		hr = m_pAudioCaptureClient->GetNextPacketSize(&nextPacketSize);
		if (FAILED(hr)) {
			m_bRun = false;
			return;
		}

		if (nextPacketSize != 0)
		{
			hr = m_pAudioCaptureClient->GetBuffer(&pData, &numFramesToRead, &dwFlags, nullptr, nullptr);
			if (FAILED(hr))
			{
				//录制系统声音时，扬声器突然停止播放触发的错误
				m_bRun = false;
				return ;
			}

			if (dwFlags & AUDCLNT_BUFFERFLAGS_SILENT)
			{
				pData = nullptr;
			}

			if (!pData) 
			{
				m_bRun = false;
				return;
			}

			int pDataLen = numFramesToRead * m_pWaveFmt->nBlockAlign;
			if (m_pRecordEvent)
				m_pRecordEvent->AudioBufEvent(pData, numFramesToRead, getCurTimestamp());

			m_pAudioCaptureClient->ReleaseBuffer(numFramesToRead);
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			continue;
		}
	}
}
