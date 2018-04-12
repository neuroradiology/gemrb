/* GemRB - Infinity Engine Emulator
 * Copyright (C) 2003 The GemRB Project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 */

#include "GUI/Console.h"

#include "win32def.h"

#include "GameData.h"
#include "Interface.h"
#include "Palette.h"
#include "ScriptEngine.h"
#include "Sprite2D.h"
#include "GUI/EventMgr.h"
#include "GUI/TextSystem/Font.h"

namespace GemRB {

Console::Console(const Region& frame)
	: Control(frame), History(5)
{
	max = 128;
	Buffer.reserve(max);
	CurPos = 0;
	HistPos = 0;
	palette = new Palette( ColorWhite, ColorBlack );
	Sprite2D* cursor = core->GetCursorSprite();
	SetCursor(cursor);
	cursor->release();

	EventMgr::EventCallback* cb = new MethodCallback<Console, const Event&, bool>(this, &Console::HandleHotKey);
	if (!EventMgr::RegisterHotKeyCallback(cb, ' ', GEM_MOD_CTRL)) {
		delete cb;
	}
}

Console::~Console(void)
{
	palette->release();
}

bool Console::HandleHotKey(const Event& e)
{
	if (e.type != Event::KeyDown) return false;

	// the only hot key console registers is for hiding / showing itself
	if (IsVisible()) {
		window->Close();
	} else {
		window->Focus();
	}
	return true;
}

int Console::DrawingShift(ieWord textw) const
{
	Sprite2D* cursor = Cursor();

	int shift = 0;
	if (textw + cursor->Width > frame.w) {
		// shift left so the cursor remains visible
		shift = (textw + cursor->Width) - frame.w;
	}
	return shift;
}

void Console::MouseDown(const MouseEvent& me, unsigned short /*Mod*/)
{
	Point p = ConvertPointFromScreen(me.Pos());

	Font* font = core->GetTextFont();
	ieWord w = font->StringSize(Buffer.substr(0, CurPos)).w;
	int shift = -DrawingShift(w);

	Size size(p.x + shift, font->LineHeight);
	Font::StringSizeMetrics metrics = {size, 0, true};
	if (shift) {
		w = font->StringSize(Buffer.substr(CurPos, String::npos), &metrics).w;
	} else {
		w = font->StringSize(Buffer, &metrics).w;
	}

	CurPos = metrics.numChars;
	MarkDirty();
}

/** Draws the Console on the Output Display */
void Console::DrawSelf(Region drawFrame, const Region& clip)
{
	Font* font = core->GetTextFont();
	Sprite2D* cursor = Cursor();

	Video* video = core->GetVideoDriver();
	video->DrawRect( clip, ColorBlack );

	ieWord w = font->StringSize(Buffer.substr(0, CurPos)).w;
	int shift = DrawingShift(w);
	drawFrame.x -= shift;
	drawFrame.w += shift;

	font->Print( drawFrame, Buffer, palette, IE_FONT_ALIGN_LEFT | IE_FONT_ALIGN_MIDDLE | IE_FONT_SINGLE_LINE);

	ieWord vcenter = (drawFrame.h / 2) + (cursor->Height / 2);
	video->BlitSprite(cursor, w + drawFrame.x, vcenter + drawFrame.y);
}

/** Sets the Text of the current control */
void Console::SetText(const String& string)
{
	Buffer = string;
}
/** Key Press Event */
bool Console::OnKeyPress(const KeyboardEvent& Key, unsigned short /*Mod*/)
{
	switch (Key.keycode) {
		case GEM_BACKSP:
			if (CurPos != 0) {
				Buffer.erase(--CurPos, 1);
			}
			break;
		case GEM_HOME:
			CurPos = 0;
			break;
		case GEM_END:
			CurPos = Buffer.length();
			break;
		case GEM_UP:
			HistoryBack();
			break;
		case GEM_DOWN:
			HistoryForward();
			break;
		case GEM_LEFT:
			if (CurPos > 0)
				CurPos--;
			break;
		case GEM_RIGHT:
			if (CurPos < Buffer.length()) {
				CurPos++;
			}
			break;
		case GEM_DELETE:
			if (CurPos < Buffer.length()) {
				Buffer.erase(CurPos, 1);
			}
			break;			
		case GEM_RETURN:
			{
				if (Buffer.length()) {
					char* cBuf = MBCStringFromString(Buffer);
					assert(cBuf);
					ScriptEngine::FunctionParameters params;
					params.push_back(ScriptEngine::Parameter(cBuf));
					core->GetGUIScriptEngine()->RunFunction("Console", "Exec", params);
					free(cBuf);
					HistoryAdd();
					Buffer.erase();
					CurPos = 0;
					HistPos = 0;
				}
			}
			break;
		default:
			if (Key.character) {
				if (Buffer.length() < max) {
					Buffer.insert(CurPos++, 1, Key.character);
				}
				break;
			}
			return false;
	}
	MarkDirty();
	return true;
}

void Console::HistoryBack()
{
	if (Buffer[0] && HistPos == 0 && History.Retrieve(HistPos) != Buffer) {
		HistoryAdd();
		HistPos++;
	}
	Buffer = History.Retrieve(HistPos);
	CurPos = Buffer.length();
	if (++HistPos >= (int)History.Size()) {
		HistPos--;
	}
}

void Console::HistoryForward()
{
	if (--HistPos < 0) {
		Buffer.erase();
		HistPos++;
	} else {
		Buffer = History.Retrieve(HistPos);
	}
	CurPos = Buffer.length();
}

void Console::HistoryAdd(bool force)
{
	if (force || Buffer.length()) {
		History.Append(Buffer, !force);
	}
}

void Console::SetFocus()
{
	Control::SetFocus();
	if (IsFocused()) {
		core->GetVideoDriver()->ShowSoftKeyboard();
	}
}

bool Console::SetEvent(int /*eventType*/, ControlEventHandler /*handler*/)
{
	return false;
}

}
