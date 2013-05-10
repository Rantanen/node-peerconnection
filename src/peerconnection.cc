
#include <node_buffer.h>

#include "peerconnection.h"
#include "talk/app/webrtc/jsep.h"
#include "third_party/webrtc/system_wrappers/interface/ref_count.h"

#include "common.h"

Persistent<Function> PeerConnection::constructor;

struct EmitIceCandidateParams {
	EmitIceCandidateParams( const webrtc::IceCandidateInterface* iceCandidate ) {
		sdpMid = iceCandidate->sdp_mid();
		sdpMLineIndex = iceCandidate->sdp_mline_index();
		iceCandidate->ToString( &candidate );
	};

	std::string sdpMid;
	int sdpMLineIndex;
	std::string candidate;
};

struct EmitSessionDescriptionParams {
	EmitSessionDescriptionParams( const webrtc::SessionDescriptionInterface* sessionDescription ) {
		type = sessionDescription->type();
		sessionDescription->ToString( &sdp );
	};

	std::string type;
	std::string sdp;
};

struct EmitAudioParams {
	EmitAudioParams( int8_t* inData, unsigned int inSize ) {
		size = inSize;
		data = new char[ size ];
		memcpy( data, inData, size );
	};
	~EmitAudioParams() { delete data; }
	char* data;
	unsigned int size;
};

//
// Callback observers
//

void CreateSessionDescriptionObserver::OnSuccess( webrtc::SessionDescriptionInterface* sdp ) {
	parent->OnCreateSessionDescriptionSuccess( sdp );
}
void CreateSessionDescriptionObserver::OnFailure( const std::string& msg ) {
	parent->OnCreateSessionDescriptionFailure( msg );
}

void SetRemoteDescriptionObserver::OnSuccess() {
	parent->OnSetRemoteDescriptionSuccess();
}
void SetRemoteDescriptionObserver::OnFailure( const std::string& msg ) {
	parent->OnSetRemoteDescriptionFailure( msg );
}

//
// PeerConnection
//

PeerConnection::PeerConnection()
{
	// Init the uvlib stuff.
	uv_mutex_init( &eventLock );
	uv_async_init( uv_default_loop(), &emitAsync, EmitAsync );

	// Create the observers.
	_createSessionDescriptionObserver = new talk_base::RefCountedObject<CreateSessionDescriptionObserver>( this );
	_setRemoteDescriptionObserver = new talk_base::RefCountedObject<SetRemoteDescriptionObserver>( this );

	// Init the PeerConnectionFactory
	_signalThread = new talk_base::Thread;
	_workerThread = new talk_base::Thread;

	_signalThread->Start();
	_workerThread->Start();

	_audioDevice =
		new webrtc::RefCountImpl<CallbackAudioDevice>( this );

	_peerConnectionFactory = webrtc::CreatePeerConnectionFactory(
			_signalThread, _workerThread, _audioDevice, NULL );

	// Create the PeerConnection
	_peerConnection =
		_peerConnectionFactory->CreatePeerConnection( _iceServers, NULL, this );

	// Set up the streams.
	talk_base::scoped_refptr<webrtc::AudioTrackInterface> audio_track(
			_peerConnectionFactory->CreateAudioTrack(
				"audio_label", _peerConnectionFactory->CreateAudioSource(NULL)));

	talk_base::scoped_refptr<webrtc::MediaStreamInterface> stream =
			_peerConnectionFactory->CreateLocalMediaStream("stream_label");

	stream->AddTrack(audio_track);
	_peerConnection->AddStream( stream, NULL );
}

PeerConnection::~PeerConnection()
{
	delete _signalThread;
	delete _workerThread;
}

void PeerConnection::SetPeerConnection( webrtc::PeerConnectionInterface* peerConnection ) {
	ASSERT( peerConnection );
	ASSERT( _peerConnection.get() == NULL );
	_peerConnection = peerConnection;
}

void PeerConnection::SetRemoteDescription( webrtc::SessionDescriptionInterface* desc ) {
	_peerConnection->SetRemoteDescription( _setRemoteDescriptionObserver, desc );

	// If the remote description is an offer, create an answer.
	if( desc->type() == webrtc::SessionDescriptionInterface::kOffer ) {
		_peerConnection->CreateAnswer( _createSessionDescriptionObserver, NULL );
	}
}

