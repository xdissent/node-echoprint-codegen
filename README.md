echoprint-codegen
=================

Echoprint Codegen Addon for Node.js


Requirements
------------

You must have the [echoprint-codegen library](https://github.com/echonest/echoprint-codegen) installed:

```sh
$ git clone https://github.com/echonest/echoprint-codegen.git
$ cd echoprint-codegen/src
$ make
$ sudo make install
$ sudo ldconfig # To tell ld about libcodegen
```

See the [echoprint-codegen README](https://github.com/echonest/echoprint-codegen/blob/master/README.md) for further requirements.


Installation
------------

Since this package is not yet published to npm, install it via git:

```sh
$ npm install git+https://github.com/xdissent/node-echoprint-codegen.git
```


Usage
-----

```js
var ecg = require('echoprint-codegen');

// buffer -     A Buffer of mono float 32 little-endian PCM data with a 11025 sample rate
// numSamples - The number of samples contained in buffer (ie. buffer.length / 4)
// songOffset - Hint about at which time in the song the samples in buffer occur
// callback -   Callback function which will receive the generated code
ecg(buffer, numSamples, songOffset, callback);
```


Examples
--------

Reading from a PCM file directly:

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

  // Read samples from file
  fs.read(fd, buffer, 0, bufferSize, 0, function (err, bytesRead) {
    if (err) throw "Error reading file";
    if (bytesRead < bufferSize) throw "Couldn't read enough";

    // Generate echoprint code
    ecg(buffer, numSamples, songOffset, console.log);
  });
});
```

Reading an MP3 using ffmpeg:

```js
var ecg = require('echoprint-codegen'),
  fs = require('fs'),
  child_process = require('child_process');

var duration = 30,                // Length of clip to extract
  numSamples = 11025 * duration,  // Number of samples to read
  songOffset = 0,                 // Start time of clip in seconds
  
  bytesPerSample = 4,     // Samples are 32 bit floats
  bufferSize = numSamples * bytesPerSample;

var ffmpeg = child_process.spawn('ffmpeg', [
  '-i', './test.mp3',   // MP3 file
  '-f', 'f32le',        // 32 bit float PCM LE
  '-ar', '11025',       // Sampling rate
  '-ac', 1,             // Mono
  '-t', duration,       // Duration in seconds
  '-ss', songOffset,    // Start time in seconds
  'pipe:1'              // Output on stdout
]);

ffmpeg.stdout.on('readable', function() {
  // Read samples from ffmpeg stdout
  var buffer = ffmpeg.stdout.read(bufferSize);
  if (buffer == null) return; // Not enough samples yet

  // Remove listener and kill ffmpeg
  ffmpeg.stdout.removeListener('readable', arguments.callee);
  ffmpeg.kill('SIGHUP');

  // Generate echoprint code
  ecg(buffer, numSamples, songOffset, console.log);
});
```