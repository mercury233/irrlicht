#ifndef IRR_C_IRR_DEVICE_LINUX_WAYLAND_H_INCLUDED
#define IRR_C_IRR_DEVICE_LINUX_WAYLAND_H_INCLUDED

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_WAYLAND_DEVICE_

#include "CIrrDeviceStub.h"
#include "IImagePresenter.h"
#include "CWaylandLibrary.h"
#include "CWaylandProtocols.h"

namespace irr
{
	void setActiveWindowScaleFactor(f32 scale);

	class CIrrDeviceLinuxWayland : public CIrrDeviceStub, public video::IImagePresenter
	{
	public:
		CIrrDeviceLinuxWayland(const SIrrlichtCreationParameters& param);
		virtual ~CIrrDeviceLinuxWayland();
		virtual bool run() IRR_OVERRIDE;
		virtual void yield() IRR_OVERRIDE;
		virtual void sleep(u32 timeMs, bool pauseTimer) IRR_OVERRIDE;
		virtual void setWindowCaption(const wchar_t* text) IRR_OVERRIDE;
		virtual bool isWindowActive() const IRR_OVERRIDE;
		virtual bool isWindowFocused() const IRR_OVERRIDE;
		virtual bool isWindowMinimized() const IRR_OVERRIDE;
		virtual video::ECOLOR_FORMAT getColorFormat() const IRR_OVERRIDE;
		virtual bool present(video::IImage*, void* = 0, core::rect<s32>* = 0) IRR_OVERRIDE { return false; }
		virtual void closeDevice() IRR_OVERRIDE;
		virtual video::IVideoModeList* getVideoModeList() IRR_OVERRIDE;
		virtual void setResizable(bool resize = false) IRR_OVERRIDE;
		virtual void setWindowSize(const core::dimension2d<u32>& size) IRR_OVERRIDE;
		virtual void minimizeWindow() IRR_OVERRIDE;
		virtual void maximizeWindow() IRR_OVERRIDE;
		virtual void restoreWindow() IRR_OVERRIDE;
		virtual core::position2di getWindowPosition() IRR_OVERRIDE { return core::position2di(-1, -1); }
		virtual bool activateJoysticks(core::array<SJoystickInfo>&) IRR_OVERRIDE { return false; }
		virtual bool setGammaRamp(f32, f32, f32, f32, f32) IRR_OVERRIDE { return false; }
		virtual bool getGammaRamp(f32&, f32&, f32&, f32&, f32&) IRR_OVERRIDE { return false; }
		virtual void clearSystemMessages() IRR_OVERRIDE;
		virtual E_DEVICE_TYPE getType() const IRR_OVERRIDE { return EIDT_WAYLAND; }

		const c8* getTextFromClipboard() const;
		void copyToClipboard(const c8* text) const;

	private:
		class CCursorControl : public gui::ICursorControl
		{
		public:
			explicit CCursorControl(CIrrDeviceLinuxWayland* device);
			virtual void setVisible(bool visible) IRR_OVERRIDE;
			virtual bool isVisible() const IRR_OVERRIDE { return Visible; }
			virtual void setPosition(const core::position2d<f32>& pos) IRR_OVERRIDE;
			virtual void setPosition(f32 x, f32 y) IRR_OVERRIDE;
			virtual void setPosition(const core::position2d<s32>& pos) IRR_OVERRIDE { Position = pos; }
			virtual void setPosition(s32 x, s32 y) IRR_OVERRIDE { Position.set(x, y); }
			virtual const core::position2d<s32>& getPosition(bool = true) IRR_OVERRIDE { return Position; }
			virtual core::position2d<f32> getRelativePosition(bool = true) IRR_OVERRIDE;
			virtual void setReferenceRect(core::rect<s32>* rect = 0) IRR_OVERRIDE;
			virtual bool getReferenceRect(core::rect<s32>& rect) IRR_OVERRIDE;
			virtual void setActiveIcon(gui::ECURSOR_ICON iconId) IRR_OVERRIDE;
			virtual gui::ECURSOR_ICON getActiveIcon() const IRR_OVERRIDE { return ActiveIcon; }
			virtual core::dimension2di getSupportedIconSize() const IRR_OVERRIDE { return core::dimension2di(64, 64); }
			void updatePosition(s32 x, s32 y) { Position.set(x, y); }
		private:
			CIrrDeviceLinuxWayland* Device;
			core::position2di Position;
			core::rect<s32> ReferenceRect;
			bool UseReferenceRect;
			bool Visible;
			gui::ECURSOR_ICON ActiveIcon;
		};

		bool createWindow();
		void createDriver();
		void destroyWayland();
		void applyConfigure(libdecor_configuration* configuration);
		void applyScale(uint32_t scale120);
		void updateCursor();
		void updateTextInputFocus();
		void postCommittedText(const char* utf8);
		void postKey(uint32_t key, bool pressed, bool autoRepeat = false);
		EKEY_CODE mapKey(xkb_keysym_t sym) const;
		void bindSeat(uint32_t capabilities);
		void initializeDataDevice();
		void destroyDataOffer(wl_data_offer*& offer);
		void* bindGlobal(uint32_t name, const wl_interface* interface, uint32_t version);

