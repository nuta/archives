#include <efijs/message.h>
#include <libplatform/libplatform.h>
#include <v8.h>


using namespace v8;

extern "C" void main() {

    printf("EFI.js version " __EFIJS_VERSION__);

    // Initialize V8.
    printf("==> V8::InitializeICUDefaultLocation");
    V8::InitializeICUDefaultLocation("efijs");
    printf("==> V8::InitializeExternalStartupData");
    V8::InitializeExternalStartupData("efijs");
    printf("==> V8::CreateDefaultPlatform");
    Platform* platform = platform::CreateDefaultPlatform();
    printf("==> V8::initializePlatform");
    V8::InitializePlatform(platform);
    printf("==> V8::initialize");
    V8::Initialize();
    // Create a new Isolate and make it the current one.
    Isolate::CreateParams create_params;
    create_params.array_buffer_allocator =
        v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    printf("==> V8::Isolate::New");
    Isolate* isolate = Isolate::New(create_params);
    {
        Isolate::Scope isolate_scope(isolate);
        // Create a stack-allocated handle scope.
        HandleScope handle_scope(isolate);
        // Create a new context.
        Local<Context> context = Context::New(isolate);
        // Enter the context for compiling and running the hello world script.
        Context::Scope context_scope(context);
        // Create a string containing the JavaScript source code.
        Local<String> source =
            String::NewFromUtf8(isolate, "'Hello' + ', World!'",
                                NewStringType::kNormal).ToLocalChecked();
        // Compile the source code.
        printf("==> V8::Script::Compile");
        Local<Script> script = Script::Compile(context, source).ToLocalChecked();
        // Run the script to get the result.
        printf("==> V8::script->Run");
        Local<Value> result = script->Run(context).ToLocalChecked();
        // Convert the result to an UTF8 string and print it.
        String::Utf8Value utf8(result);
        printf("%s\n", *utf8);
    }
    // Dispose the isolate and tear down V8.
    isolate->Dispose();
    V8::Dispose();
    V8::ShutdownPlatform();
    delete platform;
    delete create_params.array_buffer_allocator;
}
