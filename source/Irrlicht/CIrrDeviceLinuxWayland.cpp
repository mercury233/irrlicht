#include "CIrrDeviceLinuxWayland.h"

#ifdef _IRR_COMPILE_WITH_WAYLAND_DEVICE_

#include "CEGLManagerWayland.h"
#include "COpenGLDriver.h"
#include "COSOperator.h"
#include "IGUIEnvironment.h"
#include "IGUIElement.h"
#include "ISceneManager.h"
#include "irrString.h"
#include "os.h"
#include <cerrno>
#include <sched.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/mman.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <time.h>
#include <xkbcommon/xkbcommon-keysyms.h>

namespace irr
{
static const uint32_t IRR_WAYLAND_BTN_LEFT = 0x110;
static const uint32_t IRR_WAYLAND_BTN_RIGHT = 0x111;
static const uint32_t IRR_WAYLAND_BTN_MIDDLE = 0x112;

namespace video
{
	IVideoDriver* createOpenGLDriverWayland(const SIrrlichtCreationParameters&, io::IFileSystem*, IContextManager*, f32);
}

	CIrrDeviceLinuxWayland::CIrrDeviceLinuxWayland(const SIrrlichtCreationParameters& param) :
		CIrrDeviceStub(param), Display(0), Registry(0), Compositor(0), Surface(0), CursorSurface(0), Shm(0),
		Seat(0), Pointer(0), Keyboard(0), DataDeviceManager(0), DataDevice(0), DataSource(0),
		PendingDataOffer(0), SelectionDataOffer(0), FractionalManager(0), FractionalScale(0), Viewporter(0), Viewport(0),
		TextInputManager(0), TextInput(0), Decor(0), Frame(0), EglWindow(0), CursorTheme(0),
		XkbContext(0), XkbKeymap(0), XkbState(0), LogicalSize(param.WindowSize), FramebufferSize(param.WindowSize),
		ScaleFactor(1.f), PointerSerial(0), KeyboardSerial(0), PointerButtons(0), RepeatRate(0), RepeatDelay(0),
		RepeatKey(0), RepeatAt(0), Configured(false), WindowHasFocus(false), WindowMinimized(false),
		TextInputEntered(false), TextInputEnabled(false), Resizable(param.WindowResizable == 1), ScaleReported(false), ShiftDown(false), ControlDown(false),
		PendingOfferHasText(false), SelectionOfferHasText(false)
	{
#ifdef _DEBUG
		setDebugName("CIrrDeviceLinuxWayland");
#endif
		struct utsname info;
		core::stringc version("Linux Wayland");
		if (uname(&info) == 0)
		{
			version = info.sysname;
			version += " ";
			version += info.release;
			version += " Wayland";
		}
		Operator = new COSOperator(version, this);

		if (CreationParams.WindowId || (CreationParams.DriverType != video::EDT_OPENGL && CreationParams.DriverType != video::EDT_NULL))
		{
			std::fprintf(stderr, "Irrlicht Wayland: only OpenGL/Null without WindowId is supported\n");
			return;
		}

		if (CreationParams.DriverType != video::EDT_NULL && !createWindow())
			return;

		CursorControl = new CCursorControl(this);
		createDriver();
		if (!VideoDriver)
			return;
		createGUIAndScene();
	}

	CIrrDeviceLinuxWayland::~CIrrDeviceLinuxWayland()
	{
		if (GUIEnvironment) { GUIEnvironment->drop(); GUIEnvironment = 0; }
		if (SceneManager) { SceneManager->drop(); SceneManager = 0; }
		if (VideoDriver) { VideoDriver->drop(); VideoDriver = 0; }
		if (ContextManager) { ContextManager->drop(); ContextManager = 0; }
		destroyWayland();
		setActiveWindowScaleFactor(0.f);
	}

	void* CIrrDeviceLinuxWayland::bindGlobal(uint32_t name, const wl_interface* interface, uint32_t version)
	{
		const uint32_t bindVersion = version < static_cast<uint32_t>(interface->version) ? version : static_cast<uint32_t>(interface->version);
		return Library.ProxyMarshalConstructorVersioned(reinterpret_cast<wl_proxy*>(Registry), 0, interface, bindVersion,
			name, interface->name, bindVersion, 0);
	}

	void CIrrDeviceLinuxWayland::registryGlobal(void* data, wl_registry*, uint32_t name, const char* interface, uint32_t version)
	{
		CIrrDeviceLinuxWayland* self = static_cast<CIrrDeviceLinuxWayland*>(data);
		if (!std::strcmp(interface, "wl_compositor"))
			self->Compositor = static_cast<wl_compositor*>(self->bindGlobal(name, self->Library.CompositorInterface, version > 4 ? 4 : version));
		else if (!std::strcmp(interface, "wl_shm"))
			self->Shm = static_cast<wl_shm*>(self->bindGlobal(name, self->Library.ShmInterface, 1));
		else if (!std::strcmp(interface, "wl_seat"))
			self->Seat = static_cast<wl_seat*>(self->bindGlobal(name, self->Library.SeatInterface, version > 5 ? 5 : version));
		else if (!std::strcmp(interface, "wl_data_device_manager"))
			self->DataDeviceManager = static_cast<wl_data_device_manager*>(self->bindGlobal(name, self->Library.DataDeviceManagerInterface, version > 3 ? 3 : version));
		else if (!std::strcmp(interface, "wp_fractional_scale_manager_v1"))
			self->FractionalManager = static_cast<wp_fractional_scale_manager_v1*>(self->bindGlobal(name, &wp_fractional_scale_manager_v1_interface, 1));
		else if (!std::strcmp(interface, "wp_viewporter"))
			self->Viewporter = static_cast<wp_viewporter*>(self->bindGlobal(name, &wp_viewporter_interface, 1));
		else if (!std::strcmp(interface, "zwp_text_input_manager_v3"))
			self->TextInputManager = static_cast<zwp_text_input_manager_v3*>(self->bindGlobal(name, &zwp_text_input_manager_v3_interface, 1));
	}

	void CIrrDeviceLinuxWayland::registryRemove(void*, wl_registry*, uint32_t) {}

	void CIrrDeviceLinuxWayland::decorError(libdecor*, libdecor_error, const char* message)
	{
		std::fprintf(stderr, "Irrlicht Wayland libdecor: %s\n", message ? message : "unknown error");
	}

	void CIrrDeviceLinuxWayland::frameConfigure(libdecor_frame*, libdecor_configuration* configuration, void* data)
	{
		static_cast<CIrrDeviceLinuxWayland*>(data)->applyConfigure(configuration);
	}

	void CIrrDeviceLinuxWayland::frameClose(libdecor_frame*, void* data)
	{
		static_cast<CIrrDeviceLinuxWayland*>(data)->Close = true;
	}

	void CIrrDeviceLinuxWayland::frameCommit(libdecor_frame*, void* data)
	{
		CIrrDeviceLinuxWayland* self = static_cast<CIrrDeviceLinuxWayland*>(data);
		if (self->Surface)
			self->Library.ProxyMarshal(reinterpret_cast<wl_proxy*>(self->Surface), 6);
	}

