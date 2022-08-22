#include <include\cef_app.h>
#include <include\cef_client.h>
#include <include\cef_render_handler.h>


class RenderHandler : public CefRenderHandler
{
public:

	RenderHandler()
	{
	}

	bool frameStarted()
	{
		CefDoMessageLoopWork();
		return true;
	}

	//
	//	Set here the size of our browser window?
	void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect)
	{
		rect = CefRect(0, 0, 1024, 768);
	}

	//
	//	Here we copy the raw texture data over to vulkan
	//	A8R8G8B8
	void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirtyRects, const void* buffer, int width, int height)
	{
		//memcpy(Texture.data, buffer, width * height * 4);
	}

	IMPLEMENT_REFCOUNTING(RenderHandler);
};

class BrowserClient : public CefClient
{
public:
	BrowserClient(RenderHandler* renderHandler)
		: m_renderHandler(renderHandler)
	{}

	virtual CefRefPtr<CefRenderHandler> GetRenderHandler()
	{
		return m_renderHandler;
	}

	CefRefPtr<CefRenderHandler> m_renderHandler;

	IMPLEMENT_REFCOUNTING(BrowserClient);
};

namespace WorldEngine
{
	namespace CEF
	{
		namespace
		{

		}

		void Initialize()
		{
			CefMainArgs args;

			int result = CefExecuteProcess(args, nullptr, nullptr);
			if (result >= 0)
			{
				printf("CEF Process Ended; Exit (%i)\n", result);
			}

			CefSettings settings;
			bool result2 = CefInitialize(args, settings, nullptr, nullptr);
			if (!result2)
			{
				printf("CEF Initialize Error\n");
			}

			RenderHandler* renderHandler = new RenderHandler();

			CefRefPtr<CefBrowser> browser;
			CefRefPtr<BrowserClient> browserClient;

			CefWindowInfo window_info;
			CefBrowserSettings browserSettings;
			browserSettings.windowless_frame_rate = 60;
			size_t windowHandle = 0;
			window_info.SetAsWindowless(nullptr);

			browserClient = new BrowserClient(renderHandler);

			browser = CefBrowserHost::CreateBrowserSync(window_info, browserClient.get(), "http://google.com/", browserSettings, nullptr, nullptr);
			//browser->GetHost()->SendKeyEvent(..);
			//browser->GetHost()->SendMouseMoveEvent(..);
		}

		void Deinitialize()
		{
			//browser = nullptr;
			//browserClient = nullptr;
			CefShutdown();
		}

	}
}