void PeerConnection::OnCreateSessionDescriptionSuccess( webrtc::SessionDescriptionInterface* sdp ) {
	_peerConnection->SetLocalDescription( _setRemoteDescriptionObserver, sdp );
	Emit( emitAnswer, new EmitSessionDescriptionParams( sdp ) );
}

void PeerConnection::OnCreateSessionDescriptionFailure( const std::string& msg ) {
}

void PeerConnection::OnSetRemoteDescriptionSuccess() {
}

void PeerConnection::OnSetRemoteDescriptionFailure( const std::string& msg ) {
}

void PeerConnection::Emit( EmitType type, void* data ) {
	TRACE_CALL;

	AsyncEvent asyncEvent;
	asyncEvent.type = type;
	asyncEvent.data = data;
	emitAsync.data = this;

	uv_mutex_lock( &eventLock );
	_events.push( asyncEvent );
	uv_mutex_unlock( &eventLock );

	uv_async_send( &emitAsync );

}

void PeerConnection::EmitAsync( uv_async_t* req, int status ) {
	TRACE_CALL;
	HandleScope handle_scope;

	PeerConnection* self = static_cast<PeerConnection*>( req->data );

	while( !self->_events.empty() ) {

		AsyncEvent event = self->_events.front();
		self->_events.pop();

		// Placeholders for the emit parameters.
		Local<Object> evt = Object::New();
		Local<String> eventName;

		EmitIceCandidateParams* candidateParams = NULL;
		EmitSessionDescriptionParams* sdpParams = NULL;
		EmitAudioParams* audioParams = NULL;

		// Resolve the emit type.
		switch (event.type) {

			// New ICE Candidate.
			case emitIceCandidate:
				TRACE( "ICE Candidate" );
				eventName = String::New( "iceCandidate" );

				candidateParams =
					static_cast< EmitIceCandidateParams* >( event.data );
				event.data = 0;

				evt->Set( String::New( "sdpMid" ), String::New( candidateParams->sdpMid.c_str() ) );
				evt->Set( String::New( "sdpMLineIndex" ), Number::New( candidateParams->sdpMLineIndex ) );
				evt->Set( String::New( "candidate" ), String::New( candidateParams->candidate.c_str() ) );

				delete candidateParams;
				break;

			case emitAnswer:
				TRACE( "Answer" );
				eventName = String::New( "answer" );

				sdpParams = static_cast< EmitSessionDescriptionParams* >( event.data );
				event.data = 0;

				evt->Set( String::New( "type" ), String::New( sdpParams->type.c_str() ) );
				evt->Set( String::New( "sdp" ), String::New( sdpParams->sdp.c_str() ) );

				delete candidateParams;
				break;
			case emitAudio:
				TRACE( "Audio" );
				eventName = String::New( "audio" );

				audioParams = static_cast< EmitAudioParams* >( event.data );
				event.data = 0;

				// Construct the buffer.
				Buffer *slowBuffer = Buffer::New( audioParams->data, audioParams->size );
				Local<Object> globalObj = Context::GetCurrent()->Global();
				Local<Function> bufferCtor = Local<Function>::Cast( globalObj->Get( String::New("Buffer" )));
				Handle<Value> ctorArgs[3] = { slowBuffer->handle_, Integer::New( audioParams->size ), Integer::New(0) };
				Local<Object> actualBuffer = bufferCtor->NewInstance( 3, ctorArgs );
				evt->Set( String::New( "data" ), actualBuffer );

				delete audioParams;
				break;
		}

		// Callback
		Local<Value> argv[2] = { eventName, evt };
		MakeCallback( self->handle_, "emit", 2, argv );
	}
}


void PeerConnection::OnError() {
}

void PeerConnection::OnStateChange( StateType state_changed ) {
}

void PeerConnection::OnAddStream( webrtc::MediaStreamInterface* stream ) {
}

void PeerConnection::OnRemoveStream( webrtc::MediaStreamInterface* stream ) {
}

void PeerConnection::OnAudio( int8_t* buffer, unsigned int size ) {
	Emit( emitAudio, new EmitAudioParams( buffer, size ) );
}