	void CIrrDeviceLinuxWayland::frameDismissPopup(libdecor_frame*, const char*, void*) {}

	void CIrrDeviceLinuxWayland::fractionalPreferred(void* data, wp_fractional_scale_v1*, uint32_t scale)
	{
		static_cast<CIrrDeviceLinuxWayland*>(data)->applyScale(scale);
	}

	bool CIrrDeviceLinuxWayland::createWindow()
	{
		if (!Library.load())
		{
			std::fprintf(stderr, "Irrlicht Wayland: runtime library load failed: %s\n", Library.getError() ? Library.getError() : "unknown error");
			return false;
		}
		initializeWaylandProtocolInterfaces(Library);
		if (!Library.hasDecorationPlugin())
		{
			std::fprintf(stderr, "Irrlicht Wayland: no libdecor cairo/GTK decoration plugin was found\n");
			return false;
		}
		Display = Library.DisplayConnect(0);
		if (!Display)
		{
			std::fprintf(stderr, "Irrlicht Wayland: wl_display_connect failed\n");
			return false;
		}

		Registry = reinterpret_cast<wl_registry*>(Library.ProxyMarshalConstructor(reinterpret_cast<wl_proxy*>(Display), 1, Library.RegistryInterface, 0));
		static const wl_registry_listener registryListener = { registryGlobal, registryRemove };
		Library.ProxyAddListener(reinterpret_cast<wl_proxy*>(Registry), reinterpret_cast<void (**)(void)>(const_cast<wl_registry_listener*>(&registryListener)), this);
		if (Library.DisplayRoundtrip(Display) < 0 || !Compositor || !FractionalManager || !Viewporter)
		{
			std::fprintf(stderr, "Irrlicht Wayland: compositor lacks wp_fractional_scale_v1 or wp_viewporter\n");
			return false;
		}
		if (!TextInputManager)
			std::fprintf(stderr, "Irrlicht Wayland: zwp_text_input_manager_v3 is unavailable; IME input is disabled\n");

		Surface = reinterpret_cast<wl_surface*>(Library.ProxyMarshalConstructor(reinterpret_cast<wl_proxy*>(Compositor), 0, Library.SurfaceInterface, 0));
		CursorSurface = reinterpret_cast<wl_surface*>(Library.ProxyMarshalConstructor(reinterpret_cast<wl_proxy*>(Compositor), 0, Library.SurfaceInterface, 0));
		Viewport = waylandGetViewport(Library, Viewporter, Surface);
		FractionalScale = waylandGetFractionalScale(Library, FractionalManager, Surface);
		static const wp_fractional_scale_v1_listener scaleListener = { fractionalPreferred };
		Library.ProxyAddListener(reinterpret_cast<wl_proxy*>(FractionalScale), reinterpret_cast<void (**)(void)>(const_cast<wp_fractional_scale_v1_listener*>(&scaleListener)), this);
		Library.ProxyMarshal(reinterpret_cast<wl_proxy*>(Surface), 8, 1);
		if (Viewport)
			waylandViewportSetDestination(Library, Viewport, LogicalSize.Width, LogicalSize.Height);

		if (Seat)
		{
			static const wl_seat_listener seatListener = { seatCapabilities, seatName };
			Library.ProxyAddListener(reinterpret_cast<wl_proxy*>(Seat), reinterpret_cast<void (**)(void)>(const_cast<wl_seat_listener*>(&seatListener)), this);
			Library.DisplayRoundtrip(Display);
			initializeDataDevice();
		}

		static libdecor_interface decorInterface = {};
		decorInterface.error = decorError;
		Decor = Library.DecorNew(Display, &decorInterface);
		if (!Decor)
		{
			std::fprintf(stderr, "Irrlicht Wayland: libdecor or its decoration plugin is unavailable\n");
			return false;
		}
		static libdecor_frame_interface frameInterface = {};
		frameInterface.configure = frameConfigure;
		frameInterface.close = frameClose;
		frameInterface.commit = frameCommit;
		frameInterface.dismiss_popup = frameDismissPopup;
		Frame = Library.DecorDecorate(Decor, Surface, &frameInterface, this);
		if (!Frame)
		{
			std::fprintf(stderr, "Irrlicht Wayland: libdecor_decorate failed\n");
			return false;
		}
		Library.DecorFrameSetAppId(Frame, "org.moecube.koishipro");
		Library.DecorFrameSetTitle(Frame, "KoishiPro");
		if (CreationParams.WindowResizable < 2)
			setResizable(CreationParams.WindowResizable == 1);
		if (CreationParams.Fullscreen)
			Library.DecorFrameSetFullscreen(Frame, 0);
		Library.DecorFrameMap(Frame);

		for (u32 attempts = 0; !Configured && attempts < 100; ++attempts)
		{
			if (Library.DecorDispatch(Decor, 50) < 0)
				return false;
		}
		if (!Configured)
		{
			std::fprintf(stderr, "Irrlicht Wayland: timed out waiting for a decorated window configuration\n");
			return false;
		}
		applyScale(static_cast<uint32_t>(ScaleFactor * 120.f + 0.5f));
		EglWindow = Library.EglWindowCreate(Surface, FramebufferSize.Width, FramebufferSize.Height);
		if (!EglWindow)
		{
			std::fprintf(stderr, "Irrlicht Wayland: wl_egl_window_create failed\n");
			return false;
		}
		return true;
	}

	void CIrrDeviceLinuxWayland::applyConfigure(libdecor_configuration* configuration)
	{
		int width = static_cast<int>(LogicalSize.Width);
		int height = static_cast<int>(LogicalSize.Height);
		Library.DecorConfigurationGetContentSize(configuration, Frame, &width, &height);
		if (width > 0 && height > 0)
			LogicalSize.set(static_cast<u32>(width), static_cast<u32>(height));
		libdecor_window_state windowState = LIBDECOR_WINDOW_STATE_NONE;
		if (Library.DecorConfigurationGetWindowState(configuration, &windowState))
		{
			WindowHasFocus = (windowState & LIBDECOR_WINDOW_STATE_ACTIVE) != 0;
			WindowMinimized = (windowState & LIBDECOR_WINDOW_STATE_SUSPENDED) != 0;
		}
		libdecor_state* state = Library.DecorStateNew(LogicalSize.Width, LogicalSize.Height);
		Library.DecorFrameCommit(Frame, state, configuration);
		Library.DecorStateFree(state);
		Configured = true;
		applyScale(static_cast<uint32_t>(ScaleFactor * 120.f + 0.5f));
	}

