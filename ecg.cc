#include <node.h>
#include <node_buffer.h>
#include <Codegen.h>
#include <Params.h>

#define ECG_MIN_SAMPLES 200

using namespace v8;
using namespace node;

struct Baton {
  uv_work_t request;
  Persistent<Function> callback;
  Persistent<Object> buffer;
  float* pcm;
  unsigned int samples;
  int offset;
  bool overflow;
  bool underflow;
  std::string code;

  Baton(Handle<Function> cb_, Handle<Object> buffer_, int offset_, bool overflow_, bool underflow_) : 
      offset(offset_), overflow(overflow_), underflow(underflow_) {

    callback = Persistent<Function>::New(cb_);
    buffer = Persistent<Object>::New(buffer_);
    pcm = static_cast<float*>(static_cast<void*>(Buffer::Data(buffer)));
    samples = Buffer::Length(buffer) / 4; // 32 bit alignment
    request.data = this;
  }
  virtual ~Baton() {
    callback.Dispose();
    buffer.Dispose();
  }
};

void BeginGenerate(Baton* baton);
void DoGenerate(uv_work_t* req);
void AfterGenerate(uv_work_t* req);

Handle<Value> GetCodeString(const Arguments& args) {
  HandleScope scope;

  // 2 required arguments (buffer & callback)
  if (args.Length() < 2) {
    ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
    return scope.Close(Undefined());
  }

  // Ensure first arg is a buffer
  if (!Buffer::HasInstance(args[0])) {
    ThrowException(Exception::TypeError(String::New("First argument must be a buffer or string")));
    return scope.Close(Undefined());
  }

  // Optional offset
  int offset = 0;
  if (!args[1].IsEmpty() && args[1]->IsNumber()) offset = args[1]->Int32Value();

  // Validate offset
  if (offset < 0) {
    ThrowException(Exception::TypeError(String::New("Invalid offset")));
    return scope.Close(Undefined());
  }

  // Find callback in args
  Local<Function> callback;
  if (args[1]->IsFunction()) {          // No offset given
    callback = Local<Function>::Cast(args[1]);
  } else if (args[2]->IsFunction()) {   // Offset given, callback is 3rd arg
    callback = Local<Function>::Cast(args[2]);
  } else {                              // Non-function given
    ThrowException(Exception::TypeError(String::New("Must pass a callback function")));
    return scope.Close(Undefined());
  }

  // Precalculate number of samples and check for overflow
  // This is required here because Codegen would otherwise throw an exception
  // for overflows or just hang for underflows.
  uint samples = Buffer::Length(args[0]) / 4;
  bool overflow = Params::AudioStreamInput::MaxSamples < samples;
  bool underflow = ECG_MIN_SAMPLES > samples;

  // Kick off background op
  Baton* baton = new Baton(callback, args[0]->ToObject(), offset, overflow, underflow); 
  BeginGenerate(baton);
 
  return scope.Close(Undefined());
}

void BeginGenerate(Baton* baton) {
  uv_queue_work(uv_default_loop(), &baton->request, DoGenerate, (uv_after_work_cb)AfterGenerate);
}

void DoGenerate(uv_work_t* req) {
  // Generate code
  Baton* baton = static_cast<Baton*>(req->data);
  if (!baton->overflow && !baton->underflow) {
    Codegen *cg = new Codegen(baton->pcm, baton->samples, baton->offset);
    baton->code = cg->getCodeString();
  }
}

void AfterGenerate(uv_work_t* req) {
  HandleScope scope;
  Baton* baton = static_cast<Baton*>(req->data);

  Local<Value> argv[2] = { Local<Value>::New(Null()), Local<Value>::New(Null()) };
  // Check for overflow
  if (baton->overflow) {
    argv[0] = Local<Value>::New(String::New("Buffer too long"));

  // Check for underflow
  } else if (baton->underflow) {
    argv[0] = Local<Value>::New(String::New("Buffer too short"));

  // Check for non-empty code
  } else if (baton->code.length() == 0) {
    argv[0] = Local<Value>::New(String::New("No code generated"));

  // Success
  } else {
    Local<String> code = String::New(baton->code.c_str(), baton->code.length());
    argv[1] = Local<Value>::New(code);
  }

  baton->callback->Call(Context::GetCurrent()->Global(), 2, argv);
  delete baton;
}

void Initialize(Handle<Object> exports, Handle<Object> module) {
  module->Set(String::NewSymbol("exports"),
      FunctionTemplate::New(GetCodeString)->GetFunction());
}

NODE_MODULE(ecg, Initialize)
