#pragma once
#include "../Common.h"

extern char* dup_wchar_to_utf8(const wchar_t* w);

namespace capture
{
	class IAudioEvent
	{
	public:
		virtual bool AudioEvent(AVFrame* frame) = 0;
	};

	class CAudioDecoder
	{
	public:
		CAudioDecoder();
		~CAudioDecoder();

		bool Init();

		bool Start(IAudioEvent* pEvt);

		void Release();

	private:
		void OnDecodeFunction();

	private:
		wchar_t* GetMicrophoneName();

	private:
		AVFormatContext* m_pFormatCtx = nullptr;
		AVCodecContext* m_pCodecCtx = nullptr;
	
		IAudioEvent* m_pEvent = nullptr;

		enum RecordState m_state = NotStarted;

		int m_audioIndex = -1;

		std::thread m_thread;
	};
};