	void CIrrDeviceLinuxWayland::applyScale(uint32_t scale120)
	{
		if (!scale120)
			scale120 = 120;
		const f32 previousScale = ScaleFactor;
		const core::dimension2d<u32> previousFramebufferSize = FramebufferSize;
		ScaleFactor = static_cast<f32>(scale120) / 120.f;
		FramebufferSize.Width = static_cast<u32>(std::ceil(LogicalSize.Width * ScaleFactor));
		FramebufferSize.Height = static_cast<u32>(std::ceil(LogicalSize.Height * ScaleFactor));
		if (Viewport)
			waylandViewportSetDestination(Library, Viewport, LogicalSize.Width, LogicalSize.Height);
		if (EglWindow)
			Library.EglWindowResize(EglWindow, FramebufferSize.Width, FramebufferSize.Height, 0, 0);
		setActiveWindowScaleFactor(ScaleFactor);
		if (VideoDriver && CreationParams.DriverType == video::EDT_OPENGL)
		{
			video::COpenGLDriver* driver = static_cast<video::COpenGLDriver*>(VideoDriver);
			driver->setWindowScaleFactor(ScaleFactor);
			driver->OnResize(LogicalSize);
		}
		updateCursor();
		if (!ScaleReported || previousScale != ScaleFactor || previousFramebufferSize != FramebufferSize)
		{
			ScaleReported = true;
			std::fprintf(stderr, "Irrlicht Wayland: logical %ux%u, framebuffer %ux%u, scale %.3f\n",
				LogicalSize.Width, LogicalSize.Height, FramebufferSize.Width, FramebufferSize.Height, ScaleFactor);
		}
	}

	void CIrrDeviceLinuxWayland::createDriver()
	{
		if (CreationParams.DriverType == video::EDT_NULL)
		{
			VideoDriver = video::createNullDriver(FileSystem, LogicalSize);
			return;
		}
		video::SExposedVideoData exposed;
		exposed.OpenGLWayland.WaylandDisplay = Display;
		exposed.OpenGLWayland.WaylandSurface = Surface;
		exposed.OpenGLWayland.EGLDisplay = 0;
		exposed.OpenGLWayland.EGLContext = 0;
		ContextManager = new video::CEGLManagerWayland(Library, Display, Surface, EglWindow);
		if (!ContextManager->initialize(CreationParams, exposed))
		{
			ContextManager->drop();
			ContextManager = 0;
			return;
		}
		VideoDriver = video::createOpenGLDriverWayland(CreationParams, FileSystem, ContextManager, ScaleFactor);
	}

	void CIrrDeviceLinuxWayland::destroyWayland()
	{
		if (XkbState) Library.XkbStateUnref(XkbState);
		if (XkbKeymap) Library.XkbKeymapUnref(XkbKeymap);
		if (XkbContext) Library.XkbContextUnref(XkbContext);
		XkbState = 0; XkbKeymap = 0; XkbContext = 0;
		if (CursorTheme) Library.CursorThemeDestroy(CursorTheme);
		if (DataSource)
		{
			Library.ProxyMarshal(reinterpret_cast<wl_proxy*>(DataSource), 1);
			Library.ProxyDestroy(reinterpret_cast<wl_proxy*>(DataSource));
		}
		if (PendingDataOffer == SelectionDataOffer)
			PendingDataOffer = 0;
		destroyDataOffer(PendingDataOffer);
		destroyDataOffer(SelectionDataOffer);
		if (Frame) Library.DecorFrameUnref(Frame);
		if (Decor) Library.DecorUnref(Decor);
		if (EglWindow) Library.EglWindowDestroy(EglWindow);
		wl_proxy* proxies[] = {
			reinterpret_cast<wl_proxy*>(TextInput), reinterpret_cast<wl_proxy*>(DataDevice),
			reinterpret_cast<wl_proxy*>(Keyboard), reinterpret_cast<wl_proxy*>(Pointer),
			reinterpret_cast<wl_proxy*>(FractionalScale), reinterpret_cast<wl_proxy*>(Viewport), reinterpret_cast<wl_proxy*>(CursorSurface),
			reinterpret_cast<wl_proxy*>(Surface), reinterpret_cast<wl_proxy*>(TextInputManager),
			reinterpret_cast<wl_proxy*>(DataDeviceManager), reinterpret_cast<wl_proxy*>(Seat),
			reinterpret_cast<wl_proxy*>(Shm), reinterpret_cast<wl_proxy*>(FractionalManager), reinterpret_cast<wl_proxy*>(Viewporter),
			reinterpret_cast<wl_proxy*>(Compositor), reinterpret_cast<wl_proxy*>(Registry)
		};
		for (u32 i = 0; i < sizeof(proxies) / sizeof(proxies[0]); ++i)
			if (proxies[i]) Library.ProxyDestroy(proxies[i]);
		if (Display) Library.DisplayDisconnect(Display);
		Display = 0;
	}

	bool CIrrDeviceLinuxWayland::run()
	{
		os::Timer::tick();
		if (Decor && Library.DecorDispatch(Decor, 0) < 0)
			Close = true;
		if (Display)
			Library.DisplayFlush(Display);
		updateTextInputFocus();
		if (RepeatKey && RepeatRate > 0 && os::Timer::getTime() >= RepeatAt)
		{
			postKey(RepeatKey, true, true);
			RepeatAt += core::max_<u32>(1, 1000 / RepeatRate);
		}
		return !Close;
	}

	void CIrrDeviceLinuxWayland::yield() { sched_yield(); }

	void CIrrDeviceLinuxWayland::sleep(u32 timeMs, bool pauseTimer)
	{
		bool stopped = Timer ? Timer->isStopped() : true;
		if (pauseTimer && !stopped) Timer->stop();
		struct timespec ts = { static_cast<time_t>(timeMs / 1000), static_cast<long>((timeMs % 1000) * 1000000) };
		nanosleep(&ts, 0);
		if (pauseTimer && !stopped) Timer->start();
	}

	void CIrrDeviceLinuxWayland::setWindowCaption(const wchar_t* text)
	{
		if (!Frame || !text) return;
		char* utf8 = core::toMultiByte(text);
		Library.DecorFrameSetTitle(Frame, utf8);
		delete [] utf8;
	}

	bool CIrrDeviceLinuxWayland::isWindowActive() const { return WindowHasFocus && !WindowMinimized; }
	bool CIrrDeviceLinuxWayland::isWindowFocused() const { return WindowHasFocus; }
	bool CIrrDeviceLinuxWayland::isWindowMinimized() const { return WindowMinimized; }
	video::ECOLOR_FORMAT CIrrDeviceLinuxWayland::getColorFormat() const { return video::ECF_R8G8B8; }
	void CIrrDeviceLinuxWayland::closeDevice() { Close = true; }
	video::IVideoModeList* CIrrDeviceLinuxWayland::getVideoModeList() { return VideoModeList; }

	void CIrrDeviceLinuxWayland::setResizable(bool resize)
	{
		Resizable = resize;
		if (!Frame) return;
		if (resize)
		{
			Library.DecorFrameSetMinContentSize(Frame, 0, 0);
			Library.DecorFrameSetMaxContentSize(Frame, 0, 0);
			Library.DecorFrameSetCapabilities(Frame, LIBDECOR_ACTION_RESIZE);
		}
		else
		{
			Library.DecorFrameSetMinContentSize(Frame, LogicalSize.Width, LogicalSize.Height);
			Library.DecorFrameSetMaxContentSize(Frame, LogicalSize.Width, LogicalSize.Height);
			Library.DecorFrameUnsetCapabilities(Frame, LIBDECOR_ACTION_RESIZE);
		}
	}

