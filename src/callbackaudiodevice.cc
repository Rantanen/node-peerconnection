
#include "callbackaudiodevice.h"
#include "third_party/webrtc/modules/audio_device/audio_device_utility.h"

using namespace webrtc;

CallbackAudioDevice::CallbackAudioDevice( CallbackAudioDeviceObserver* observer ):
	isPlaying(false), isRecording(false),
	lastProcess(0), processInterval(5),
	_audioCallback(0), _deviceObserver(observer)
{
	TRACE_CALL;
}

CallbackAudioDevice::~CallbackAudioDevice() {
}

int32_t CallbackAudioDevice::RegisterAudioCallback( webrtc::AudioTransport* audioCallback ) {
	_audioCallback = audioCallback;
	return 0;
}

int32_t CallbackAudioDevice::TimeUntilNextProcess() {
	if( lastProcess == 0 ) { return 0; }
	return lastProcess + processInterval - webrtc::AudioDeviceUtility::GetTimeInMS();
}

int32_t CallbackAudioDevice::Process() {
	int8_t recordBuffer[ 3840 ];
	uint32_t samplesOut(0);

	if( isPlaying ) {
		_audioCallback->NeedMorePlayData( 480, 2, 1, 48000, &recordBuffer[0], samplesOut );
		_deviceObserver->OnAudio( recordBuffer, samplesOut*2 );
	}

	lastProcess = webrtc::AudioDeviceUtility::GetTimeInMS();
	return 0;
}

void CallbackAudioDevice::Transmit( int8_t* sampleBuffer, uint32_t count ) {
	if( isRecording ) {
		uint32_t newMicLevel(0);
		_audioCallback->RecordedDataIsAvailable(
				&sampleBuffer[0],
				count,
				2, 1, 48000, 0, 0, 0, newMicLevel );
	}
}

int32_t CallbackAudioDevice::StartPlayout() {
	TRACE_CALL;
	isPlaying = true;
	return 0;
}

int32_t CallbackAudioDevice::StopPlayout() {
	TRACE_CALL;
	isPlaying = false;
	return 0;
};

bool CallbackAudioDevice::Playing() const {
	TRACE_CALL;
	return isPlaying;
};

int32_t CallbackAudioDevice::StartRecording() {
	TRACE_CALL;
	isRecording = true;
	return 0;
}
int32_t CallbackAudioDevice::StopRecording() {
	TRACE_CALL;
	isRecording = false;
	return 0;
}
bool CallbackAudioDevice::Recording() const {
	TRACE_CALL;
	return isRecording;
}

