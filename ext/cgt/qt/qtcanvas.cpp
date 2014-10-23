/**********************************************************************
 *                                                                    *
 * tgt - Tiny Graphics Toolbox                                        *
 *                                                                    *
 * Copyright (C) 2006-2008 Visualization and Computer Graphics Group, *
 * Department of Computer Science, University of Muenster, Germany.   *
 * <http://viscg.uni-muenster.de>                                     *
 *                                                                    *
 * This file is part of the tgt library. This library is free         *
 * software; you can redistribute it and/or modify it under the terms *
 * of the GNU Lesser General Public License version 2.1 as published  *
 * by the Free Software Foundation.                                   *
 *                                                                    *
 * This library is distributed in the hope that it will be useful,    *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU Lesser General Public License for more details.                *
 *                                                                    *
 * You should have received a copy of the GNU Lesser General Public   *
 * License in the file "LICENSE.txt" along with this library.         *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 **********************************************************************/

#include "qtcanvas.h"

namespace tgt {

// shared context widget
QGLWidget* QtCanvas::shareWidget_ = 0;

QtCanvas::QtCanvas(const std::string& title,
                   const ivec2& size,
                   const Buffers buffers,
                   QWidget* parent, bool shared, Qt::WFlags f, char* /*name*/)
    : GLCanvas(title, size, buffers)
    , QGLWidget(getQGLFormat(buffers), parent, (shared ? shareWidget_ : 0), f)
{
    resize(size.x, size.y);
    if (shared && shareWidget_ == 0)
        shareWidget_ = this;

    setWindowTitle(QString(title.c_str()));

    setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // we have our own AutoBufferSwap-mechanism (GLCanvas::setAutoFlush), so disable the one of qt
    setAutoBufferSwap(false);

    rgbaSize_ = ivec4(format().redBufferSize(),
                      format().greenBufferSize(),
                      format().blueBufferSize(),
                      format().alphaBufferSize());
    stencilSize_ = format().stencilBufferSize();
    depthSize_ = format().depthBufferSize();
    doubleBuffered_ = doubleBuffer();
    stereoViewing_ = format().stereo();
}

QtCanvas::QtCanvas(QWidget* parent, bool shared, Qt::WFlags f, char* /*name*/)
    : GLCanvas()
    , QGLWidget(parent, (shared ? shareWidget_ : 0), f)
{
    if (shared && shareWidget_ == 0)
        shareWidget_ = this;

    // we have our own AutoBufferSwap-mechanism (GLCanvas::setAutoFlush), so disable the one of qt
    setAutoBufferSwap(false);

    rgbaSize_ = ivec4(format().redBufferSize(),
                      format().greenBufferSize(),
                      format().blueBufferSize(),
                      format().alphaBufferSize());
    stencilSize_ = format().stencilBufferSize();
    depthSize_ = format().depthBufferSize();
    doubleBuffered_ = doubleBuffer();
    stereoViewing_ = format().stereo();
}

QtCanvas::~QtCanvas() {}

void QtCanvas::init() {
    GLCanvas::init();
}

void QtCanvas::initializeGL() {
}

void QtCanvas::resizeGL(int w, int h) {
    sizeChanged(ivec2(w, h));
}

void QtCanvas::paintGL() {
    paint();
}

void QtCanvas::repaint() {
    updateGL();
}

void QtCanvas::update() {
    QWidget::update();
}

void QtCanvas::swap() {
    QGLWidget::swapBuffers();
}

void QtCanvas::toggleFullScreen() {
    if (fullscreen_) {
        fullscreen_ = false;
        showNormal();
    }
    else {
        showFullScreen();
        fullscreen_ = !fullscreen_;
    }
};

/*
    These functions are called by qt if there is a user-caused event, like the pressing of a
    mouse-button, as in this case. The press-event has to be turned into a tgt::MouseEvent and
    handled: Either by a custom eventloop, or by directly broadcasting it to all eventlisteners
    that are associated with this canvas. The same is true for the other event-handling
    methods.
*/
void QtCanvas::enterEvent(QEvent* /*e*/) {
    tgt::MouseEvent* enterEv = new tgt::MouseEvent(0, 0, tgt::MouseEvent::ENTER,
        tgt::MouseEvent::MODIFIER_NONE, tgt::MouseEvent::MOUSE_BUTTON_NONE, tgt::ivec2(width(), height()));
    eventHandler_->broadcastEvent(enterEv);
}

void QtCanvas::leaveEvent(QEvent* /*e*/) {
    tgt::MouseEvent* leaveEv = new tgt::MouseEvent(0, 0, tgt::MouseEvent::EXIT,
        tgt::MouseEvent::MODIFIER_NONE, tgt::MouseEvent::MOUSE_BUTTON_NONE, tgt::ivec2(width(), height()));
    eventHandler_->broadcastEvent(leaveEv);
}

void QtCanvas::mousePressEvent(QMouseEvent* e) {
    tgt::MouseEvent* prEv = new tgt::MouseEvent(e->x(), e->y(), tgt::MouseEvent::PRESSED,
        getModifier(e), getButton(e), tgt::ivec2(width(), height()));
    eventHandler_->broadcastEvent(prEv);
}

// See mousePressEvent
void QtCanvas::mouseReleaseEvent (QMouseEvent* e) {
    tgt::MouseEvent* relEv = new tgt::MouseEvent(e->x(), e->y(), tgt::MouseEvent::RELEASED,
        getModifier(e), getButton(e), tgt::ivec2(width(), height()));
    eventHandler_->broadcastEvent(relEv);
}

// See mousePressEvent
void QtCanvas::mouseMoveEvent(QMouseEvent*  e) {
    tgt::MouseEvent* movEv = new tgt::MouseEvent(e->x(), e->y(), tgt::MouseEvent::MOTION,
        getModifier(e), getButtons(e), tgt::ivec2(width(), height())); // FIXME: submit information which mouse buttons are pressed
    eventHandler_->broadcastEvent(movEv);
}

// See mousePressEvent
void QtCanvas::mouseDoubleClickEvent(QMouseEvent* e) {
    tgt::MouseEvent* dcEv = new tgt::MouseEvent(e->x(), e->y(), tgt::MouseEvent::DOUBLECLICK,
                                                getModifier(e), getButton(e), tgt::ivec2(width(), height()));
    eventHandler_->broadcastEvent(dcEv);
}

// See mousePressEvent
// does not distinguish correctly between UP and DOWN => works fine in voreen!
void QtCanvas::wheelEvent(QWheelEvent* e) {
    tgt::MouseEvent::MouseButtons b = tgt::MouseEvent::MOUSE_WHEEL_DOWN;
    if (e->delta() > 0)
        b = tgt::MouseEvent::MOUSE_WHEEL_UP;
    tgt::MouseEvent* wheelEv = new tgt::MouseEvent(e->x(),e->y(), tgt::MouseEvent::WHEEL,
                                                   getModifier(e), b, tgt::ivec2(width(), height()));
    eventHandler_->broadcastEvent(wheelEv);
}

// See mousePressEvent
void QtCanvas::keyPressEvent(QKeyEvent* event) {
    tgt::KeyEvent* ke = new tgt::KeyEvent(getKey(event->key()), getModifier(event), true);
    eventHandler_->broadcastEvent(ke);
}

// See mousePressEvent
void QtCanvas::keyReleaseEvent(QKeyEvent* event) {
    tgt::KeyEvent* ke = new tgt::KeyEvent(getKey(event->key()), getModifier(event), false);
    eventHandler_->broadcastEvent(ke);
}

// yes, we need this in voreen FL
void QtCanvas::timerEvent(QTimerEvent* /*e*/) {
    tgt::TimeEvent* te = new tgt::TimeEvent();
    eventHandler_->broadcastEvent(te);
}

tgt::MouseEvent::MouseButtons QtCanvas::getButton(QMouseEvent* e) {
    switch (e->button()) {
        case Qt::LeftButton:
            return tgt::MouseEvent::MOUSE_BUTTON_LEFT;
        case Qt::RightButton:
            return tgt::MouseEvent::MOUSE_BUTTON_RIGHT;
        case Qt::MidButton:
            return tgt::MouseEvent::MOUSE_BUTTON_MIDDLE;
        default:
            return tgt::MouseEvent::MOUSE_BUTTON_NONE;
    }
}

tgt::MouseEvent::MouseButtons QtCanvas::getButtons(QMouseEvent* e) {
    /*tgt::MouseEvent::MouseButtons*/ int buttons = tgt::MouseEvent::MOUSE_BUTTON_NONE;

    if (e->buttons() & Qt::LeftButton)
        buttons |= tgt::MouseEvent::MOUSE_BUTTON_LEFT;
    if (e->buttons() & Qt::RightButton)
        buttons |= tgt::MouseEvent::MOUSE_BUTTON_RIGHT;
    if (e->buttons() & Qt::MidButton)
        buttons |= tgt::MouseEvent::MOUSE_BUTTON_MIDDLE;

    return static_cast<tgt::MouseEvent::MouseButtons>(buttons);
}

tgt::Event::Modifier QtCanvas::getModifier(QInputEvent* e) {
    int result = 0;
    if (e->modifiers() & Qt::ControlModifier)
        result |= int(tgt::Event::CTRL);
    if (e->modifiers() & Qt::ShiftModifier)
        result |= int(tgt::Event::SHIFT);
    if (e->modifiers() & Qt::AltModifier)
        result |= int(tgt::Event::ALT);
    if (e->modifiers() & Qt::MetaModifier)
        result |= int(tgt::Event::META);
    if (e->modifiers() == Qt::NoModifier)
        result = int(tgt::Event::MODIFIER_NONE);
    return tgt::Event::Modifier(result);
}

QGLFormat QtCanvas::getQGLFormat(const Buffers buffers) {
    QGLFormat format = QGLFormat();
    format.setAlpha(buffers & GLCanvas::ALPHA_BUFFER);
    format.setDepth(buffers & GLCanvas::DEPTH_BUFFER);
    format.setDoubleBuffer(buffers & GLCanvas::DOUBLE_BUFFER);
    format.setStencil(buffers & GLCanvas::STENCIL_BUFFER);
    format.setAccum(buffers & GLCanvas::ACCUM_BUFFER);
    format.setStereo(buffers & GLCanvas::STEREO_VIEWING);
    format.setSampleBuffers(buffers & GLCanvas::MULTISAMPLING);

    return format;
}

KeyEvent::KeyCode QtCanvas::getKey(int key) {
    switch(key) {
        case Qt::Key_Escape   : return tgt::KeyEvent::K_ESCAPE;
        case Qt::Key_Tab  : return tgt::KeyEvent::K_TAB;
            // case Qt::Key_Backtab  : return tgt::KeyEvent::K_;
        case Qt::Key_Backspace    : return tgt::KeyEvent::K_BACKSPACE;
        case Qt::Key_Return   : return tgt::KeyEvent::K_RETURN;
        case Qt::Key_Enter    : return tgt::KeyEvent::K_KP_ENTER;
        case Qt::Key_Insert   : return tgt::KeyEvent::K_INSERT;
        case Qt::Key_Delete   : return tgt::KeyEvent::K_DELETE;
        case Qt::Key_Pause    : return tgt::KeyEvent::K_PAUSE;
        case Qt::Key_Print    : return tgt::KeyEvent::K_PRINT;
        case Qt::Key_SysReq   : return tgt::KeyEvent::K_SYSREQ;
        case Qt::Key_Clear    : return tgt::KeyEvent::K_CLEAR;
        case Qt::Key_Home     : return tgt::KeyEvent::K_HOME;
        case Qt::Key_End  : return tgt::KeyEvent::K_END;
        case Qt::Key_Left     : return tgt::KeyEvent::K_LEFT;
        case Qt::Key_Up   : return tgt::KeyEvent::K_UP;
        case Qt::Key_Right    : return tgt::KeyEvent::K_RIGHT;
        case Qt::Key_Down     : return tgt::KeyEvent::K_DOWN;
        case Qt::Key_PageUp   : return tgt::KeyEvent::K_PAGEUP;
        case Qt::Key_PageDown     : return tgt::KeyEvent::K_PAGEDOWN;
        case Qt::Key_Shift    : return tgt::KeyEvent::K_LSHIFT;  //TODO: qt only knows one shift, control, meta and alt
        case Qt::Key_Control : return tgt::KeyEvent::K_LCTRL;//...
        case Qt::Key_Meta : return tgt::KeyEvent::K_RMETA;//...
        case Qt::Key_Alt  : return tgt::KeyEvent::K_LALT;//...
        case Qt::Key_AltGr    : return tgt::KeyEvent::K_MODE;
        case Qt::Key_CapsLock     : return tgt::KeyEvent::K_CAPSLOCK;
        case Qt::Key_NumLock  : return tgt::KeyEvent::K_NUMLOCK;
        case Qt::Key_ScrollLock   : return tgt::KeyEvent::K_SCROLLOCK;
        case Qt::Key_F1   : return tgt::KeyEvent::K_F1;
        case Qt::Key_F2   : return tgt::KeyEvent::K_F2;
        case Qt::Key_F3   : return tgt::KeyEvent::K_F3;
        case Qt::Key_F4   : return tgt::KeyEvent::K_F4;
        case Qt::Key_F5   : return tgt::KeyEvent::K_F5;
        case Qt::Key_F6   : return tgt::KeyEvent::K_F6;
        case Qt::Key_F7   : return tgt::KeyEvent::K_F7;
        case Qt::Key_F8   : return tgt::KeyEvent::K_F8;
        case Qt::Key_F9   : return tgt::KeyEvent::K_F9;
        case Qt::Key_F10  : return tgt::KeyEvent::K_F10;
        case Qt::Key_F11  : return tgt::KeyEvent::K_F11;
        case Qt::Key_F12  : return tgt::KeyEvent::K_F12;
        case Qt::Key_F13  : return tgt::KeyEvent::K_F13;
        case Qt::Key_F14  : return tgt::KeyEvent::K_F14;
        case Qt::Key_F15  : return tgt::KeyEvent::K_F15;
            // case Qt::Key_F16  : return tgt::KeyEvent::K_;
            // case Qt::Key_F17  : return tgt::KeyEvent::K_;
            // case Qt::Key_F18  : return tgt::KeyEvent::K_;
            // case Qt::Key_F19  : return tgt::KeyEvent::K_;
            // case Qt::Key_F20  : return tgt::KeyEvent::K_;
            // case Qt::Key_F21  : return tgt::KeyEvent::K_;
            // case Qt::Key_F22  : return tgt::KeyEvent::K_;
            // case Qt::Key_F23  : return tgt::KeyEvent::K_;
            // case Qt::Key_F24  : return tgt::KeyEvent::K_;
            // case Qt::Key_F25  : return tgt::KeyEvent::K_;
            // case Qt::Key_F26  : return tgt::KeyEvent::K_;
            // case Qt::Key_F27  : return tgt::KeyEvent::K_;
            // case Qt::Key_F28  : return tgt::KeyEvent::K_;
            // case Qt::Key_F29  : return tgt::KeyEvent::K_;
            // case Qt::Key_F30  : return tgt::KeyEvent::K_;
            // case Qt::Key_F31  : return tgt::KeyEvent::K_;
            // case Qt::Key_F32  : return tgt::KeyEvent::K_;
            // case Qt::Key_F33  : return tgt::KeyEvent::K_;
            // case Qt::Key_F34  : return tgt::KeyEvent::K_;
            // case Qt::Key_F35  : return tgt::KeyEvent::K_;
        case Qt::Key_Super_L  : return tgt::KeyEvent::K_LSUPER;
        case Qt::Key_Super_R  : return tgt::KeyEvent::K_RSUPER;
        case Qt::Key_Menu     : return tgt::KeyEvent::K_MENU;
            // case Qt::Key_Hyper_L  : return tgt::KeyEvent::K_;
            // case Qt::Key_Hyper_R  : return tgt::KeyEvent::K_;
        case Qt::Key_Help     : return tgt::KeyEvent::K_HELP;
            // case Qt::Key_Direction_L  : return tgt::KeyEvent::K_;
            // case Qt::Key_Direction_R  : return tgt::KeyEvent::K_;
        case Qt::Key_Space    : return tgt::KeyEvent::K_SPACE;
        case Qt::Key_Exclam   : return tgt::KeyEvent::K_EXCLAIM;
            // case Qt::Key_QuoteDbl     : return tgt::KeyEvent::K_;
            // case Qt::Key_NumberSign   : return tgt::KeyEvent::K_;
        case Qt::Key_Dollar   : return tgt::KeyEvent::K_DOLLAR;
            // case Qt::Key_Percent  : return tgt::KeyEvent::K_;
        case Qt::Key_Ampersand    : return tgt::KeyEvent::K_AMPERSAND;
            // case Qt::Key_Apostrophe   : return tgt::KeyEvent::K_;
        case Qt::Key_ParenLeft    : return tgt::KeyEvent::K_LEFTPAREN;
        case Qt::Key_ParenRight   : return tgt::KeyEvent::K_RIGHTPAREN;
        case Qt::Key_Asterisk     : return tgt::KeyEvent::K_ASTERISK;
        case Qt::Key_Plus     : return tgt::KeyEvent::K_PLUS;
        case Qt::Key_Comma    : return tgt::KeyEvent::K_COMMA;
        case Qt::Key_Minus    : return tgt::KeyEvent::K_MINUS;
        case Qt::Key_Period   : return tgt::KeyEvent::K_PERIOD;
        case Qt::Key_Slash    : return tgt::KeyEvent::K_SLASH;
        case Qt::Key_0    : return tgt::KeyEvent::K_0;
        case Qt::Key_1    : return tgt::KeyEvent::K_1;
        case Qt::Key_2    : return tgt::KeyEvent::K_2;
        case Qt::Key_3    : return tgt::KeyEvent::K_3;
        case Qt::Key_4    : return tgt::KeyEvent::K_4;
        case Qt::Key_5    : return tgt::KeyEvent::K_5;
        case Qt::Key_6    : return tgt::KeyEvent::K_6;
        case Qt::Key_7    : return tgt::KeyEvent::K_7;
        case Qt::Key_8    : return tgt::KeyEvent::K_8;
        case Qt::Key_9    : return tgt::KeyEvent::K_9;
        case Qt::Key_Colon    : return tgt::KeyEvent::K_COLON;
        case Qt::Key_Semicolon    : return tgt::KeyEvent::K_SEMICOLON;
        case Qt::Key_Less     : return tgt::KeyEvent::K_LESS;
        case Qt::Key_Equal    : return tgt::KeyEvent::K_EQUALS;
        case Qt::Key_Greater  : return tgt::KeyEvent::K_GREATER;
        case Qt::Key_Question     : return tgt::KeyEvent::K_QUESTION;
        case Qt::Key_At   : return tgt::KeyEvent::K_AT;
        case Qt::Key_A    : return tgt::KeyEvent::K_A;
        case Qt::Key_B    : return tgt::KeyEvent::K_B;
        case Qt::Key_C    : return tgt::KeyEvent::K_C;
        case Qt::Key_D    : return tgt::KeyEvent::K_D;
        case Qt::Key_E    : return tgt::KeyEvent::K_E;
        case Qt::Key_F    : return tgt::KeyEvent::K_F;
        case Qt::Key_G    : return tgt::KeyEvent::K_G;
        case Qt::Key_H    : return tgt::KeyEvent::K_H;
        case Qt::Key_I    : return tgt::KeyEvent::K_I;
        case Qt::Key_J    : return tgt::KeyEvent::K_J;
        case Qt::Key_K    : return tgt::KeyEvent::K_K;
        case Qt::Key_L    : return tgt::KeyEvent::K_L;
        case Qt::Key_M    : return tgt::KeyEvent::K_M;
        case Qt::Key_N    : return tgt::KeyEvent::K_N;
        case Qt::Key_O    : return tgt::KeyEvent::K_O;
        case Qt::Key_P    : return tgt::KeyEvent::K_P;
        case Qt::Key_Q    : return tgt::KeyEvent::K_Q;
        case Qt::Key_R    : return tgt::KeyEvent::K_R;
        case Qt::Key_S    : return tgt::KeyEvent::K_S;
        case Qt::Key_T    : return tgt::KeyEvent::K_T;
        case Qt::Key_U    : return tgt::KeyEvent::K_U;
        case Qt::Key_V    : return tgt::KeyEvent::K_V;
        case Qt::Key_W    : return tgt::KeyEvent::K_W;
        case Qt::Key_X    : return tgt::KeyEvent::K_X;
        case Qt::Key_Y    : return tgt::KeyEvent::K_Y;
        case Qt::Key_Z    : return tgt::KeyEvent::K_Z;

        case Qt::Key_BracketLeft  : return tgt::KeyEvent::K_LEFTBRACKET;
        case Qt::Key_Backslash    : return tgt::KeyEvent::K_BACKSLASH;
        case Qt::Key_BracketRight     : return tgt::KeyEvent::K_RIGHTBRACKET;
            // case Qt::Key_AsciiCircum  : return tgt::KeyEvent::K_;
        case Qt::Key_Underscore   : return tgt::KeyEvent::K_UNDERSCORE;
            // case Qt::Key_QuoteLeft    : return tgt::KeyEvent::K_;
            // case Qt::Key_BraceLeft    : return tgt::KeyEvent::K_;
            // case Qt::Key_Bar  : return tgt::KeyEvent::K_;
            // case Qt::Key_BraceRight   : return tgt::KeyEvent::K_;
            // case Qt::Key_AsciiTilde   : return tgt::KeyEvent::K_;
            // case Qt::Key_nobreakspace     : return tgt::KeyEvent::K_;
            // case Qt::Key_exclamdown   : return tgt::KeyEvent::K_;
            // case Qt::Key_cent     : return tgt::KeyEvent::K_;
            // case Qt::Key_sterling     : return tgt::KeyEvent::K_;
            // case Qt::Key_currency     : return tgt::KeyEvent::K_;
            // case Qt::Key_yen  : return tgt::KeyEvent::K_;
            // case Qt::Key_brokenbar    : return tgt::KeyEvent::K_;
            // case Qt::Key_section  : return tgt::KeyEvent::K_;
            // case Qt::Key_diaeresis    : return tgt::KeyEvent::K_;
            // case Qt::Key_copyright    : return tgt::KeyEvent::K_;
            // case Qt::Key_ordfeminine  : return tgt::KeyEvent::K_;
            // case Qt::Key_guillemotleft    : return tgt::KeyEvent::K_;
            // case Qt::Key_notsign  : return tgt::KeyEvent::K_;
            // case Qt::Key_hyphen   : return tgt::KeyEvent::K_;
            // case Qt::Key_registered   : return tgt::KeyEvent::K_;
            // case Qt::Key_macron   : return tgt::KeyEvent::K_;
            // case Qt::Key_degree   : return tgt::KeyEvent::K_;
            // case Qt::Key_plusminus    : return tgt::KeyEvent::K_;
            // case Qt::Key_twosuperior  : return tgt::KeyEvent::K_;
            // case Qt::Key_threesuperior    : return tgt::KeyEvent::K_;
            // case Qt::Key_acute    : return tgt::KeyEvent::K_;
            // case Qt::Key_mu   : return tgt::KeyEvent::K_;
            // case Qt::Key_paragraph    : return tgt::KeyEvent::K_;
            // case Qt::Key_periodcentered   : return tgt::KeyEvent::K_;
            // case Qt::Key_cedilla  : return tgt::KeyEvent::K_;
            // case Qt::Key_onesuperior  : return tgt::KeyEvent::K_;
            // case Qt::Key_masculine    : return tgt::KeyEvent::K_;
            // case Qt::Key_guillemotright   : return tgt::KeyEvent::K_;
            // case Qt::Key_onequarter   : return tgt::KeyEvent::K_;
            // case Qt::Key_onehalf  : return tgt::KeyEvent::K_;
            // case Qt::Key_threequarters    : return tgt::KeyEvent::K_;
            // case Qt::Key_questiondown     : return tgt::KeyEvent::K_;
            // case Qt::Key_Agrave   : return tgt::KeyEvent::K_;
            // case Qt::Key_Aacute   : return tgt::KeyEvent::K_;
            // case Qt::Key_Acircumflex  : return tgt::KeyEvent::K_;
            // case Qt::Key_Atilde   : return tgt::KeyEvent::K_;
            // case Qt::Key_Adiaeresis   : return tgt::KeyEvent::K_;
            // case Qt::Key_Aring    : return tgt::KeyEvent::K_;
            // case Qt::Key_AE   : return tgt::KeyEvent::K_;
            // case Qt::Key_Ccedilla     : return tgt::KeyEvent::K_;
            // case Qt::Key_Egrave   : return tgt::KeyEvent::K_;
            // case Qt::Key_Eacute   : return tgt::KeyEvent::K_;
            // case Qt::Key_Ecircumflex  : return tgt::KeyEvent::K_;
            // case Qt::Key_Ediaeresis   : return tgt::KeyEvent::K_;
            // case Qt::Key_Igrave   : return tgt::KeyEvent::K_;
            // case Qt::Key_Iacute   : return tgt::KeyEvent::K_;
            // case Qt::Key_Icircumflex  : return tgt::KeyEvent::K_;
            // case Qt::Key_Idiaeresis   : return tgt::KeyEvent::K_;
            // case Qt::Key_ETH  : return tgt::KeyEvent::K_;
            // case Qt::Key_Ntilde   : return tgt::KeyEvent::K_;
            // case Qt::Key_Ograve   : return tgt::KeyEvent::K_;
            // case Qt::Key_Oacute   : return tgt::KeyEvent::K_;
            // case Qt::Key_Ocircumflex  : return tgt::KeyEvent::K_;
            // case Qt::Key_Otilde   : return tgt::KeyEvent::K_;
            // case Qt::Key_Odiaeresis   : return tgt::KeyEvent::K_;
            // case Qt::Key_multiply     : return tgt::KeyEvent::K_;
            // case Qt::Key_Ooblique     : return tgt::KeyEvent::K_;
            // case Qt::Key_Ugrave   : return tgt::KeyEvent::K_;
            // case Qt::Key_Uacute   : return tgt::KeyEvent::K_;
            // case Qt::Key_Ucircumflex  : return tgt::KeyEvent::K_;
            // case Qt::Key_Udiaeresis   : return tgt::KeyEvent::K_;
            // case Qt::Key_Yacute   : return tgt::KeyEvent::K_;
            // case Qt::Key_THORN    : return tgt::KeyEvent::K_;
            // case Qt::Key_ssharp   : return tgt::KeyEvent::K_;
            // case Qt::Key_division     : return tgt::KeyEvent::K_;
            // case Qt::Key_ydiaeresis   : return tgt::KeyEvent::K_;
            // case Qt::Key_Multi_key    : return tgt::KeyEvent::K_;
            // case Qt::Key_Codeinput    : return tgt::KeyEvent::K_;
            // case Qt::Key_SingleCandidate  : return tgt::KeyEvent::K_;
            // case Qt::Key_MultipleCandidate    : return tgt::KeyEvent::K_;
            // case Qt::Key_PreviousCandidate    : return tgt::KeyEvent::K_;
            // case Qt::Key_Mode_switch  : return tgt::KeyEvent::K_;
            // case Qt::Key_Kanji    : return tgt::KeyEvent::K_;
            // case Qt::Key_Muhenkan     : return tgt::KeyEvent::K_;
            // case Qt::Key_Henkan   : return tgt::KeyEvent::K_;
            // case Qt::Key_Romaji   : return tgt::KeyEvent::K_;
            // case Qt::Key_Hiragana     : return tgt::KeyEvent::K_;
            // case Qt::Key_Katakana     : return tgt::KeyEvent::K_;
            // case Qt::Key_Hiragana_Katakana    : return tgt::KeyEvent::K_;
            // case Qt::Key_Zenkaku  : return tgt::KeyEvent::K_;
            // case Qt::Key_Hankaku  : return tgt::KeyEvent::K_;
            // case Qt::Key_Zenkaku_Hankaku  : return tgt::KeyEvent::K_;
            // case Qt::Key_Touroku  : return tgt::KeyEvent::K_;
            // case Qt::Key_Massyo   : return tgt::KeyEvent::K_;
            // case Qt::Key_Kana_Lock    : return tgt::KeyEvent::K_;
            // case Qt::Key_Kana_Shift   : return tgt::KeyEvent::K_;
            // case Qt::Key_Eisu_Shift   : return tgt::KeyEvent::K_;
            // case Qt::Key_Eisu_toggle  : return tgt::KeyEvent::K_;
            // case Qt::Key_Hangul   : return tgt::KeyEvent::K_;
            // case Qt::Key_Hangul_Start     : return tgt::KeyEvent::K_;
            // case Qt::Key_Hangul_End   : return tgt::KeyEvent::K_;
            // case Qt::Key_Hangul_Hanja     : return tgt::KeyEvent::K_;
            // case Qt::Key_Hangul_Jamo  : return tgt::KeyEvent::K_;
            // case Qt::Key_Hangul_Romaja    : return tgt::KeyEvent::K_;
            // case Qt::Key_Hangul_Jeonja    : return tgt::KeyEvent::K_;
            // case Qt::Key_Hangul_Banja     : return tgt::KeyEvent::K_;
            // case Qt::Key_Hangul_PreHanja  : return tgt::KeyEvent::K_;
            // case Qt::Key_Hangul_PostHanja     : return tgt::KeyEvent::K_;
            // case Qt::Key_Hangul_Special   : return tgt::KeyEvent::K_;
            // case Qt::Key_Dead_Grave   : return tgt::KeyEvent::K_;
            // case Qt::Key_Dead_Acute   : return tgt::KeyEvent::K_;
            // case Qt::Key_Dead_Circumflex  : return tgt::KeyEvent::K_;
            // case Qt::Key_Dead_Tilde   : return tgt::KeyEvent::K_;
            // case Qt::Key_Dead_Macron  : return tgt::KeyEvent::K_;
            // case Qt::Key_Dead_Breve   : return tgt::KeyEvent::K_;
            // case Qt::Key_Dead_Abovedot    : return tgt::KeyEvent::K_;
            // case Qt::Key_Dead_Diaeresis   : return tgt::KeyEvent::K_;
            // case Qt::Key_Dead_Abovering   : return tgt::KeyEvent::K_;
            // case Qt::Key_Dead_Doubleacute     : return tgt::KeyEvent::K_;
            // case Qt::Key_Dead_Caron   : return tgt::KeyEvent::K_;
            // case Qt::Key_Dead_Cedilla     : return tgt::KeyEvent::K_;
            // case Qt::Key_Dead_Ogonek  : return tgt::KeyEvent::K_;
            // case Qt::Key_Dead_Iota    : return tgt::KeyEvent::K_;
            // case Qt::Key_Dead_Voiced_Sound    : return tgt::KeyEvent::K_;
            // case Qt::Key_Dead_Semivoiced_Sound    : return tgt::KeyEvent::K_;
            // case Qt::Key_Dead_Belowdot    : return tgt::KeyEvent::K_;
            // case Qt::Key_Dead_Hook    : return tgt::KeyEvent::K_;
            // case Qt::Key_Dead_Horn    : return tgt::KeyEvent::K_;
            // case Qt::Key_Back     : return tgt::KeyEvent::K_;
            // case Qt::Key_Forward  : return tgt::KeyEvent::K_;
            // case Qt::Key_Stop     : return tgt::KeyEvent::K_;
            // case Qt::Key_Refresh  : return tgt::KeyEvent::K_;
            // case Qt::Key_VolumeDown   : return tgt::KeyEvent::K_;
            // case Qt::Key_VolumeMute   : return tgt::KeyEvent::K_;
            // case Qt::Key_VolumeUp     : return tgt::KeyEvent::K_;
            // case Qt::Key_BassBoost    : return tgt::KeyEvent::K_;
            // case Qt::Key_BassUp   : return tgt::KeyEvent::K_;
            // case Qt::Key_BassDown     : return tgt::KeyEvent::K_;
            // case Qt::Key_TrebleUp     : return tgt::KeyEvent::K_;
            // case Qt::Key_TrebleDown   : return tgt::KeyEvent::K_;
            // case Qt::Key_MediaPlay    : return tgt::KeyEvent::K_;
            // case Qt::Key_MediaStop    : return tgt::KeyEvent::K_;
            // case Qt::Key_MediaPrevious    : return tgt::KeyEvent::K_;
            // case Qt::Key_MediaNext    : return tgt::KeyEvent::K_;
            // case Qt::Key_MediaRecord  : return tgt::KeyEvent::K_;
            // case Qt::Key_HomePage     : return tgt::KeyEvent::K_;
            // case Qt::Key_Favorites    : return tgt::KeyEvent::K_;
            // case Qt::Key_Search   : return tgt::KeyEvent::K_;
            // case Qt::Key_Standby  : return tgt::KeyEvent::K_;
            // case Qt::Key_OpenUrl  : return tgt::KeyEvent::K_;
            // case Qt::Key_LaunchMail   : return tgt::KeyEvent::K_;
            // case Qt::Key_LaunchMedia  : return tgt::KeyEvent::K_;
            // case Qt::Key_Launch0  : return tgt::KeyEvent::K_;
            // case Qt::Key_Launch1  : return tgt::KeyEvent::K_;
            // case Qt::Key_Launch2  : return tgt::KeyEvent::K_;
            // case Qt::Key_Launch3  : return tgt::KeyEvent::K_;
            // case Qt::Key_Launch4  : return tgt::KeyEvent::K_;
            // case Qt::Key_Launch5  : return tgt::KeyEvent::K_;
            // case Qt::Key_Launch6  : return tgt::KeyEvent::K_;
            // case Qt::Key_Launch7  : return tgt::KeyEvent::K_;
            // case Qt::Key_Launch8  : return tgt::KeyEvent::K_;
            // case Qt::Key_Launch9  : return tgt::KeyEvent::K_;
            // case Qt::Key_LaunchA  : return tgt::KeyEvent::K_;
            // case Qt::Key_LaunchB  : return tgt::KeyEvent::K_;
            // case Qt::Key_LaunchC  : return tgt::KeyEvent::K_;
            // case Qt::Key_LaunchD  : return tgt::KeyEvent::K_;
            // case Qt::Key_LaunchE  : return tgt::KeyEvent::K_;
            // case Qt::Key_LaunchF  : return tgt::KeyEvent::K_;
            // case Qt::Key_MediaLast    : return tgt::KeyEvent::K_;
            // case Qt::Key_unknown  : return tgt::KeyEvent::K_;
            // case Qt::Key_Call     : return tgt::KeyEvent::K_;
            // case Qt::Key_Context1     : return tgt::KeyEvent::K_;
            // case Qt::Key_Context2     : return tgt::KeyEvent::K_;
            // case Qt::Key_Context3     : return tgt::KeyEvent::K_;
            // case Qt::Key_Context4     : return tgt::KeyEvent::K_;
            // case Qt::Key_Flip     : return tgt::KeyEvent::K_;
            // case Qt::Key_Hangup   : return tgt::KeyEvent::K_;
            // case Qt::Key_No   : return tgt::KeyEvent::K_;
            // case Qt::Key_Select   : return tgt::KeyEvent::K_;
            // case Qt::Key_Yes : return tgt::KeyEvent::K_;
        default: return tgt::KeyEvent::K_UNKNOWN;
    }
}

void QtCanvas::setSize(ivec2 newSize) {
    QWidget::resize(newSize.x, newSize.y);
}

QSize QtCanvas::sizeHint() const {
    const ivec2& size = getSize();

    return QSize(size.x, size.y);
}

void QtCanvas::acquireAsCurrentContext() {
    this->makeCurrent();
}

void QtCanvas::releaseAsCurrentContext() {
    this->doneCurrent();
}


} // namespace