	void CIrrDeviceLinuxWayland::setWindowSize(const core::dimension2d<u32>& size)
	{
		LogicalSize = size;
		if (Frame)
		{
			libdecor_state* state = Library.DecorStateNew(size.Width, size.Height);
			Library.DecorFrameCommit(Frame, state, 0);
			Library.DecorStateFree(state);
			applyScale(static_cast<uint32_t>(ScaleFactor * 120.f + 0.5f));
		}
		else if (VideoDriver)
			VideoDriver->OnResize(size);
	}

	void CIrrDeviceLinuxWayland::minimizeWindow() { if (Frame) Library.DecorFrameSetMinimized(Frame); WindowMinimized = true; }
	void CIrrDeviceLinuxWayland::maximizeWindow() { if (Frame) Library.DecorFrameSetMaximized(Frame); }
	void CIrrDeviceLinuxWayland::restoreWindow() { if (Frame) Library.DecorFrameUnsetMaximized(Frame); WindowMinimized = false; }
	void CIrrDeviceLinuxWayland::clearSystemMessages() { if (Display) while (Library.DisplayDispatchPending(Display) > 0) {} }
	void CIrrDeviceLinuxWayland::copyToClipboard(const c8* text) const
	{
		CIrrDeviceLinuxWayland* self = const_cast<CIrrDeviceLinuxWayland*>(this);
		self->Clipboard = text ? text : "";
		if (!self->DataDevice || (!self->KeyboardSerial && !self->PointerSerial))
			return;
		if (self->DataSource)
		{
			self->Library.ProxyMarshal(reinterpret_cast<wl_proxy*>(self->DataSource), 1);
			self->Library.ProxyDestroy(reinterpret_cast<wl_proxy*>(self->DataSource));
			self->DataSource = 0;
		}
		self->DataSource = reinterpret_cast<wl_data_source*>(self->Library.ProxyMarshalConstructor(
			reinterpret_cast<wl_proxy*>(self->DataDeviceManager), 0, self->Library.DataSourceInterface, 0));
		if (!self->DataSource)
			return;
		static const wl_data_source_listener listener = { dataSourceTarget, dataSourceSend, dataSourceCancelled,
			dataSourceDropPerformed, dataSourceFinished, dataSourceAction };
		self->Library.ProxyAddListener(reinterpret_cast<wl_proxy*>(self->DataSource),
			reinterpret_cast<void (**)(void)>(const_cast<wl_data_source_listener*>(&listener)), self);
		self->Library.ProxyMarshal(reinterpret_cast<wl_proxy*>(self->DataSource), 0, "text/plain;charset=utf-8");
		self->Library.ProxyMarshal(reinterpret_cast<wl_proxy*>(self->DataSource), 0, "text/plain");
		const uint32_t serial = self->KeyboardSerial ? self->KeyboardSerial : self->PointerSerial;
		self->Library.ProxyMarshal(reinterpret_cast<wl_proxy*>(self->DataDevice), 1, self->DataSource, serial);
		self->Library.DisplayFlush(self->Display);
	}

	const c8* CIrrDeviceLinuxWayland::getTextFromClipboard() const
	{
		CIrrDeviceLinuxWayland* self = const_cast<CIrrDeviceLinuxWayland*>(this);
		if (self->DataSource)
			return self->Clipboard.c_str();
		if (!self->SelectionDataOffer || !self->SelectionOfferHasText)
			return self->Clipboard.c_str();
		int descriptors[2];
		if (pipe(descriptors) != 0)
			return self->Clipboard.c_str();
		fcntl(descriptors[0], F_SETFD, FD_CLOEXEC);
		fcntl(descriptors[1], F_SETFD, FD_CLOEXEC);
		fcntl(descriptors[0], F_SETFL, fcntl(descriptors[0], F_GETFL) | O_NONBLOCK);
		self->Library.ProxyMarshal(reinterpret_cast<wl_proxy*>(self->SelectionDataOffer), 1,
			self->SelectionOfferMime.c_str(), descriptors[1]);
		close(descriptors[1]);
		self->Library.DisplayFlush(self->Display);
		self->Clipboard = "";
		char buffer[4096];
		pollfd descriptor = { descriptors[0], POLLIN | POLLHUP, 0 };
		while (poll(&descriptor, 1, 1000) > 0)
		{
			const ssize_t count = read(descriptors[0], buffer, sizeof(buffer));
			if (count > 0)
				self->Clipboard.append(buffer, static_cast<u32>(count));
			else if (!count || errno != EAGAIN)
				break;
			if (descriptor.revents & POLLHUP)
			{
				ssize_t remaining = 0;
				while ((remaining = read(descriptors[0], buffer, sizeof(buffer))) > 0)
					self->Clipboard.append(buffer, static_cast<u32>(remaining));
				break;
			}
		}
		close(descriptors[0]);
		return self->Clipboard.c_str();
	}

	CIrrDeviceLinuxWayland::CCursorControl::CCursorControl(CIrrDeviceLinuxWayland* device) :
		Device(device), UseReferenceRect(false), Visible(true), ActiveIcon(gui::ECI_NORMAL) {}
	void CIrrDeviceLinuxWayland::CCursorControl::setVisible(bool visible) { Visible = visible; Device->updateCursor(); }
	void CIrrDeviceLinuxWayland::CCursorControl::setPosition(const core::position2d<f32>& pos) { setPosition(pos.X, pos.Y); }
	void CIrrDeviceLinuxWayland::CCursorControl::setPosition(f32 x, f32 y) { Position.set(static_cast<s32>(x * Device->LogicalSize.Width), static_cast<s32>(y * Device->LogicalSize.Height)); }
	core::position2d<f32> CIrrDeviceLinuxWayland::CCursorControl::getRelativePosition(bool)
	{
		const f32 width = UseReferenceRect ? static_cast<f32>(ReferenceRect.getWidth()) : static_cast<f32>(Device->LogicalSize.Width);
		const f32 height = UseReferenceRect ? static_cast<f32>(ReferenceRect.getHeight()) : static_cast<f32>(Device->LogicalSize.Height);
		return core::position2d<f32>(width ? Position.X / width : 0.f, height ? Position.Y / height : 0.f);
	}
	void CIrrDeviceLinuxWayland::CCursorControl::setReferenceRect(core::rect<s32>* rect) { UseReferenceRect = rect != 0; if (rect) ReferenceRect = *rect; }
	bool CIrrDeviceLinuxWayland::CCursorControl::getReferenceRect(core::rect<s32>& rect) { rect = UseReferenceRect ? ReferenceRect : core::rect<s32>(0, 0, Device->LogicalSize.Width, Device->LogicalSize.Height); return UseReferenceRect; }
	void CIrrDeviceLinuxWayland::CCursorControl::setActiveIcon(gui::ECURSOR_ICON iconId) { ActiveIcon = iconId; Device->updateCursor(); }

