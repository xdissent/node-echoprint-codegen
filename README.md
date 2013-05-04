echoprint-codegen
=================

Echoprint Codegen Addon for Node.js

Installation
------------

You must have the [echoprint-codegen library](https://github.com/echonest/echoprint-codegen) installed (`make install` within the `src` folder).

Example
-------

```js
var ecg = require('echoprint-codegen'),
  fs = require('fs');

var numSamples = 330750,  // Number of samples to read from file (30 seconds)
  songOffset = 0,         // Sample is from the beginning of the song
  
  bytesPerSample = 4,     // Samples are 32 bit floats
  bufferSize = numSamples * bytesPerSample,
  buffer = new Buffer(bufferSize);

fs.open('./test.pcm', 'r', function (err, fd) {
  if (err) throw "Error opening file";

  fs.read(fd, buffer, 0, bufferSize, 0, function (err, bytesRead) {
    if (err) throw "Error reading file";
    if (bytesRead < bufferSize) throw "Couldn't read enough";

    ecg(buffer, numSamples, songOffset, function(code) {
      console.log(code);
    });
  });
});
```