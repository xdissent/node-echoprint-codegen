#include <node.h>
#include <node_buffer.h>
#include <echoprint/Codegen.h>
#include <iostream>

using namespace v8;

Handle<Value> GetCodeString(const Arguments& args) {
  HandleScope scope;

  if (args.Length() < 3) {
    ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
    return scope.Close(Undefined());
  }

  Local<Value> pcm_buffer = args[0]->ToObject();

  if (!node::Buffer::HasInstance(pcm_buffer)) {
    ThrowException(Exception::TypeError(String::New("First argument must be a buffer.")));
    return scope.Close(Undefined());
  }

  if (!args[1]->IsNumber() || !args[2]->IsNumber()) {
    ThrowException(Exception::TypeError(String::New("Wrong arguments")));
    return scope.Close(Undefined());
  }
 
  float *pcm = (float *)node::Buffer::Data(pcm_buffer);
  Codegen *cg = new Codegen(pcm, args[1]->NumberValue(), args[2]->NumberValue());
  std::string code = cg->getCodeString();

  const unsigned argc = 1;
  Local<Value> argv[argc] = { Local<Value>::New(String::New(code.c_str(), code.length())) };
  Local<Function> cb = Local<Function>::Cast(args[3]);
  cb->Call(Context::GetCurrent()->Global(), argc, argv);

  return scope.Close(Undefined());
}

void Init(Handle<Object> exports, Handle<Object> module) {
  module->Set(String::NewSymbol("exports"),
      FunctionTemplate::New(GetCodeString)->GetFunction());
}

NODE_MODULE(ecg, Init)