	void CIrrDeviceLinuxWayland::updateCursor()
	{
		if (!Pointer || !PointerSerial || !CursorSurface || !CursorControl)
			return;
		CCursorControl* cursorControl = static_cast<CCursorControl*>(CursorControl);
		if (!cursorControl->isVisible())
		{
			Library.ProxyMarshal(reinterpret_cast<wl_proxy*>(Pointer), 0, PointerSerial, 0, 0, 0);
			return;
		}
		if (!Shm) return;
		const int cursorScale = static_cast<int>(std::ceil(ScaleFactor));
		if (CursorTheme) Library.CursorThemeDestroy(CursorTheme);
		CursorTheme = Library.CursorThemeLoad(0, 24 * cursorScale, Shm);
		if (!CursorTheme) return;
		static const char* names[gui::ECI_COUNT] = {
			"left_ptr", "crosshair", "pointer", "help", "text", "not-allowed", "wait",
			"all-scroll", "nesw-resize", "nwse-resize", "ns-resize", "ew-resize", "n-resize"
		};
		gui::ECURSOR_ICON icon = cursorControl->getActiveIcon();
		if (icon < 0 || icon >= gui::ECI_COUNT) icon = gui::ECI_NORMAL;
		wl_cursor* cursor = Library.CursorThemeGetCursor(CursorTheme, names[icon]);
		if (!cursor || !cursor->image_count) cursor = Library.CursorThemeGetCursor(CursorTheme, "left_ptr");
		if (!cursor || !cursor->image_count) return;
		wl_cursor_image* image = cursor->images[0];
		wl_buffer* buffer = Library.CursorImageGetBuffer(image);
		Library.ProxyMarshal(reinterpret_cast<wl_proxy*>(CursorSurface), 8, cursorScale);
		Library.ProxyMarshal(reinterpret_cast<wl_proxy*>(CursorSurface), 1, buffer, 0, 0);
		Library.ProxyMarshal(reinterpret_cast<wl_proxy*>(CursorSurface), 2, 0, 0, image->width, image->height);
		Library.ProxyMarshal(reinterpret_cast<wl_proxy*>(CursorSurface), 6);
		Library.ProxyMarshal(reinterpret_cast<wl_proxy*>(Pointer), 0, PointerSerial, CursorSurface,
			static_cast<int32_t>(image->hotspot_x / cursorScale), static_cast<int32_t>(image->hotspot_y / cursorScale));
	}

	void CIrrDeviceLinuxWayland::seatCapabilities(void* data, wl_seat*, uint32_t capabilities) { static_cast<CIrrDeviceLinuxWayland*>(data)->bindSeat(capabilities); }
	void CIrrDeviceLinuxWayland::seatName(void*, wl_seat*, const char*) {}

	void CIrrDeviceLinuxWayland::bindSeat(uint32_t capabilities)
	{
		if ((capabilities & WL_SEAT_CAPABILITY_POINTER) && !Pointer)
		{
			Pointer = reinterpret_cast<wl_pointer*>(Library.ProxyMarshalConstructor(reinterpret_cast<wl_proxy*>(Seat), 0, Library.PointerInterface, 0));
			static const wl_pointer_listener listener = { pointerEnter, pointerLeave, pointerMotion, pointerButton, pointerAxis, pointerFrame, pointerAxisSource, pointerAxisStop, pointerAxisDiscrete };
			Library.ProxyAddListener(reinterpret_cast<wl_proxy*>(Pointer), reinterpret_cast<void (**)(void)>(const_cast<wl_pointer_listener*>(&listener)), this);
		}
		else if (!(capabilities & WL_SEAT_CAPABILITY_POINTER) && Pointer)
		{
			if (Library.ProxyGetVersion(reinterpret_cast<wl_proxy*>(Pointer)) >= 3)
				Library.ProxyMarshal(reinterpret_cast<wl_proxy*>(Pointer), 1);
			Library.ProxyDestroy(reinterpret_cast<wl_proxy*>(Pointer));
			Pointer = 0;
			PointerSerial = 0;
		}
		if ((capabilities & WL_SEAT_CAPABILITY_KEYBOARD) && !Keyboard)
		{
			Keyboard = reinterpret_cast<wl_keyboard*>(Library.ProxyMarshalConstructor(reinterpret_cast<wl_proxy*>(Seat), 1, Library.KeyboardInterface, 0));
			static const wl_keyboard_listener listener = { keyboardKeymap, keyboardEnter, keyboardLeave, keyboardKey, keyboardModifiers, keyboardRepeatInfo };
			Library.ProxyAddListener(reinterpret_cast<wl_proxy*>(Keyboard), reinterpret_cast<void (**)(void)>(const_cast<wl_keyboard_listener*>(&listener)), this);
		}
		else if (!(capabilities & WL_SEAT_CAPABILITY_KEYBOARD) && Keyboard)
		{
			if (Library.ProxyGetVersion(reinterpret_cast<wl_proxy*>(Keyboard)) >= 3)
				Library.ProxyMarshal(reinterpret_cast<wl_proxy*>(Keyboard), 0);
			Library.ProxyDestroy(reinterpret_cast<wl_proxy*>(Keyboard));
			Keyboard = 0;
			KeyboardSerial = 0;
			RepeatKey = 0;
			ShiftDown = false;
			ControlDown = false;
		}
		if (TextInputManager && !TextInput)
		{
			TextInput = waylandGetTextInput(Library, TextInputManager, Seat);
			static const zwp_text_input_v3_listener listener = { textInputEnter, textInputLeave, textInputPreedit, textInputCommit, textInputDelete, textInputDone };
			Library.ProxyAddListener(reinterpret_cast<wl_proxy*>(TextInput), reinterpret_cast<void (**)(void)>(const_cast<zwp_text_input_v3_listener*>(&listener)), this);
		}
	}

	void CIrrDeviceLinuxWayland::initializeDataDevice()
	{
		if (!DataDeviceManager || !Seat || DataDevice)
			return;
		DataDevice = reinterpret_cast<wl_data_device*>(Library.ProxyMarshalConstructor(
			reinterpret_cast<wl_proxy*>(DataDeviceManager), 1, Library.DataDeviceInterface, 0, Seat));
		if (!DataDevice)
			return;
		static const wl_data_device_listener listener = { dataDeviceOffer, dataDeviceEnter, dataDeviceLeave,
			dataDeviceMotion, dataDeviceDrop, dataDeviceSelection };
		Library.ProxyAddListener(reinterpret_cast<wl_proxy*>(DataDevice),
			reinterpret_cast<void (**)(void)>(const_cast<wl_data_device_listener*>(&listener)), this);
	}

	void CIrrDeviceLinuxWayland::destroyDataOffer(wl_data_offer*& offer)
	{
		if (!offer)
			return;
		Library.ProxyMarshal(reinterpret_cast<wl_proxy*>(offer), 2);
		Library.ProxyDestroy(reinterpret_cast<wl_proxy*>(offer));
		offer = 0;
	}

	void CIrrDeviceLinuxWayland::dataDeviceOffer(void* data, wl_data_device*, wl_data_offer* offer)
	{
		CIrrDeviceLinuxWayland* self = static_cast<CIrrDeviceLinuxWayland*>(data);
		if (self->PendingDataOffer && self->PendingDataOffer != self->SelectionDataOffer)
			self->destroyDataOffer(self->PendingDataOffer);
		self->PendingDataOffer = offer;
		self->PendingOfferHasText = false;
		self->PendingOfferMime = "";
		static const wl_data_offer_listener listener = { dataOfferMime, dataOfferSourceActions, dataOfferAction };
		self->Library.ProxyAddListener(reinterpret_cast<wl_proxy*>(offer),
			reinterpret_cast<void (**)(void)>(const_cast<wl_data_offer_listener*>(&listener)), self);
	}

