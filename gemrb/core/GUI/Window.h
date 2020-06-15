/* GemRB - Infinity Engine Emulator
 * Copyright (C) 2003 The GemRB Project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 */

/**
 * @file Window.h
 * Declares Window, class serving as a container for Control/widget objects
 * and displaying windows in GUI
 * @author The GemRB Project
 */

#ifndef WINDOW_H
#define WINDOW_H

#include "ScrollView.h"
#include "WindowManager.h"

#include <set>

namespace GemRB {

class Control;
class Sprite2D;

/**
 * @class Window
 * Class serving as a container for Control/widget objects
 * and displaying windows in GUI.
 */

using WindowActionResponder = View::ActionResponder<Window*>;

class GEM_EXPORT Window : public ScrollView, public WindowActionResponder {
public:
	// !!! Keep these synchronized with GUIDefines.py !!!
	enum WindowPosition {
		PosTop = 1,		// top
		PosBottom = 2,	// bottom
		PosVmid = 3,	// top + bottom = vmid
		PosLeft = 4,	// left
		PosRight = 8,	// right
		PosHmid = 12,	// left + right = hmid
		PosCentered = 15// top + bottom + left + right = center
	};

	enum WindowFlags {
		Draggable = 1,			// window can be dragged (by clicking anywhere in its frame) a la PST radial menu
		Borderless = 2,			// window will not draw the ornate borders (see WindowManager::DrawWindows)
		DestroyOnClose = 4,		// window will be deleted on close rather then hidden and sent to the back
		AlphaChannel = 8,		// Create window with RGBA buffer suitable for creating non rectangular windows
		Modal = 16,
		NoSounds = 32			// doesn't play the open/close sounds
	};
	
	enum Action : WindowActionResponder::Action {
		// !!! Keep these synchronized with GUIDefines.py !!!
		Closed = 0,
		LostFocus = 1,
		GainedFocus = 2
	};
	
	using WindowEventHandler = WindowActionResponder::Responder;

private:
	void RecreateBuffer();

	void SubviewAdded(View* view, View* parent) override;
	void SubviewRemoved(View* view, View* parent) override;

	void FlagsChanged(unsigned int /*oldflags*/) override;
	void SizeChanged(const Size&) override;
	void WillDraw() override;

	// attempt to set focus to view. return the focused view which is view if success or the currently focused view (if any) on failure
	View* TrySetFocus(View* view);
	bool IsDragable() const;

	inline void DispatchMouseMotion(View*, const MouseEvent&);
	inline void DispatchMouseDown(View*, const MouseEvent& me, unsigned short /*Mod*/);
	inline void DispatchMouseUp(View*, const MouseEvent& me, unsigned short /*Mod*/);

	inline void DispatchTouchDown(View*, const TouchEvent& te, unsigned short /*Mod*/);
	inline void DispatchTouchUp(View*, const TouchEvent& te, unsigned short /*Mod*/);
	inline void DispatchTouchGesture(View*, const GestureEvent& gesture);
	
	inline bool DispatchKey(View*, const Event&);
	
	bool OnKeyPress(const KeyboardEvent& /*Key*/, unsigned short /*Mod*/) override;
	
	bool OnMouseDrag(const MouseEvent&) override;
	void OnMouseLeave(const MouseEvent& /*me*/, const DragOp*) override;
	
	ViewScriptingRef* CreateScriptingRef(ScriptingId id, ResRef group) override;

public:
	Window(const Region& frame, WindowManager& mgr);

	void Close();
	void Focus();
	bool DisplayModal(WindowManager::ModalShadow = WindowManager::ShadowNone);

	/** Sets 'view' as Focused */
	void SetFocused(View* view);
	View* FocusedView() const { return focusView; }

	void SetPosition(WindowPosition);
	String TooltipText() const override;
	Sprite2D* Cursor() const override;
	bool IsDisabledCursor() const override;
	bool IsReceivingEvents() const override { return true; }

	const VideoBufferPtr& DrawWithoutComposition();
	void RedrawControls(const char* VarName, unsigned int Sum);

	bool DispatchEvent(const Event&);
	bool RegisterHotKeyCallback(EventMgr::EventCallback, KeyboardKey key);
	bool UnRegisterHotKeyCallback(EventMgr::EventCallback, KeyboardKey key);
	
	bool IsOpaque() const override { return (Flags()&AlphaChannel) == 0; }
	bool HitTest(const Point& p) const override;

	bool InActionHandler() const;
	void SetAction(Responder handler, const ActionKey& key) override;
	bool PerformAction(const ActionKey& action) override;
	bool SupportsAction(const ActionKey& action) override;

private: // Private attributes
	typedef std::map<KeyboardKey, EventMgr::EventCallback> KeyMap;

	std::set<Control*> Controls;
	KeyMap HotKeys;

	View* focusView; // keyboard focus
	View* trackingView; // out of bounds mouse tracking
	View* hoverView; // view the mouse was last over
	Holder<DragOp> drag;
	unsigned long lastMouseMoveTime;

	VideoBufferPtr backBuffer;
	WindowManager& manager;
	
	WindowEventHandler eventHandlers[3];
};

}

#endif
