
"use strict";

var nodertc = require('bindings')({
    bindings: 'peerconnection.node',
    try: [['module_root', 'compiled', 'platform', 'arch', 'bindings' ]]
});
var events = require('events');

function inherits( target, source ) {
	for( var k in source.prototype ) {
		target.prototype[k] = source.prototype[k];
    }
	return target;
}

exports.PeerConnection = inherits( nodertc.PeerConnection, events.EventEmitter );


