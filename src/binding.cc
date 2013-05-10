
#include <node.h>
#include <v8.h>

#include "peerconnection.h"

using namespace v8;

void InitAll( Handle<Object> exports ) {
	PeerConnection::Init( exports );
}

NODE_MODULE( peerconnection, InitAll );
