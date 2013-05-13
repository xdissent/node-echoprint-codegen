#include <node.h>
#include <node_buffer.h>
#include <Codegen.h>

using namespace v8;
using namespace node;

struct Baton {
  uv_work_t request;
  Persistent<Function> callback;
  Persistent<Object> buffer;
  float* pcm;
  unsigned int samples;
  int offset;
  std::string code;

  Baton(Handle<Function> cb_, Handle<Object> chunk_, int offset_) : offset(offset_) {
    callback = Persistent<Function>::New(cb_);
    buffer = Persistent<Object>::New(chunk_);
    pcm = static_cast<float*>(static_cast<void*>(Buffer::Data(buffer)));
    samples = Buffer::Length(buffer) / 4;
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

  if (args.Length() < 2) {
    ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
    return scope.Close(Undefined());
  }

  if (!node::Buffer::HasInstance(args[0])) {
    ThrowException(Exception::TypeError(String::New("First argument must be a buffer.")));
    return scope.Close(Undefined());
  }

  int offset = 0;
  if (args[1]->IsNumber()) offset = args[1]->Int32Value();

  Local<Function> callback;
  if (args.Length() == 3) {
    callback = Local<Function>::Cast(args[2]);
  } else {
    callback = Local<Function>::Cast(args[1]);
  }

  Baton* baton = new Baton(callback, args[0]->ToObject(), offset); 
  BeginGenerate(baton);
 
  return scope.Close(Undefined());
}

void BeginGenerate(Baton* baton) {
  uv_queue_work(uv_default_loop(), &baton->request, DoGenerate, (uv_after_work_cb)AfterGenerate);
}

void DoGenerate(uv_work_t* req) {
  Baton* baton = static_cast<Baton*>(req->data);
  Codegen *cg = new Codegen(baton->pcm, baton->samples, baton->offset);
  baton->code = cg->getCodeString();
}

void AfterGenerate(uv_work_t* req) {
  HandleScope scope;
  Baton* baton = static_cast<Baton*>(req->data);
  Local<Value> argv[1] = { Local<Value>::New(String::New(baton->code.c_str(), baton->code.length())) };
  baton->callback->Call(Context::GetCurrent()->Global(), 1, argv);
  delete baton;
}

void Initialize(Handle<Object> exports, Handle<Object> module) {
  module->Set(String::NewSymbol("exports"),
      FunctionTemplate::New(GetCodeString)->GetFunction());
}

NODE_MODULE(ecg, Initialize)
