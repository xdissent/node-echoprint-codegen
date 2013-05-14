binding = require './build/Release/ecg'
path = require 'path'
child_process = require 'child_process'

module.exports = ->
  return binding arguments... unless typeof arguments[0] == 'string'
  throw "Must specify duration if start given" if arguments.length == 3
  if arguments.length == 4
    start = arguments[1]
    duration = arguments[2]
    callback = arguments[3]
  else if arguments.length == 2
    start = ''; duration = ''
    callback = arguments[1]
  else throw "Must specify a callback"

  cmd = "echoprint-codegen \"#{arguments[0]}\" #{start} #{duration}"
  cmdPath = path.join __dirname, 'build/Release', cmd
  child_process.exec cmdPath, (err, stdout) ->
    return callback(err) if err?
    try callback null, JSON.parse(stdout)[0] catch err then callback err