	void CIrrDeviceLinuxWayland::dataDeviceEnter(void*, wl_data_device*, uint32_t, wl_surface*, wl_fixed_t, wl_fixed_t, wl_data_offer*) {}
	void CIrrDeviceLinuxWayland::dataDeviceLeave(void*, wl_data_device*) {}
	void CIrrDeviceLinuxWayland::dataDeviceMotion(void*, wl_data_device*, uint32_t, wl_fixed_t, wl_fixed_t) {}
	void CIrrDeviceLinuxWayland::dataDeviceDrop(void*, wl_data_device*) {}

	void CIrrDeviceLinuxWayland::dataDeviceSelection(void* data, wl_data_device*, wl_data_offer* offer)
	{
		CIrrDeviceLinuxWayland* self = static_cast<CIrrDeviceLinuxWayland*>(data);
		if (self->SelectionDataOffer && self->SelectionDataOffer != offer)
			self->destroyDataOffer(self->SelectionDataOffer);
		self->SelectionDataOffer = offer;
		self->SelectionOfferHasText = offer && offer == self->PendingDataOffer && self->PendingOfferHasText;
		self->SelectionOfferMime = self->SelectionOfferHasText ? self->PendingOfferMime : "";
		if (offer == self->PendingDataOffer)
			self->PendingDataOffer = 0;
	}

	void CIrrDeviceLinuxWayland::dataOfferMime(void* data, wl_data_offer* offer, const char* mime)
	{
		CIrrDeviceLinuxWayland* self = static_cast<CIrrDeviceLinuxWayland*>(data);
		if (offer != self->PendingDataOffer || !mime)
			return;
		if (!std::strcmp(mime, "text/plain;charset=utf-8"))
		{
			self->PendingOfferHasText = true;
			self->PendingOfferMime = mime;
		}
		else if (!std::strcmp(mime, "text/plain") && self->PendingOfferMime.empty())
		{
			self->PendingOfferHasText = true;
			self->PendingOfferMime = mime;
		}
	}
	void CIrrDeviceLinuxWayland::dataOfferSourceActions(void*, wl_data_offer*, uint32_t) {}
	void CIrrDeviceLinuxWayland::dataOfferAction(void*, wl_data_offer*, uint32_t) {}
	void CIrrDeviceLinuxWayland::dataSourceTarget(void*, wl_data_source*, const char*) {}

	void CIrrDeviceLinuxWayland::dataSourceSend(void* data, wl_data_source*, const char* mime, int32_t fd)
	{
		CIrrDeviceLinuxWayland* self = static_cast<CIrrDeviceLinuxWayland*>(data);
		if (mime && (!std::strcmp(mime, "text/plain;charset=utf-8") || !std::strcmp(mime, "text/plain")))
		{
			const char* cursor = self->Clipboard.c_str();
			size_t remaining = self->Clipboard.size();
			while (remaining)
			{
				const ssize_t count = write(fd, cursor, remaining);
				if (count > 0) { cursor += count; remaining -= static_cast<size_t>(count); }
				else if (errno != EINTR) break;
			}
		}
		close(fd);
	}

	void CIrrDeviceLinuxWayland::dataSourceCancelled(void* data, wl_data_source* source)
	{
		CIrrDeviceLinuxWayland* self = static_cast<CIrrDeviceLinuxWayland*>(data);
		if (source != self->DataSource)
			return;
		self->Library.ProxyMarshal(reinterpret_cast<wl_proxy*>(source), 1);
		self->Library.ProxyDestroy(reinterpret_cast<wl_proxy*>(source));
		self->DataSource = 0;
	}
	void CIrrDeviceLinuxWayland::dataSourceDropPerformed(void*, wl_data_source*) {}
	void CIrrDeviceLinuxWayland::dataSourceFinished(void*, wl_data_source*) {}
	void CIrrDeviceLinuxWayland::dataSourceAction(void*, wl_data_source*, uint32_t) {}

	void CIrrDeviceLinuxWayland::pointerEnter(void* data, wl_pointer*, uint32_t serial, wl_surface*, wl_fixed_t x, wl_fixed_t y)
	{
		CIrrDeviceLinuxWayland* self = static_cast<CIrrDeviceLinuxWayland*>(data);
		self->PointerSerial = serial;
		static_cast<CCursorControl*>(self->CursorControl)->updatePosition(wl_fixed_to_int(x), wl_fixed_to_int(y));
		self->updateCursor();
	}
	void CIrrDeviceLinuxWayland::pointerLeave(void* data, wl_pointer*, uint32_t, wl_surface*) { static_cast<CIrrDeviceLinuxWayland*>(data)->PointerSerial = 0; }
	void CIrrDeviceLinuxWayland::pointerMotion(void* data, wl_pointer*, uint32_t, wl_fixed_t x, wl_fixed_t y)
	{
		CIrrDeviceLinuxWayland* self = static_cast<CIrrDeviceLinuxWayland*>(data);
		CCursorControl* cursor = static_cast<CCursorControl*>(self->CursorControl);
		cursor->updatePosition(wl_fixed_to_int(x), wl_fixed_to_int(y));
		SEvent event = {};
		event.EventType = EET_MOUSE_INPUT_EVENT;
		event.MouseInput.Event = EMIE_MOUSE_MOVED;
		event.MouseInput.X = cursor->getPosition(false).X;
		event.MouseInput.Y = cursor->getPosition(false).Y;
		event.MouseInput.ButtonStates = self->PointerButtons;
		event.MouseInput.Shift = self->ShiftDown;
		event.MouseInput.Control = self->ControlDown;
		self->postEventFromUser(event);
	}
	void CIrrDeviceLinuxWayland::pointerButton(void* data, wl_pointer*, uint32_t serial, uint32_t, uint32_t button, uint32_t state)
	{
		CIrrDeviceLinuxWayland* self = static_cast<CIrrDeviceLinuxWayland*>(data);
		self->PointerSerial = serial;
		const bool pressed = state == WL_POINTER_BUTTON_STATE_PRESSED;
		EMOUSE_INPUT_EVENT type = EMIE_MOUSE_MOVED;
		u32 mask = 0;
		if (button == IRR_WAYLAND_BTN_LEFT) { type = pressed ? EMIE_LMOUSE_PRESSED_DOWN : EMIE_LMOUSE_LEFT_UP; mask = EMBSM_LEFT; }
		else if (button == IRR_WAYLAND_BTN_RIGHT) { type = pressed ? EMIE_RMOUSE_PRESSED_DOWN : EMIE_RMOUSE_LEFT_UP; mask = EMBSM_RIGHT; }
		else if (button == IRR_WAYLAND_BTN_MIDDLE) { type = pressed ? EMIE_MMOUSE_PRESSED_DOWN : EMIE_MMOUSE_LEFT_UP; mask = EMBSM_MIDDLE; }
		else return;
		if (pressed) self->PointerButtons |= mask; else self->PointerButtons &= ~mask;
		CCursorControl* cursor = static_cast<CCursorControl*>(self->CursorControl);
		SEvent event = {};
		event.EventType = EET_MOUSE_INPUT_EVENT;
		event.MouseInput.Event = type;
		event.MouseInput.X = cursor->getPosition(false).X;
		event.MouseInput.Y = cursor->getPosition(false).Y;
		event.MouseInput.ButtonStates = self->PointerButtons;
		event.MouseInput.Shift = self->ShiftDown;
		event.MouseInput.Control = self->ControlDown;
		self->postEventFromUser(event);
		if (pressed)
		{
			u32 clicks = self->checkSuccessiveClicks(event.MouseInput.X, event.MouseInput.Y, type);
			if (clicks == 2 || clicks == 3)
			{
				event.MouseInput.Event = static_cast<EMOUSE_INPUT_EVENT>(EMIE_LMOUSE_DOUBLE_CLICK + (type - EMIE_LMOUSE_PRESSED_DOWN) + (clicks - 2) * 3);
				self->postEventFromUser(event);
			}
		}
	}
	void CIrrDeviceLinuxWayland::pointerAxis(void* data, wl_pointer*, uint32_t, uint32_t axis, wl_fixed_t value)
	{
		if (axis != WL_POINTER_AXIS_VERTICAL_SCROLL) return;
		CIrrDeviceLinuxWayland* self = static_cast<CIrrDeviceLinuxWayland*>(data);
		CCursorControl* cursor = static_cast<CCursorControl*>(self->CursorControl);
		SEvent event = {};
		event.EventType = EET_MOUSE_INPUT_EVENT;
		event.MouseInput.Event = EMIE_MOUSE_WHEEL;
		event.MouseInput.X = cursor->getPosition(false).X;
		event.MouseInput.Y = cursor->getPosition(false).Y;
		event.MouseInput.Wheel = static_cast<f32>(-wl_fixed_to_double(value) / 10.0);
		event.MouseInput.ButtonStates = self->PointerButtons;
		event.MouseInput.Shift = self->ShiftDown;
		event.MouseInput.Control = self->ControlDown;
		self->postEventFromUser(event);
	}
	void CIrrDeviceLinuxWayland::pointerFrame(void*, wl_pointer*) {}
	void CIrrDeviceLinuxWayland::pointerAxisSource(void*, wl_pointer*, uint32_t) {}
	void CIrrDeviceLinuxWayland::pointerAxisStop(void*, wl_pointer*, uint32_t, uint32_t) {}
	void CIrrDeviceLinuxWayland::pointerAxisDiscrete(void*, wl_pointer*, uint32_t, int32_t) {}

