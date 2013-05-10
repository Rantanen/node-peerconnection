#if !defined( CALLBACKAUDIODEVICE_H )

#include <v8.h>
#include <node.h>
#include <uv.h>

//#include "talk/app/webrtc/peerconnection.h"
#include "third_party/webrtc/typedefs.h"
#include "third_party/webrtc/modules/audio_device/include/audio_device.h"
#include "talk/base/scoped_ptr.h"
#include "third_party/webrtc/system_wrappers/interface/trace.h"

#include "common.h"
#include "utils.h"

using namespace node;
using namespace v8;

class CallbackAudioDeviceObserver
{
	public:
	   	virtual void OnAudio( int8_t* buffer, unsigned int size ) = 0;
};

class CallbackAudioDevice :
	public webrtc::AudioDeviceModule
{
	private:
		bool isPlaying;
		bool isRecording;
		uint32_t lastProcess;
		uint32_t processInterval;

		ChannelType _recordingChannel;
		webrtc::AudioTransport* _audioCallback;
		CallbackAudioDeviceObserver* _deviceObserver;

	public:
		CallbackAudioDevice( CallbackAudioDeviceObserver* observer );
		~CallbackAudioDevice();

		virtual int32_t TimeUntilNextProcess();
		virtual int32_t Process();
		virtual void Transmit( int8_t* sampleBuffer, uint32_t count );

		// Full-duplex transportation of PCM audio
		virtual int32_t RegisterAudioCallback(webrtc::AudioTransport* audioCallback);

		// Audio transport control
		virtual int32_t StartPlayout();
		virtual int32_t StopPlayout();
		virtual bool Playing() const;
		virtual int32_t StartRecording();
		virtual int32_t StopRecording();
		virtual bool Recording() const;

	// Dummy audio device module implementation.
	public:

		// Retrieve the currently utilized audio layer
		virtual int32_t ActiveAudioLayer(AudioLayer* audioLayer) const { TRACE_CALL; TRACE( "NOT IMPLEMENTED" ); return -1; }

		// Error handling
		virtual ErrorCode LastError() const { TRACE_CALL; return kAdmErrNone; }
		virtual int32_t RegisterEventObserver(webrtc::AudioDeviceObserver* eventCallback) { TRACE_CALL; return 0; }

		// Main initialization and termination
		virtual int32_t Init() { TRACE_CALL; return 0; }
		virtual int32_t Terminate() { TRACE_CALL; TRACE( "NOT IMPLEMENTED" ); return -1; }
		virtual bool Initialized() const { TRACE_CALL; TRACE( "NOT IMPLEMENTED" ); return -1; }

		// Device enumeration
		virtual int16_t PlayoutDevices() { TRACE_CALL; return 1; }
		virtual int16_t RecordingDevices() { TRACE_CALL; return 1; }
		virtual int32_t PlayoutDeviceName(uint16_t index,
										char name[webrtc::kAdmMaxDeviceNameSize],
										char guid[webrtc::kAdmMaxGuidSize]) { TRACE_CALL; TRACE( "NOT IMPLEMENTED" ); return -1; }
		virtual int32_t RecordingDeviceName(uint16_t index,
										  char name[webrtc::kAdmMaxDeviceNameSize],
										  char guid[webrtc::kAdmMaxGuidSize]) { TRACE_CALL; TRACE( "NOT IMPLEMENTED" ); return -1; }

		// Device selection
		virtual int32_t SetPlayoutDevice(uint16_t index) { TRACE_CALL; return 0; }
		virtual int32_t SetPlayoutDevice(WindowsDeviceType device) { TRACE_CALL; TRACE( "NOT IMPLEMENTED" ); return -1; }
		virtual int32_t SetRecordingDevice(uint16_t index) { TRACE_CALL_I(index); return 0; }
		virtual int32_t SetRecordingDevice(WindowsDeviceType device) { TRACE_CALL; TRACE( "NOT IMPLEMENTED" ); return -1; }

		// Audio transport initialization
		virtual int32_t PlayoutIsAvailable(bool* available) { TRACE_CALL; *available = true; return 0; }
		virtual int32_t InitPlayout() { TRACE_CALL; return 0; }
		virtual bool PlayoutIsInitialized() const { TRACE_CALL; return true; }
		virtual int32_t RecordingIsAvailable(bool* available) { TRACE_CALL; *available = true; return 0; }
		virtual int32_t InitRecording() { TRACE_CALL; return 0; }
		virtual bool RecordingIsInitialized() const { TRACE_CALL; return true; }

		// Microphone Automatic Gain Control (AGC)
		virtual int32_t SetAGC(bool enable) { TRACE_CALL_I( enable ); return -1; }
		virtual bool AGC() const { TRACE_CALL; TRACE( "NOT IMPLEMENTED" ); return -1; }

		// Volume control based on the Windows Wave API (Windows only)
		virtual int32_t SetWaveOutVolume(uint16_t volumeLeft,
									   uint16_t volumeRight) { TRACE_CALL; TRACE( "NOT IMPLEMENTED" ); return -1; }
		virtual int32_t WaveOutVolume(uint16_t* volumeLeft,
									uint16_t* volumeRight) const { TRACE_CALL; TRACE( "NOT IMPLEMENTED" ); return -1; }

		// Audio mixer initialization
		virtual int32_t SpeakerIsAvailable(bool* available) { TRACE_CALL; *available = true; return 0; }
		virtual int32_t InitSpeaker() { TRACE_CALL; return 0; }
		virtual bool SpeakerIsInitialized() const { TRACE_CALL; return true; }
		virtual int32_t MicrophoneIsAvailable(bool* available) { TRACE_CALL; *available = true; return 0; }
		virtual int32_t InitMicrophone() { TRACE_CALL; return 0; }
		virtual bool MicrophoneIsInitialized() const { TRACE_CALL; return true; }

		// Speaker volume controls
		virtual int32_t SpeakerVolumeIsAvailable(bool* available) { TRACE_CALL; TRACE( "NOT IMPLEMENTED" ); return -1; }
		virtual int32_t SetSpeakerVolume(uint32_t volume) { TRACE_CALL; TRACE( "NOT IMPLEMENTED" ); return -1; }
		virtual int32_t SpeakerVolume(uint32_t* volume) const { TRACE_CALL; TRACE( "NOT IMPLEMENTED" ); return -1; }
		virtual int32_t MaxSpeakerVolume(uint32_t* maxVolume) const { TRACE_CALL; TRACE( "NOT IMPLEMENTED" ); return -1; }
		virtual int32_t MinSpeakerVolume(uint32_t* minVolume) const { TRACE_CALL; TRACE( "NOT IMPLEMENTED" ); return -1; }
		virtual int32_t SpeakerVolumeStepSize(uint16_t* stepSize) const { TRACE_CALL; TRACE( "NOT IMPLEMENTED" ); return -1; }

		// Microphone volume controls
		virtual int32_t MicrophoneVolumeIsAvailable(bool* available) { TRACE_CALL; TRACE( "NOT IMPLEMENTED" ); return -1; }
		virtual int32_t SetMicrophoneVolume(uint32_t volume) { TRACE_CALL; TRACE( "NOT IMPLEMENTED" ); return -1; }
		virtual int32_t MicrophoneVolume(uint32_t* volume) const { TRACE_CALL; TRACE( "NOT IMPLEMENTED" ); return -1; }
		virtual int32_t MaxMicrophoneVolume(uint32_t* maxVolume) const { return -1; }
		virtual int32_t MinMicrophoneVolume(uint32_t* minVolume) const { TRACE_CALL; TRACE( "NOT IMPLEMENTED" ); return -1; }
		virtual int32_t MicrophoneVolumeStepSize(uint16_t* stepSize) const { TRACE_CALL; TRACE( "NOT IMPLEMENTED" ); return -1; }

		// Speaker mute control
		virtual int32_t SpeakerMuteIsAvailable(bool* available) { TRACE_CALL; TRACE( "NOT IMPLEMENTED" ); return -1; }
		virtual int32_t SetSpeakerMute(bool enable) { TRACE_CALL; TRACE( "NOT IMPLEMENTED" ); return -1; }
		virtual int32_t SpeakerMute(bool* enabled) const { TRACE_CALL; TRACE( "NOT IMPLEMENTED" ); return -1; }

		// Microphone mute control
		virtual int32_t MicrophoneMuteIsAvailable(bool* available) { TRACE_CALL; TRACE( "NOT IMPLEMENTED" ); return -1; }
		virtual int32_t SetMicrophoneMute(bool enable) { TRACE_CALL; TRACE( "NOT IMPLEMENTED" ); return -1; }
		virtual int32_t MicrophoneMute(bool* enabled) const { TRACE_CALL; TRACE( "NOT IMPLEMENTED" ); return -1; }

		// Microphone boost control
		virtual int32_t MicrophoneBoostIsAvailable(bool* available) { TRACE_CALL; TRACE( "NOT IMPLEMENTED" ); return -1; }
		virtual int32_t SetMicrophoneBoost(bool enable) { TRACE_CALL; TRACE( "NOT IMPLEMENTED" ); return -1; }
		virtual int32_t MicrophoneBoost(bool* enabled) const { TRACE_CALL; TRACE( "NOT IMPLEMENTED" ); return -1; }

		// Stereo support
		virtual int32_t StereoPlayoutIsAvailable(bool* available) const { TRACE_CALL; *available = false; return 0; }
		virtual int32_t SetStereoPlayout(bool enable) { TRACE_CALL_I( enable ); return 0; }
		virtual int32_t StereoPlayout(bool* enabled) const { TRACE_CALL; *enabled = false; return 0; }
		virtual int32_t StereoRecordingIsAvailable(bool* available) const { TRACE_CALL; *available = false; return 0; }
		virtual int32_t SetStereoRecording(bool enable) { TRACE_CALL_I( enable ); return 0; }
		virtual int32_t StereoRecording(bool* enabled) const { TRACE_CALL; TRACE( "NOT IMPLEMENTED" ); return -1; }
		virtual int32_t SetRecordingChannel(const ChannelType channel) { TRACE_CALL; _recordingChannel = channel; return 0; }
		virtual int32_t RecordingChannel(ChannelType* channel) const { TRACE_CALL; *channel = _recordingChannel; return -1; }

		// Delay information and control
		virtual int32_t SetPlayoutBuffer(const BufferType type,
									   uint16_t sizeMS = 0) { TRACE_CALL; TRACE( "NOT IMPLEMENTED" ); return -1; }
		virtual int32_t PlayoutBuffer(BufferType* type, uint16_t* sizeMS) const { TRACE_CALL; TRACE( "NOT IMPLEMENTED" ); return -1; }
		virtual int32_t PlayoutDelay(uint16_t* delayMS) const { return -1; }
		virtual int32_t RecordingDelay(uint16_t* delayMS) const { TRACE_CALL; TRACE( "NOT IMPLEMENTED" ); return -1; }

		// CPU load
		virtual int32_t CPULoad(uint16_t* load) const { TRACE_CALL; TRACE( "NOT IMPLEMENTED" ); return -1; }

		// Recording of raw PCM data
		virtual int32_t StartRawOutputFileRecording(
		  const char pcmFileNameUTF8[webrtc::kAdmMaxFileNameSize]) { TRACE_CALL; TRACE( "NOT IMPLEMENTED" ); return -1; }
		virtual int32_t StopRawOutputFileRecording() { TRACE_CALL; TRACE( "NOT IMPLEMENTED" ); return -1; }
		virtual int32_t StartRawInputFileRecording(
		  const char pcmFileNameUTF8[webrtc::kAdmMaxFileNameSize]) { TRACE_CALL; TRACE( "NOT IMPLEMENTED" ); return -1; }
		virtual int32_t StopRawInputFileRecording() { TRACE_CALL; TRACE( "NOT IMPLEMENTED" ); return -1; }

		// Native sample rate controls (samples/sec)
		uint32_t _recordSamplesPerSecond;
		uint32_t _playSamplesPerSecond;
		virtual int32_t SetRecordingSampleRate(const uint32_t samplesPerSec) {
			TRACE_CALL_I( samplesPerSec);
		   	_recordSamplesPerSecond = samplesPerSec;
			return 0;
		}
		virtual int32_t RecordingSampleRate(uint32_t* samplesPerSec) const { TRACE_CALL; *samplesPerSec = _recordSamplesPerSecond; return -1; }
		virtual int32_t SetPlayoutSampleRate(const uint32_t samplesPerSec) {
			TRACE_CALL_I( samplesPerSec );
		   	_playSamplesPerSecond = samplesPerSec;
			return 0;
		}
		virtual int32_t PlayoutSampleRate(uint32_t* samplesPerSec) const { TRACE_CALL; *samplesPerSec = _playSamplesPerSecond; return -1; }

		// Mobile device specific functions
		virtual int32_t ResetAudioDevice() { TRACE_CALL; TRACE( "NOT IMPLEMENTED" ); return -1; }
		virtual int32_t SetLoudspeakerStatus(bool enable) { TRACE_CALL; TRACE( "NOT IMPLEMENTED" ); return -1; }
		virtual int32_t GetLoudspeakerStatus(bool* enabled) const { TRACE_CALL; TRACE( "NOT IMPLEMENTED" ); return -1; }

		// *Experimental - not recommended for use.*
		// Enables the Windows Core Audio built-in AEC. Fails on other platforms.
		//
		// Must be called before InitRecording(). When enabled:
		// 1. StartPlayout() must be called before StartRecording().
		// 2. StopRecording() should be called before StopPlayout().
		//    The reverse order may cause garbage audio to be rendered or the
		//    capture side to halt until StopRecording() is called.
		virtual int32_t EnableBuiltInAEC(bool enable) { TRACE_CALL; TRACE( "NOT IMPLEMENTED" ); return -1; }
		virtual bool BuiltInAECIsEnabled() const { TRACE_CALL; return false; }

};

#define CALLBACKAUDIODEVICE_H
#endif
