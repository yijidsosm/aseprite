// Aseprite
// Copyright (C) 2022  Igara Studio S.A.
//
// This program is distributed under the terms of
// the End-User License Agreement for Aseprite.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/ui/editor/delayed_mouse_move.h"

#include "app/ui/editor/editor.h"

namespace app {

DelayedMouseMove::DelayedMouseMove(DelayedMouseMoveDelegate* delegate,
                                   Editor* editor,
                                   const int interval)
  : m_delegate(delegate)
  , m_editor(editor)
  , m_timer(interval)
  , m_spritePos(std::numeric_limits<int>::min(),
                std::numeric_limits<int>::min())
{
  ASSERT(m_delegate);
  m_timer.Tick.connect([this] { commitMouseMove(); });
}

void DelayedMouseMove::onMouseDown(const ui::MouseMessage* msg)
{
  updateSpritePos(msg);
}

bool DelayedMouseMove::onMouseMove(const ui::MouseMessage* msg)
{
  if (!updateSpritePos(msg))
    return false;

  if (!m_timer.isRunning()) {
    if (m_timer.interval() > 0) {
      m_timer.start();
    }
    else {
      // Commit immediately
      commitMouseMove();
    }
  }
  return true;
}

void DelayedMouseMove::onMouseUp(const ui::MouseMessage* msg)
{
  updateSpritePos(msg);
  commitMouseMove();
}

void DelayedMouseMove::commitMouseMove()
{
  if (m_timer.isRunning())
    m_timer.stop();

  m_delegate->onCommitMouseMove(m_editor, spritePos());
}

const gfx::Point& DelayedMouseMove::spritePos() const
{
  ASSERT(m_spritePos.x != std::numeric_limits<int>::min() &&
         m_spritePos.y != std::numeric_limits<int>::min());
  return m_spritePos;
}

bool DelayedMouseMove::updateSpritePos(const ui::MouseMessage* msg)
{
  // The autoScroll() function controls the "infinite scroll" when we
  // touch the viewport borders.
  const gfx::Point mousePos = m_editor->autoScroll(msg, AutoScroll::MouseDir);
  const gfx::Point spritePos = m_editor->screenToEditor(mousePos);

  // Avoid redrawing everything if the position in the canvas didn't
  // change.
  //
  // TODO Remove this if we add support for anti-aliasing in the
  //      transformations.
  if (m_spritePos != spritePos) {
    m_spritePos = spritePos;
    return true;
  }
  else
    return false;
}

} // namespace app
