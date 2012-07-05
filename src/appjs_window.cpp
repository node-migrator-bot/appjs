#include <node.h>
#include "appjs.h"
#include "appjs_window.h"
#include "includes/cef_handler.h"
#include "includes/util.h"

extern CefRefPtr<ClientHandler> g_handler;

namespace appjs {

using namespace v8;

Window::Window() {};
Window::~Window() {};

Persistent<Function> Window::constructor;

void Window::Init () {
  CREATE_NODE_CONSTRUCTOR("Window");

  DEFINE_PROTOTYPE_METHOD("show",Show);
  DEFINE_PROTOTYPE_METHOD("hide",Hide);
  DEFINE_PROTOTYPE_METHOD("destroy",Destroy);
  DEFINE_PROTOTYPE_METHOD("runInBrowser",RunInBrowser);

  END_CONSTRUCTOR();
}

Handle<Value> Window::New(const Arguments& args) {
  HandleScope scope;

  Handle<Object> self = Persistent<Object>::New(args.This());

  char* url = (args[0]->IsString()) ? V8StringToChar(args[0]->ToString()) : (char*) "/";
  Persistent<Object> windowSettings = Persistent<Object>::New((args[1]->IsObject()) ? args[1]->ToObject() : Object::New());

  Settings* settings = new Settings(windowSettings);
  MainWindow* obj = new MainWindow(url,settings);
  obj->setV8Handle(self);

  self->SetPointerInInternalField (0, obj);

  return scope.Close(args.This());
}

Handle<Value> Window::NewInstance(const Arguments& args) {
  HandleScope scope;

  const unsigned argc = 2;
  Handle<Value> argv[argc] = { args[0],args[1] };
  Local<Object> instance = constructor->NewInstance(argc, argv);

  Local<Object> global = Context::GetCurrent()->Global();
  Local<Object> process = global->Get(String::NewSymbol("process"))->ToObject();
  Local<Function> emitterConstructor = Local<Function>::Cast(process->Get(String::NewSymbol("EventEmitter")));
  Local<Value> emitterProto = emitterConstructor->Get(String::New("prototype"));
  Local<Object> windowProto = Local<Object>::Cast(instance->GetPrototype());
  windowProto->SetPrototype(emitterProto);

  return scope.Close(instance);
}

Handle<Value> Window::Show(const Arguments& args) {
  HandleScope scope;

  MainWindow *obj = ObjectWrap::Unwrap<MainWindow> (args.This());
  obj->show();

  return scope.Close(args.This());
}

Handle<Value> Window::Hide(const Arguments& args) {
  HandleScope scope;
  
  MainWindow *obj = ObjectWrap::Unwrap<MainWindow> (args.This());

  obj->hide();

  return scope.Close(args.This());
}

Handle<Value> Window::Destroy(const Arguments& args) {
  HandleScope scope;
  
  MainWindow *obj = ObjectWrap::Unwrap<MainWindow> (args.This());

  obj->destroy();

  return scope.Close(args.This());
}

Handle<Value> Window::RunInBrowser(const Arguments& args) {
  HandleScope scope;

  if(!args[0]->IsFunction())
    THROW_BAD_ARGS;

  CefRefPtr<CefBrowser> browser = g_handler->GetBrowser();

  if(browser) {
    char* script = V8StringToFunctionChar(args[0]->ToString());
    g_handler->GetBrowser()->GetMainFrame()->ExecuteJavaScript(script,"",0);
  }

  return scope.Close(args.This());
}

} /* appjs */

