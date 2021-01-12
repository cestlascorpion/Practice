const http = require('http');
const os = require('os');

console.log("kubia server starting");

var handler = function(request, response) {
    console.log("recv request from " + request.connection.remoteAddress);
    response.writeHead(200);
    response.end("you have hit " + os.hostname() + "\n");
};

var wwww = http.createServer(handler);
wwww.listen(8080);
