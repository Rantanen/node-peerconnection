
var fs = require('fs');
var express = require('express');
var app = express();
var http = require('http');
var nodertc = require('../index');
var server = http.createServer(app)
var io = require('socket.io').listen( server );

app.get( '/', function( req, res ) {
    res.sendfile( __dirname + '/index.html' );
});

var connections = [];
io.sockets.on('connection', function (socket) {
    console.log( 'New connection' );
    socket.emit('start');

	var fs = require('fs');
	var file = fs.openSync( socket.id + ".pcm", "w" );

    socket.on('call', function( sdp ) {
		console.log( "New call:" );
        console.log( sdp.sdp );
		var c = new nodertc.PeerConnection();

		c.on( 'icecandidate', function( evt ) {
			socket.emit( 'icecandidate', evt );
		});

		c.on( 'answer', function( evt ) {
			socket.emit( 'answer', evt );
		});

		c.on( 'audio', function( evt ) {
            console.log( evt.data );
            c.transmit( evt.data );
		});

		c.setRemoteDescription( sdp );
		connections.push( c );

		socket.on( 'icecandidate', function( evt ) {
			console.log( "Adding ice candidate!" );
			console.dir( evt );
			c.addIceCandidate( evt );
		});
    });
});

server.listen( 1234, function( err ) {
    console.log( 'Test server ready at port 1234.' );
});
