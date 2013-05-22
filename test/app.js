
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
			console.log('Incoming audio');
            c.transmit( evt.data );
		});

		c.on( 'signalingstatechange', function( state ) { console.log( 'signaling: ' +  state ); });
		c.on( 'iceconnectionstatechange', function( state ) { console.log( 'connection: ' + state ); });
		c.on( 'icegatheringstatechange', function( state ) { console.log( 'gathering: ' + state ); });
		c.on( 'icestatechange', function( state ) { console.log( 'icestate: ' + state ); });
		c.on( 'statechange', function( state ) { console.log( 'state: ' + state ); });

		c.setRemoteDescription( sdp );
		connections.push( c );

		socket.on( 'icecandidate', function( evt ) {
			console.log( "Adding ice candidate!" );
			console.dir( evt );
			c.addIceCandidate( evt );
		});

		socket.on('disconnect', function () {
			c.close();
		});
		socket.on('close', function () {
			c.close();
		});
    });
});

server.listen( 1234, function( err ) {
    console.log( 'Test server ready at port 1234.' );
});