void PeerConnection::OnIceCandidate( const webrtc::IceCandidateInterface* candidate ) {
	TRACE_CALL;
	Emit( emitIceCandidate, new EmitIceCandidateParams( candidate ) );
}

Handle<Value> PeerConnection::New( const Arguments& args ) {
	HandleScope scope;

	if( !args.IsConstructCall()) {
		return ThrowException(Exception::TypeError(
					String::New("Use the new operator to construct the PeerConnection.")));
	}

	PeerConnection* obj = new talk_base::RefCountedObject<PeerConnection>();
	obj->Wrap( args.This() );

	return args.This();
}

Handle<Value> PeerConnection::SetRemoteDescription( const Arguments& args ) {
	HandleScope scope;

	REQ_OBJ_ARG( 0, desc );

	String::Utf8Value type( desc->Get( String::NewSymbol( "type" ) )->ToString() );
	String::Utf8Value sdp( desc->Get( String::NewSymbol( "sdp" ) )->ToString() );

	webrtc::SessionDescriptionInterface* session_description(
			webrtc::CreateSessionDescription(
					std::string( *type ),
					std::string( *sdp )));

	if( !session_description ) {
		ThrowException( Exception::TypeError( String::New("Could not parse session description") ) );
		return scope.Close(Undefined());
	};

	PeerConnection* rtcConnection = ObjectWrap::Unwrap<PeerConnection>( args.This() );
	rtcConnection->SetRemoteDescription( session_description );

	return scope.Close( Undefined() );
}


Handle<Value> PeerConnection::AddIceCandidate( const Arguments& args ) {
	HandleScope scope;

	REQ_OBJ_ARG( 0, iceCandidate );
	
	String::Utf8Value sdpMid( iceCandidate->Get( String::NewSymbol( "sdpMid" ) )->ToString() );
	String::Utf8Value candidate( iceCandidate->Get( String::NewSymbol( "candidate" ) )->ToString() );
	int sdpMLineIndex( iceCandidate->Get( String::NewSymbol( "sdpMLineIndex" ) )->ToNumber()->IntegerValue() );

	TRACE_S( "sdpMid", *sdpMid );
	TRACE_S( "candidate", *candidate );
	TRACE_I( "sdpMLineIndex", sdpMLineIndex );

	talk_base::scoped_ptr< webrtc::IceCandidateInterface > iceItf(
			webrtc::CreateIceCandidate(
					std::string( *sdpMid ),
					sdpMLineIndex,
					std::string( *candidate )
			));

	if( !iceItf.get() ) {
		return ThrowException( Exception::TypeError(
					String::New("Failed to parse candidate!" )));
	}


	PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( args.This() );
	if( !self->_peerConnection->AddIceCandidate( iceItf.get() ) ) {
		return ThrowException( Exception::TypeError(
					String::New("Failed to add candidate!" )));
	}

	return scope.Close( Undefined() );
}

Handle<Value> PeerConnection::Transmit( const Arguments& args ) {
	HandleScope scope;

	REQ_OBJ_ARG( 0, pcmBuffer );

	char* pcmData = Buffer::Data( pcmBuffer );
	int dataLength = Buffer::Length( pcmBuffer );
	int8_t* data = reinterpret_cast<int8_t*>( pcmData );

	PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( args.This() );
	self->_audioDevice->Transmit( data, dataLength / 2 );

	return scope.Close( Undefined() );
}

void PeerConnection::Init( Handle<Object> exports ) {
	Local<FunctionTemplate> tpl = FunctionTemplate::New( New );
	tpl->SetClassName( String::NewSymbol( "PeerConnection" ) );
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	tpl->PrototypeTemplate()->Set( String::NewSymbol( "addIceCandidate" ),
			FunctionTemplate::New( AddIceCandidate )->GetFunction() );

	tpl->PrototypeTemplate()->Set( String::NewSymbol( "transmit" ),
			FunctionTemplate::New( Transmit )->GetFunction() );

	// Add the 'acceptCall' method.
	tpl->PrototypeTemplate()->Set( String::NewSymbol( "setRemoteDescription" ),
			FunctionTemplate::New( SetRemoteDescription )->GetFunction() );

	Persistent<Function> ctor = Persistent<Function>::New( tpl->GetFunction() );
	exports->Set( String::NewSymbol("PeerConnection"), ctor );

}
