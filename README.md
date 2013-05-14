node-echoprint-codegen
======================

Echoprint Codegen Addon for Node.js


Requirements
------------

From the [echoprint-codegen README](https://github.com/echonest/echoprint-codegen/blob/master/README.md):

* [Boost](http://boost.org) >= 1.35
* [TagLib](http://taglib.github.io)
* [ffmpeg](http://ffmpeg.org) **Only required for filename mode**
* zlib

For Ubuntu/Debian:

```sh
$ sudo apt-get install libboost1.42-dev libtag1-dev zlib1g-dev [ffmpeg]
```

For OS X:

```sh
brew install boost taglib [ffmpeg]
```


Installation
------------

Since this package is not yet published to npm, install it via git:

```sh
$ npm install git+https://github.com/xdissent/node-echoprint-codegen.git
```


Usage
-----

There are two modes of operation: `filename` and `buffer`. Filename mode **requires** the `ffmpeg` command to be available on your path. It takes a filename and an optional start time and duration, and returns and object containing various metadata as well as the generated code:

```js
var ecg = require('echoprint-codegen');

// ecg(filename, start, duration, callback);
//
// filename - The path to a file of any type which ffmpeg can read
// start    - Sample after this number of seconds of audio (optional)
// duration - The length of the sample to use (optional - required if start given)
// callback - Callback function which will receive the generated code

// Example - Sample entire mp3 file:
ecg('/tmp.mp3', function(err, data) {
  if (err) throw err;
  console.log(data.code);
});

// Example - Sample 30 seconds of audio beginning at 10 seconds:
ecg('/tmp.mp3', 10, 30, function(err, data) {
  if (err) throw err;
  console.log(data.code);
});
```

The second mode of operation is `buffer` mode, in which PCM audio data is passed directly into the code generator via buffer. The generated code is passed to the callback as a string - there is no extra metadata returned:

```js
var ecg = require('echoprint-codegen');

// ecg(buffer, start, callback);
//
// buffer   - Buffer of mono 32 bit le float PCM data with a sample rate of 11025
// start    - Hint indicating at which point the samples occur in the song (optional)
// callback - Callback function which will receive the generated code

// Example - Pass a buffer presumably taken from the beginning of a song:
ecg(buffer, function(err, code) {
  if (err) throw err;
  console.log(code);
});

// Example - Pass a buffer taken from 10 seconds into the song:
ecg(buffer, 10, function(err, code) {
  if (err) throw err;
  console.log(code);
});
```


Examples
--------

Reading from a PCM file directly:

```js
var ecg = require('echoprint-codegen'),
  fs = require('fs');

var samples = 11025 * 30,     // Number of samples to read from file (30 seconds)
  start = 0,                  // Sample is from the beginning of the song
  bufferSize = samples * 4,   // Samples are 32 bit floats
  buffer = new Buffer(bufferSize);

fs.open('/test.pcm', 'r', function (err, fd) {
  if (err) throw "Error opening file";

  // Read samples from file
  fs.read(fd, buffer, 0, bufferSize, 0, function (err, bytesRead) {
    if (err) throw "Error reading file";
    if (bytesRead < bufferSize) throw "Couldn't read enough";

    // Generate echoprint code
    ecg(buffer, start, function(err, code) {
      if (err) throw err;
      console.log(code);
    });
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
  start = 0,                      // Start time of clip in seconds
  bufferSize = numSamples * 4;    // Samples are 32 bit floats

var ffmpeg = child_process.spawn('ffmpeg', [
  '-i',  '/test.mp3',   // MP3 file
  '-f',  'f32le',       // 32 bit float PCM LE
  '-ar', '11025',       // Sampling rate
  '-ac',  1,            // Mono
  '-t',   duration,     // Duration in seconds
  '-ss',  start,        // Start time in seconds
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
  ecg(buffer, start, function(err, code) {
    if (err) throw err;
    console.log(code);
  });
});
```