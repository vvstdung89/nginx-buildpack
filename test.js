
var http = require('http');
var qs = require('querystring');
var sys = require('sys');
var exec = require('child_process').exec;

var options = {
    host: 'localhost',
    path: '/transcode/pseudo-hls?url=http%3A%2F%2Fwww.w3schools.com%2Fhtml%2Fmov_bbb.mp4'
};



function processPost(request, response, callback) {
    var body = '';
        request.on('data', function (data) {
            console.log("data ... " + data);
            body += data;
            // Too much POST data, kill the connection!
            if (body.length > 1e6)
                request.connection.destroy();
        });

        request.on('end', function () {
            console.log("end ... ");
            var p = qs.parse(body);
            var host = p['host'];
            var port = p['port'];
            var option = p['option'];

            var options = {
                host: host,
                port: port,
                path: option
            };

            callback_js = function(response) {

              response.on('data', function (chunk) {
                
              });

              response.on('end', function () {
                callback(response.headers['location']);
              });
            }
            var req = http.request(options, callback_js).end();
            

           // var cmd = 'curl -v ' + link;
        //     child = exec(cmd, function (error, stdout, stderr) {
        //       console.log(stdout + " " + stderr);
        //       if (error !== null) {
        //         console.log('exec error: ' + error);
        //       }
        //     });

        });
}

http.createServer(function(request, response) {
    console.log("start process packet")
    if(request.method == 'POST') {
        processPost(request, response, function(check) {
            // Use request.post here
            response.writeHead(200, "OK", {'Content-Type': 'text/plain','Access-Control-Allow-Origin':'*'} );
            response.end(check);
        });
    }

}).listen(9999);