		static void registryGlobal(void*, wl_registry*, uint32_t, const char*, uint32_t);
		static void registryRemove(void*, wl_registry*, uint32_t);
		static void decorError(libdecor*, libdecor_error, const char*);
		static void frameConfigure(libdecor_frame*, libdecor_configuration*, void*);
		static void frameClose(libdecor_frame*, void*);
		static void frameCommit(libdecor_frame*, void*);
		static void frameDismissPopup(libdecor_frame*, const char*, void*);
		static void fractionalPreferred(void*, wp_fractional_scale_v1*, uint32_t);
		static void seatCapabilities(void*, wl_seat*, uint32_t);
		static void seatName(void*, wl_seat*, const char*);
		static void pointerEnter(void*, wl_pointer*, uint32_t, wl_surface*, wl_fixed_t, wl_fixed_t);
		static void pointerLeave(void*, wl_pointer*, uint32_t, wl_surface*);
		static void pointerMotion(void*, wl_pointer*, uint32_t, wl_fixed_t, wl_fixed_t);
		static void pointerButton(void*, wl_pointer*, uint32_t, uint32_t, uint32_t, uint32_t);
		static void pointerAxis(void*, wl_pointer*, uint32_t, uint32_t, wl_fixed_t);
		static void pointerFrame(void*, wl_pointer*);
		static void pointerAxisSource(void*, wl_pointer*, uint32_t);
		static void pointerAxisStop(void*, wl_pointer*, uint32_t, uint32_t);
		static void pointerAxisDiscrete(void*, wl_pointer*, uint32_t, int32_t);
		static void keyboardKeymap(void*, wl_keyboard*, uint32_t, int32_t, uint32_t);
		static void keyboardEnter(void*, wl_keyboard*, uint32_t, wl_surface*, wl_array*);
		static void keyboardLeave(void*, wl_keyboard*, uint32_t, wl_surface*);
		static void keyboardKey(void*, wl_keyboard*, uint32_t, uint32_t, uint32_t, uint32_t);
		static void keyboardModifiers(void*, wl_keyboard*, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
		static void keyboardRepeatInfo(void*, wl_keyboard*, int32_t, int32_t);
		static void dataDeviceOffer(void*, wl_data_device*, wl_data_offer*);
		static void dataDeviceEnter(void*, wl_data_device*, uint32_t, wl_surface*, wl_fixed_t, wl_fixed_t, wl_data_offer*);
		static void dataDeviceLeave(void*, wl_data_device*);
		static void dataDeviceMotion(void*, wl_data_device*, uint32_t, wl_fixed_t, wl_fixed_t);
		static void dataDeviceDrop(void*, wl_data_device*);
		static void dataDeviceSelection(void*, wl_data_device*, wl_data_offer*);
		static void dataOfferMime(void*, wl_data_offer*, const char*);
		static void dataOfferSourceActions(void*, wl_data_offer*, uint32_t);
		static void dataOfferAction(void*, wl_data_offer*, uint32_t);
		static void dataSourceTarget(void*, wl_data_source*, const char*);
		static void dataSourceSend(void*, wl_data_source*, const char*, int32_t);
		static void dataSourceCancelled(void*, wl_data_source*);
		static void dataSourceDropPerformed(void*, wl_data_source*);
		static void dataSourceFinished(void*, wl_data_source*);
		static void dataSourceAction(void*, wl_data_source*, uint32_t);
		static void textInputEnter(void*, zwp_text_input_v3*, wl_surface*);
		static void textInputLeave(void*, zwp_text_input_v3*, wl_surface*);
		static void textInputPreedit(void*, zwp_text_input_v3*, const char*, int32_t, int32_t);
		static void textInputCommit(void*, zwp_text_input_v3*, const char*);
		static void textInputDelete(void*, zwp_text_input_v3*, uint32_t, uint32_t);
		static void textInputDone(void*, zwp_text_input_v3*, uint32_t);

		CWaylandLibrary Library;
		wl_display* Display;
		wl_registry* Registry;
		wl_compositor* Compositor;
		wl_surface* Surface;
		wl_surface* CursorSurface;
		wl_shm* Shm;
		wl_seat* Seat;
		wl_pointer* Pointer;
		wl_keyboard* Keyboard;
		wl_data_device_manager* DataDeviceManager;
		wl_data_device* DataDevice;
		wl_data_source* DataSource;
		wl_data_offer* PendingDataOffer;
		wl_data_offer* SelectionDataOffer;
		wp_fractional_scale_manager_v1* FractionalManager;
		wp_fractional_scale_v1* FractionalScale;
		wp_viewporter* Viewporter;
		wp_viewport* Viewport;
		zwp_text_input_manager_v3* TextInputManager;
		zwp_text_input_v3* TextInput;
		libdecor* Decor;
		libdecor_frame* Frame;
		wl_egl_window* EglWindow;
		wl_cursor_theme* CursorTheme;
		xkb_context* XkbContext;
		xkb_keymap* XkbKeymap;
		xkb_state* XkbState;
		core::dimension2d<u32> LogicalSize;
		core::dimension2d<u32> FramebufferSize;
		f32 ScaleFactor;
		uint32_t PointerSerial;
		uint32_t KeyboardSerial;
		uint32_t PointerButtons;
		int32_t RepeatRate;
		int32_t RepeatDelay;
		uint32_t RepeatKey;
		u32 RepeatAt;
		bool Configured;
		bool WindowHasFocus;
		bool WindowMinimized;
		bool TextInputEntered;
		bool TextInputEnabled;
		bool Resizable;
		bool ScaleReported;
		bool ShiftDown;
		bool ControlDown;
		bool PendingOfferHasText;
		bool SelectionOfferHasText;
		core::stringc PendingOfferMime;
		core::stringc SelectionOfferMime;
		mutable core::stringc Clipboard;
	};
}

#endif
#endif