	void CIrrDeviceLinuxWayland::keyboardKeymap(void* data, wl_keyboard*, uint32_t format, int32_t fd, uint32_t size)
	{
		CIrrDeviceLinuxWayland* self = static_cast<CIrrDeviceLinuxWayland*>(data);
		if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) { close(fd); return; }
		char* map = static_cast<char*>(mmap(0, size, PROT_READ, MAP_PRIVATE, fd, 0));
		if (map == MAP_FAILED) { close(fd); return; }
		if (!self->XkbContext) self->XkbContext = self->Library.XkbContextNew(XKB_CONTEXT_NO_FLAGS);
		xkb_keymap* keymap = self->Library.XkbKeymapNewFromString(self->XkbContext, map, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
		munmap(map, size); close(fd);
		if (!keymap) return;
		if (self->XkbState) self->Library.XkbStateUnref(self->XkbState);
		if (self->XkbKeymap) self->Library.XkbKeymapUnref(self->XkbKeymap);
		self->XkbKeymap = keymap;
		self->XkbState = self->Library.XkbStateNew(keymap);
	}
	void CIrrDeviceLinuxWayland::keyboardEnter(void* data, wl_keyboard*, uint32_t serial, wl_surface*, wl_array*) { CIrrDeviceLinuxWayland* self = static_cast<CIrrDeviceLinuxWayland*>(data); self->KeyboardSerial = serial; self->WindowHasFocus = true; }
	void CIrrDeviceLinuxWayland::keyboardLeave(void* data, wl_keyboard*, uint32_t, wl_surface*) { CIrrDeviceLinuxWayland* self = static_cast<CIrrDeviceLinuxWayland*>(data); self->WindowHasFocus = false; self->RepeatKey = 0; self->ShiftDown = false; self->ControlDown = false; }
	void CIrrDeviceLinuxWayland::keyboardKey(void* data, wl_keyboard*, uint32_t serial, uint32_t, uint32_t key, uint32_t state)
	{
		CIrrDeviceLinuxWayland* self = static_cast<CIrrDeviceLinuxWayland*>(data);
		self->KeyboardSerial = serial;
		const bool pressed = state == WL_KEYBOARD_KEY_STATE_PRESSED;
		self->postKey(key, pressed);
		if (pressed && self->RepeatRate > 0 && self->XkbKeymap && self->Library.XkbKeymapKeyRepeats(self->XkbKeymap, key + 8)) { self->RepeatKey = key; self->RepeatAt = os::Timer::getTime() + self->RepeatDelay; }
		else if (!pressed && self->RepeatKey == key) self->RepeatKey = 0;
	}
	void CIrrDeviceLinuxWayland::keyboardModifiers(void* data, wl_keyboard*, uint32_t, uint32_t depressed, uint32_t latched, uint32_t locked, uint32_t group)
	{
		CIrrDeviceLinuxWayland* self = static_cast<CIrrDeviceLinuxWayland*>(data);
		if (self->XkbState) self->Library.XkbStateUpdateMask(self->XkbState, depressed, latched, locked, 0, 0, group);
	}
	void CIrrDeviceLinuxWayland::keyboardRepeatInfo(void* data, wl_keyboard*, int32_t rate, int32_t delay) { CIrrDeviceLinuxWayland* self = static_cast<CIrrDeviceLinuxWayland*>(data); self->RepeatRate = rate; self->RepeatDelay = delay; }

