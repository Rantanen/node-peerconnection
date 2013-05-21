#include <v8.h>
#include <node.h>
#include <uv.h>

#include <queue>

//#include "talk/app/webrtc/peerconnection.h"
#include "talk/app/webrtc/peerconnectioninterface.h"
#include "talk/base/thread.h"
#include "talk/base/scoped_ptr.h"
#include "third_party/webrtc/system_wrappers/interface/ref_count.h"

#include "callbackaudiodevice.h"
#include "utils.h"

using namespace node;
using namespace v8;

class PeerConnection;

// CreateSessionDescriptionObserver is required for Jsep callbacks.
class CreateSessionDescriptionObserver :
	public webrtc::CreateSessionDescriptionObserver
{
	private:
		PeerConnection* parent;
	public:
		CreateSessionDescriptionObserver( PeerConnection* connection ): parent(connection) {};

		virtual void OnSuccess( webrtc::SessionDescriptionInterface* sdp );
		virtual void OnFailure( const std::string& msg );
};

class SetRemoteDescriptionObserver :
	public webrtc::SetSessionDescriptionObserver
{
	private:
		PeerConnection* parent;
	public:
		SetRemoteDescriptionObserver( PeerConnection* connection): parent(connection) {};

		virtual void OnSuccess();
		virtual void OnFailure( const std::string& msg );
};

class PeerConnection :

		public ObjectWrap,

		// PeerConnectionObserver implements the main PeerConnection handling.
		public webrtc::PeerConnectionObserver,

		// Audio observer
		public CallbackAudioDeviceObserver
{

	enum EmitType {
		emitIceCandidate,
		emitAnswer,
		emitAudio,
		emitSignalingChange,
		emitIceConnectionChange,
		emitIceGatheringChange,
		emitIceStateChange,
		emitStateChange
	};

	private:
		webrtc::RefCountImpl<CallbackAudioDevice>* _audioDevice;
		webrtc::PeerConnectionInterface::IceServers _iceServers;

		talk_base::scoped_refptr<webrtc::PeerConnectionFactoryInterface> _peerConnectionFactory;
		talk_base::scoped_refptr<webrtc::PeerConnectionInterface> _peerConnection;

		talk_base::Thread* _signalThread;
		talk_base::Thread* _workerThread;

		// talk_base::scoped_refptr<webrtc::PeerConnectionInterface> peerConnection;
		talk_base::scoped_refptr<CreateSessionDescriptionObserver> _createSessionDescriptionObserver;
		talk_base::scoped_refptr<SetRemoteDescriptionObserver> _setRemoteDescriptionObserver;

		struct AsyncEvent { EmitType type; void* data; };
		std::queue< AsyncEvent > _events;
		uv_mutex_t eventLock;
		uv_async_t emitAsync;;

	public:
	
	PeerConnection();

	~PeerConnection();

	void SetPeerConnection( webrtc::PeerConnectionInterface* peerConnection );
	void SetRemoteDescription( webrtc::SessionDescriptionInterface* desc );

	void OnCreateSessionDescriptionSuccess( webrtc::SessionDescriptionInterface* sdp );
	void OnCreateSessionDescriptionFailure( const std::string& msg );
	void OnSetRemoteDescriptionSuccess();
	void OnSetRemoteDescriptionFailure( const std::string& msg );

	void Emit( EmitType type, void* data );
	static void EmitAsync( uv_async_t* req, int status );

	//
	// PeerConnectionObserver implementation.
	//

	virtual void OnError();

	// Triggered when SignalingState changed.
	virtual void OnSignalingChange( webrtc::PeerConnectionInterface::SignalingState new_state );

	virtual void OnIceConnectionChange( webrtc::PeerConnectionInterface::IceConnectionState new_state );
	virtual void OnIceGatheringChange( webrtc::PeerConnectionInterface::IceGatheringState new_state );
	virtual void OnIceStateChange( webrtc::PeerConnectionInterface::IceState new_state );


	// Triggered when SignalingState or IceState have changed.
	// TODO(bemasc): Remove once callers transition to OnSignalingChange.
	virtual void OnStateChange(StateType state_changed);

	// Triggered when media is received on a new stream from remote peer.
	virtual void OnAddStream( webrtc::MediaStreamInterface* stream );

	// Triggered when a remote peer close a stream.
	virtual void OnRemoveStream( webrtc::MediaStreamInterface* stream );

	// New Ice candidate have been found.
	virtual void OnIceCandidate(const webrtc::IceCandidateInterface* candidate );

	// 
	// CallbackAudioDeviceObserver
	//
	void OnAudio( int8_t* buffer, unsigned int size );

	//
	// Nodejs wrapping.
	//
	static Persistent<Function> constructor;
	static Handle<Value> New( const Arguments& args );
	static Handle<Value> Create( webrtc::SessionDescriptionInterface* sessionDescription );
	static Handle<Value> SetRemoteDescription( const Arguments& args );
	static Handle<Value> AddIceCandidate( const Arguments& args );
	static Handle<Value> Transmit( const Arguments& args );
	static Handle<Value> Close( const Arguments& args );
	static void Init( Handle<Object> exports );
};
