#include <include\cef_client.h>
#include <include\cef_app.h>

class MyV8Handler : public CefV8Handler {
public:

	bool Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval, CefString& exception)
	{
		if (name == "myfunc")
		{
			if (arguments.size() == 2)
			{
				std::string RemoteIP = arguments[0]->GetStringValue().ToString();
				unsigned int RemotePort = arguments[1]->GetUIntValue();
				printf("[SUBPROCESS] MYFUNC %s %u\n", RemoteIP.c_str(), RemotePort);

				CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("My_Message");
				CefRefPtr<CefListValue> args = msg->GetArgumentList();
				args->SetString(0, arguments[0]->GetStringValue());
				args->SetInt(1, arguments[1]->GetIntValue());

				CefRefPtr<CefV8Context> context = CefV8Context::GetCurrentContext();
				context->GetBrowser()->GetMainFrame()->SendProcessMessage(PID_BROWSER, msg);
				return true;
			}
		}

		// Function does not exist.
		return false;
	}

	// Provide the reference counting implementation for this class.
	IMPLEMENT_REFCOUNTING(MyV8Handler);
};

class MyApp : public CefApp, public CefRenderProcessHandler
{
public:

	bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefProcessId source_process, CefRefPtr<CefProcessMessage> message)
	{
		return false;
	}

	CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler()
	{
		return this;
	}

	void OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context)
	{
	}

	void OnWebKitInitialized()
	{
		std::string extensionCode =
			"var test;"
			"if (!test)"
			"test = {};"
			"(function() {"
			"test.myfunc = function(RemoteAddr, RemotePort) {"
			"native function myfunc(RemoteAddr, RemotePort);"
			"return myfunc(RemoteAddr, RemotePort);"
			"};"
			"})();";

		CefRefPtr<CefV8Handler> handler = new MyV8Handler();
		CefRegisterExtension("v8/test", extensionCode, handler);
	}

	IMPLEMENT_REFCOUNTING(MyApp);
};


// Program entry-point function.
int main(int argc, char* argv[]) {
    // Initialize the macOS sandbox for this helper process.
    //CefScopedSandboxContext sandbox_context;
    //if (!sandbox_context.Initialize(argc, argv))
    //    return 1;

    //// Load the CEF framework library at runtime instead of linking directly
    //// as required by the macOS sandbox implementation.
    //CefScopedLibraryLoader library_loader;
  /*  if (!library_loader.LoadInHelper())
        return 1;*/

    // Structure for passing command-line arguments.
    // The definition of this structure is platform-specific.
	CefMainArgs main_args;

    // Implementation of the CefApp interface.
    CefRefPtr<MyApp> app(new MyApp);

    // Execute the sub-process logic. This will block until the sub-process should exit.
    return CefExecuteProcess(main_args, app.get(), NULL);
}