	EKEY_CODE CIrrDeviceLinuxWayland::mapKey(xkb_keysym_t sym) const
	{
		if (sym >= XKB_KEY_a && sym <= XKB_KEY_z) return static_cast<EKEY_CODE>(KEY_KEY_A + sym - XKB_KEY_a);
		if (sym >= XKB_KEY_A && sym <= XKB_KEY_Z) return static_cast<EKEY_CODE>(KEY_KEY_A + sym - XKB_KEY_A);
		if (sym >= XKB_KEY_0 && sym <= XKB_KEY_9) return static_cast<EKEY_CODE>(KEY_KEY_0 + sym - XKB_KEY_0);
		if (sym >= XKB_KEY_F1 && sym <= XKB_KEY_F24) return static_cast<EKEY_CODE>(KEY_F1 + sym - XKB_KEY_F1);
		if (sym >= XKB_KEY_KP_0 && sym <= XKB_KEY_KP_9) return static_cast<EKEY_CODE>(KEY_NUMPAD0 + sym - XKB_KEY_KP_0);
		switch (sym)
		{
		case XKB_KEY_Return: case XKB_KEY_KP_Enter: return KEY_RETURN;
		case XKB_KEY_Escape: return KEY_ESCAPE; case XKB_KEY_BackSpace: return KEY_BACK;
		case XKB_KEY_Tab: case XKB_KEY_ISO_Left_Tab: return KEY_TAB; case XKB_KEY_space: return KEY_SPACE; case XKB_KEY_Left: return KEY_LEFT;
		case XKB_KEY_Right: return KEY_RIGHT; case XKB_KEY_Up: return KEY_UP; case XKB_KEY_Down: return KEY_DOWN;
		case XKB_KEY_Home: return KEY_HOME; case XKB_KEY_End: return KEY_END; case XKB_KEY_Page_Up: return KEY_PRIOR;
		case XKB_KEY_Page_Down: return KEY_NEXT; case XKB_KEY_Insert: return KEY_INSERT; case XKB_KEY_Delete: return KEY_DELETE;
		case XKB_KEY_Pause: return KEY_PAUSE; case XKB_KEY_Print: return KEY_SNAPSHOT;
		case XKB_KEY_Caps_Lock: return KEY_CAPITAL; case XKB_KEY_Num_Lock: return KEY_NUMLOCK; case XKB_KEY_Scroll_Lock: return KEY_SCROLL;
		case XKB_KEY_Shift_L: return KEY_LSHIFT; case XKB_KEY_Shift_R: return KEY_RSHIFT;
		case XKB_KEY_Control_L: return KEY_LCONTROL; case XKB_KEY_Control_R: return KEY_RCONTROL;
		case XKB_KEY_Alt_L: return KEY_LMENU; case XKB_KEY_Alt_R: return KEY_RMENU;
		case XKB_KEY_Super_L: return KEY_LWIN; case XKB_KEY_Super_R: return KEY_RWIN; case XKB_KEY_Menu: return KEY_APPS;
		case XKB_KEY_KP_Multiply: return KEY_MULTIPLY; case XKB_KEY_KP_Add: return KEY_ADD;
		case XKB_KEY_KP_Subtract: return KEY_SUBTRACT; case XKB_KEY_KP_Decimal: return KEY_DECIMAL; case XKB_KEY_KP_Divide: return KEY_DIVIDE;
		case XKB_KEY_semicolon: case XKB_KEY_colon: return KEY_OEM_1;
		case XKB_KEY_equal: case XKB_KEY_plus: return KEY_PLUS;
		case XKB_KEY_comma: case XKB_KEY_less: return KEY_COMMA;
		case XKB_KEY_minus: case XKB_KEY_underscore: return KEY_MINUS;
		case XKB_KEY_period: case XKB_KEY_greater: return KEY_PERIOD;
		case XKB_KEY_slash: case XKB_KEY_question: return KEY_OEM_2;
		case XKB_KEY_grave: case XKB_KEY_asciitilde: return KEY_OEM_3;
		case XKB_KEY_bracketleft: case XKB_KEY_braceleft: return KEY_OEM_4;
		case XKB_KEY_backslash: case XKB_KEY_bar: return KEY_OEM_5;
		case XKB_KEY_bracketright: case XKB_KEY_braceright: return KEY_OEM_6;
		case XKB_KEY_apostrophe: case XKB_KEY_quotedbl: return KEY_OEM_7;
		default: return KEY_NONE;
		}
	}

	void CIrrDeviceLinuxWayland::postKey(uint32_t key, bool pressed, bool autoRepeat)
	{
		if (!XkbState) return;
		const xkb_keycode_t code = key + 8;
		const xkb_keysym_t sym = Library.XkbStateKeyGetOneSym(XkbState, code);
		const EKEY_CODE mapped = mapKey(sym);
		if (mapped == KEY_LSHIFT || mapped == KEY_RSHIFT) ShiftDown = pressed;
		if (mapped == KEY_LCONTROL || mapped == KEY_RCONTROL) ControlDown = pressed;
		wchar_t character = 0;
		if (pressed)
		{
			char utf8[16] = {};
			if (Library.XkbStateKeyGetUtf8(XkbState, code, utf8, sizeof(utf8)) > 0)
			{
				wchar_t wide[4] = {};
				core::utf8ToWchar(utf8, wide, sizeof(wide));
				character = wide[0];
			}
		}
		SEvent event = {};
		event.EventType = EET_KEY_INPUT_EVENT;
		event.KeyInput.Char = character;
		event.KeyInput.Key = mapped;
		event.KeyInput.PressedDown = pressed;
		event.KeyInput.Shift = ShiftDown;
		event.KeyInput.Control = ControlDown;
		event.KeyInput.AutoRepeat = autoRepeat;
		postEventFromUser(event);
	}

	void CIrrDeviceLinuxWayland::updateTextInputFocus()
	{
		if (!TextInput || !TextInputEntered || !GUIEnvironment) return;
		gui::IGUIElement* focus = GUIEnvironment->getFocus();
		const bool enabled = focus && focus->getType() == gui::EGUIET_EDIT_BOX && focus->isEnabled();
		if (enabled != TextInputEnabled)
		{
			TextInputEnabled = enabled;
			if (enabled) waylandTextInputEnable(Library, TextInput); else waylandTextInputDisable(Library, TextInput);
		}
		if (enabled)
		{
			const core::rect<s32>& rect = focus->getAbsolutePosition();
			waylandTextInputSetCursorRectangle(Library, TextInput, rect.UpperLeftCorner.X, rect.LowerRightCorner.Y - 1, 1, 1);
		}
	}

	void CIrrDeviceLinuxWayland::postCommittedText(const char* utf8)
	{
		if (!utf8) return;
		wchar_t* text = core::toWideChar(utf8);
		for (const wchar_t* p = text; *p; ++p)
		{
			SEvent event = {};
			event.EventType = EET_KEY_INPUT_EVENT;
			event.KeyInput.Char = *p;
			event.KeyInput.Key = KEY_ACCEPT;
			event.KeyInput.PressedDown = true;
			postEventFromUser(event);
			event.KeyInput.PressedDown = false;
			postEventFromUser(event);
		}
		delete [] text;
	}
	void CIrrDeviceLinuxWayland::textInputEnter(void* data, zwp_text_input_v3*, wl_surface*) { static_cast<CIrrDeviceLinuxWayland*>(data)->TextInputEntered = true; }
	void CIrrDeviceLinuxWayland::textInputLeave(void* data, zwp_text_input_v3*, wl_surface*) { CIrrDeviceLinuxWayland* self = static_cast<CIrrDeviceLinuxWayland*>(data); self->TextInputEntered = false; self->TextInputEnabled = false; }
	void CIrrDeviceLinuxWayland::textInputPreedit(void*, zwp_text_input_v3*, const char*, int32_t, int32_t) {}
	void CIrrDeviceLinuxWayland::textInputCommit(void* data, zwp_text_input_v3*, const char* text) { static_cast<CIrrDeviceLinuxWayland*>(data)->postCommittedText(text); }
	void CIrrDeviceLinuxWayland::textInputDelete(void*, zwp_text_input_v3*, uint32_t, uint32_t) {}
	void CIrrDeviceLinuxWayland::textInputDone(void*, zwp_text_input_v3*, uint32_t) {}
}

#endif
