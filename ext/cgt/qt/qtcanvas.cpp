/**********************************************************************
 *                                                                    *
 * cgt - CAMP Graphics Toolbox, Copyright (C) 2012-2015               *
 *     Chair for Computer Aided Medical Procedures                    *
 *     Technische Universitaet Muenchen, Germany.                     *
 *     <http://campar.in.tum.de/>                                     *
 *                                                                    *
 * forked from tgt - Tiny Graphics Toolbox, Copyright (C) 2006-2011   *
 *     Visualization and Computer Graphics Group, Department of       *
 *     Computer Science, University of Muenster, Germany.             *
 *     <http://viscg.uni-muenster.de>                                 *
 *                                                                    *
 * This file is part of the cgt library. This library is free         *
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

#include <QThread>

namespace cgt {

// shared context widget
QGLWidget* QtCanvas::shareWidget_ = 0;

QtCanvas::QtCanvas(const std::string& title,
                   const ivec2& size,
                   const Buffers buffers,
                   QWidget* parent, bool shared, Qt::WindowFlags f, char* /*name*/)
    : QGLWidget(getQGLFormat(buffers), 0, (shared ? shareWidget_ : 0), f)
    , GLCanvas(title, size, buffers)
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

    connect(this, SIGNAL(s_sizeChangedExternally(int, int)), this, SLOT(sizeChangedExternally(int, int)));
}

QtCanvas::QtCanvas(QWidget* parent, bool shared, Qt::WindowFlags f, char* /*name*/)
    : QGLWidget(0, (shared ? shareWidget_ : 0), f)
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

void QtCanvas::moveThreadAffinity(void* threadPointer) {
    QThread* qThreadPointer = static_cast<QThread*>(threadPointer);
    this->context()->moveToThread(qThreadPointer);
}

void* QtCanvas::getCurrentThreadPointer() {
    return QThread::currentThread();
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
    QGLWidget::makeCurrent();
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
    mouse-button, as in this case. The press-event has to be turned into a cgt::MouseEvent and
    handled: Either by a custom eventloop, or by directly broadcasting it to all eventlisteners
    that are associated with this canvas. The same is true for the other event-handling
    methods.
*/
void QtCanvas::enterEvent(QEvent* /*e*/) {
    cgt::MouseEvent* enterEv = new cgt::MouseEvent(0, 0, cgt::MouseEvent::ENTER,
        cgt::MouseEvent::MODIFIER_NONE, cgt::MouseEvent::MOUSE_BUTTON_NONE, cgt::ivec2(width(), height()));
    eventHandler_->broadcastEvent(enterEv);
}

void QtCanvas::leaveEvent(QEvent* /*e*/) {
    cgt::MouseEvent* leaveEv = new cgt::MouseEvent(0, 0, cgt::MouseEvent::EXIT,
        cgt::MouseEvent::MODIFIER_NONE, cgt::MouseEvent::MOUSE_BUTTON_NONE, cgt::ivec2(width(), height()));
    eventHandler_->broadcastEvent(leaveEv);
}

void QtCanvas::mousePressEvent(QMouseEvent* e) {
    cgt::MouseEvent* prEv = new cgt::MouseEvent(e->x(), e->y(), cgt::MouseEvent::PRESSED,
        getModifier(e), getButton(e), cgt::ivec2(width(), height()));
    eventHandler_->broadcastEvent(prEv);
}

// See mousePressEvent
void QtCanvas::mouseReleaseEvent (QMouseEvent* e) {
    cgt::MouseEvent* relEv = new cgt::MouseEvent(e->x(), e->y(), cgt::MouseEvent::RELEASED,
        getModifier(e), getButton(e), cgt::ivec2(width(), height()));
    eventHandler_->broadcastEvent(relEv);
}

// See mousePressEvent
void QtCanvas::mouseMoveEvent(QMouseEvent*  e) {
    cgt::MouseEvent* movEv = new cgt::MouseEvent(e->x(), e->y(), cgt::MouseEvent::MOTION,
        getModifier(e), getButtons(e), cgt::ivec2(width(), height())); // FIXME: submit information which mouse buttons are pressed
    eventHandler_->broadcastEvent(movEv);
}

// See mousePressEvent
void QtCanvas::mouseDoubleClickEvent(QMouseEvent* e) {
    cgt::MouseEvent* dcEv = new cgt::MouseEvent(e->x(), e->y(), cgt::MouseEvent::DOUBLECLICK,
                                                getModifier(e), getButton(e), cgt::ivec2(width(), height()));
    eventHandler_->broadcastEvent(dcEv);
}

// See mousePressEvent
// does not distinguish correctly between UP and DOWN => works fine in voreen!
void QtCanvas::wheelEvent(QWheelEvent* e) {
    cgt::MouseEvent::MouseButtons b = cgt::MouseEvent::MOUSE_WHEEL_DOWN;
    if (e->delta() > 0)
        b = cgt::MouseEvent::MOUSE_WHEEL_UP;
    cgt::MouseEvent* wheelEv = new cgt::MouseEvent(e->x(),e->y(), cgt::MouseEvent::WHEEL,
                                                   getModifier(e), b, cgt::ivec2(width(), height()));
    eventHandler_->broadcastEvent(wheelEv);
}

// See mousePressEvent
void QtCanvas::keyPressEvent(QKeyEvent* event) {
    cgt::KeyEvent* ke = new cgt::KeyEvent(getKey(event->key()), getModifier(event), true);
    eventHandler_->broadcastEvent(ke);
}

// See mousePressEvent
void QtCanvas::keyReleaseEvent(QKeyEvent* event) {
    cgt::KeyEvent* ke = new cgt::KeyEvent(getKey(event->key()), getModifier(event), false);
    eventHandler_->broadcastEvent(ke);
}

// yes, we need this in voreen FL
void QtCanvas::timerEvent(QTimerEvent* /*e*/) {
    cgt::TimeEvent* te = new cgt::TimeEvent();
    eventHandler_->broadcastEvent(te);
}

cgt::MouseEvent::MouseButtons QtCanvas::getButton(QMouseEvent* e) {
    switch (e->button()) {
        case Qt::LeftButton:
            return cgt::MouseEvent::MOUSE_BUTTON_LEFT;
        case Qt::RightButton:
            return cgt::MouseEvent::MOUSE_BUTTON_RIGHT;
        case Qt::MidButton:
            return cgt::MouseEvent::MOUSE_BUTTON_MIDDLE;
        default:
            return cgt::MouseEvent::MOUSE_BUTTON_NONE;
    }
}

cgt::MouseEvent::MouseButtons QtCanvas::getButtons(QMouseEvent* e) {
    /*cgt::MouseEvent::MouseButtons*/ int buttons = cgt::MouseEvent::MOUSE_BUTTON_NONE;

    if (e->buttons() & Qt::LeftButton)
        buttons |= cgt::MouseEvent::MOUSE_BUTTON_LEFT;
    if (e->buttons() & Qt::RightButton)
        buttons |= cgt::MouseEvent::MOUSE_BUTTON_RIGHT;
    if (e->buttons() & Qt::MidButton)
        buttons |= cgt::MouseEvent::MOUSE_BUTTON_MIDDLE;

    return static_cast<cgt::MouseEvent::MouseButtons>(buttons);
}

cgt::Event::Modifier QtCanvas::getModifier(QInputEvent* e) {
    int result = 0;
    if (e->modifiers() & Qt::ControlModifier)
        result |= int(cgt::Event::CTRL);
    if (e->modifiers() & Qt::ShiftModifier)
        result |= int(cgt::Event::SHIFT);
    if (e->modifiers() & Qt::AltModifier)
        result |= int(cgt::Event::ALT);
    if (e->modifiers() & Qt::MetaModifier)
        result |= int(cgt::Event::META);
    if (e->modifiers() == Qt::NoModifier)
        result = int(cgt::Event::MODIFIER_NONE);
    return cgt::Event::Modifier(result);
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
        case Qt::Key_Escape   : return cgt::KeyEvent::K_ESCAPE;
        case Qt::Key_Tab  : return cgt::KeyEvent::K_TAB;
            // case Qt::Key_Backtab  : return cgt::KeyEvent::K_;
        case Qt::Key_Backspace    : return cgt::KeyEvent::K_BACKSPACE;
        case Qt::Key_Return   : return cgt::KeyEvent::K_RETURN;
        case Qt::Key_Enter    : return cgt::KeyEvent::K_KP_ENTER;
        case Qt::Key_Insert   : return cgt::KeyEvent::K_INSERT;
        case Qt::Key_Delete   : return cgt::KeyEvent::K_DELETE;
        case Qt::Key_Pause    : return cgt::KeyEvent::K_PAUSE;
        case Qt::Key_Print    : return cgt::KeyEvent::K_PRINT;
        case Qt::Key_SysReq   : return cgt::KeyEvent::K_SYSREQ;
        case Qt::Key_Clear    : return cgt::KeyEvent::K_CLEAR;
        case Qt::Key_Home     : return cgt::KeyEvent::K_HOME;
        case Qt::Key_End  : return cgt::KeyEvent::K_END;
        case Qt::Key_Left     : return cgt::KeyEvent::K_LEFT;
        case Qt::Key_Up   : return cgt::KeyEvent::K_UP;
        case Qt::Key_Right    : return cgt::KeyEvent::K_RIGHT;
        case Qt::Key_Down     : return cgt::KeyEvent::K_DOWN;
        case Qt::Key_PageUp   : return cgt::KeyEvent::K_PAGEUP;
        case Qt::Key_PageDown     : return cgt::KeyEvent::K_PAGEDOWN;
        case Qt::Key_Shift    : return cgt::KeyEvent::K_LSHIFT;  // Qt only knows one shift, control, meta and alt
        case Qt::Key_Control : return cgt::KeyEvent::K_LCTRL;//...
        case Qt::Key_Meta : return cgt::KeyEvent::K_RMETA;//...
        case Qt::Key_Alt  : return cgt::KeyEvent::K_LALT;//...
        case Qt::Key_AltGr    : return cgt::KeyEvent::K_MODE;
        case Qt::Key_CapsLock     : return cgt::KeyEvent::K_CAPSLOCK;
        case Qt::Key_NumLock  : return cgt::KeyEvent::K_NUMLOCK;
        case Qt::Key_ScrollLock   : return cgt::KeyEvent::K_SCROLLOCK;
        case Qt::Key_F1   : return cgt::KeyEvent::K_F1;
        case Qt::Key_F2   : return cgt::KeyEvent::K_F2;
        case Qt::Key_F3   : return cgt::KeyEvent::K_F3;
        case Qt::Key_F4   : return cgt::KeyEvent::K_F4;
        case Qt::Key_F5   : return cgt::KeyEvent::K_F5;
        case Qt::Key_F6   : return cgt::KeyEvent::K_F6;
        case Qt::Key_F7   : return cgt::KeyEvent::K_F7;
        case Qt::Key_F8   : return cgt::KeyEvent::K_F8;
        case Qt::Key_F9   : return cgt::KeyEvent::K_F9;
        case Qt::Key_F10  : return cgt::KeyEvent::K_F10;
        case Qt::Key_F11  : return cgt::KeyEvent::K_F11;
        case Qt::Key_F12  : return cgt::KeyEvent::K_F12;
        case Qt::Key_F13  : return cgt::KeyEvent::K_F13;
        case Qt::Key_F14  : return cgt::KeyEvent::K_F14;
        case Qt::Key_F15  : return cgt::KeyEvent::K_F15;
            // case Qt::Key_F16  : return cgt::KeyEvent::K_;
            // case Qt::Key_F17  : return cgt::KeyEvent::K_;
            // case Qt::Key_F18  : return cgt::KeyEvent::K_;
            // case Qt::Key_F19  : return cgt::KeyEvent::K_;
            // case Qt::Key_F20  : return cgt::KeyEvent::K_;
            // case Qt::Key_F21  : return cgt::KeyEvent::K_;
            // case Qt::Key_F22  : return cgt::KeyEvent::K_;
            // case Qt::Key_F23  : return cgt::KeyEvent::K_;
            // case Qt::Key_F24  : return cgt::KeyEvent::K_;
            // case Qt::Key_F25  : return cgt::KeyEvent::K_;
            // case Qt::Key_F26  : return cgt::KeyEvent::K_;
            // case Qt::Key_F27  : return cgt::KeyEvent::K_;
            // case Qt::Key_F28  : return cgt::KeyEvent::K_;
            // case Qt::Key_F29  : return cgt::KeyEvent::K_;
            // case Qt::Key_F30  : return cgt::KeyEvent::K_;
            // case Qt::Key_F31  : return cgt::KeyEvent::K_;
            // case Qt::Key_F32  : return cgt::KeyEvent::K_;
            // case Qt::Key_F33  : return cgt::KeyEvent::K_;
            // case Qt::Key_F34  : return cgt::KeyEvent::K_;
            // case Qt::Key_F35  : return cgt::KeyEvent::K_;
        case Qt::Key_Super_L  : return cgt::KeyEvent::K_LSUPER;
        case Qt::Key_Super_R  : return cgt::KeyEvent::K_RSUPER;
        case Qt::Key_Menu     : return cgt::KeyEvent::K_MENU;
            // case Qt::Key_Hyper_L  : return cgt::KeyEvent::K_;
            // case Qt::Key_Hyper_R  : return cgt::KeyEvent::K_;
        case Qt::Key_Help     : return cgt::KeyEvent::K_HELP;
            // case Qt::Key_Direction_L  : return cgt::KeyEvent::K_;
            // case Qt::Key_Direction_R  : return cgt::KeyEvent::K_;
        case Qt::Key_Space    : return cgt::KeyEvent::K_SPACE;
        case Qt::Key_Exclam   : return cgt::KeyEvent::K_EXCLAIM;
            // case Qt::Key_QuoteDbl     : return cgt::KeyEvent::K_;
            // case Qt::Key_NumberSign   : return cgt::KeyEvent::K_;
        case Qt::Key_Dollar   : return cgt::KeyEvent::K_DOLLAR;
            // case Qt::Key_Percent  : return cgt::KeyEvent::K_;
        case Qt::Key_Ampersand    : return cgt::KeyEvent::K_AMPERSAND;
            // case Qt::Key_Apostrophe   : return cgt::KeyEvent::K_;
        case Qt::Key_ParenLeft    : return cgt::KeyEvent::K_LEFTPAREN;
        case Qt::Key_ParenRight   : return cgt::KeyEvent::K_RIGHTPAREN;
        case Qt::Key_Asterisk     : return cgt::KeyEvent::K_ASTERISK;
        case Qt::Key_Plus     : return cgt::KeyEvent::K_PLUS;
        case Qt::Key_Comma    : return cgt::KeyEvent::K_COMMA;
        case Qt::Key_Minus    : return cgt::KeyEvent::K_MINUS;
        case Qt::Key_Period   : return cgt::KeyEvent::K_PERIOD;
        case Qt::Key_Slash    : return cgt::KeyEvent::K_SLASH;
        case Qt::Key_0    : return cgt::KeyEvent::K_0;
        case Qt::Key_1    : return cgt::KeyEvent::K_1;
        case Qt::Key_2    : return cgt::KeyEvent::K_2;
        case Qt::Key_3    : return cgt::KeyEvent::K_3;
        case Qt::Key_4    : return cgt::KeyEvent::K_4;
        case Qt::Key_5    : return cgt::KeyEvent::K_5;
        case Qt::Key_6    : return cgt::KeyEvent::K_6;
        case Qt::Key_7    : return cgt::KeyEvent::K_7;
        case Qt::Key_8    : return cgt::KeyEvent::K_8;
        case Qt::Key_9    : return cgt::KeyEvent::K_9;
        case Qt::Key_Colon    : return cgt::KeyEvent::K_COLON;
        case Qt::Key_Semicolon    : return cgt::KeyEvent::K_SEMICOLON;
        case Qt::Key_Less     : return cgt::KeyEvent::K_LESS;
        case Qt::Key_Equal    : return cgt::KeyEvent::K_EQUALS;
        case Qt::Key_Greater  : return cgt::KeyEvent::K_GREATER;
        case Qt::Key_Question     : return cgt::KeyEvent::K_QUESTION;
        case Qt::Key_At   : return cgt::KeyEvent::K_AT;
        case Qt::Key_A    : return cgt::KeyEvent::K_A;
        case Qt::Key_B    : return cgt::KeyEvent::K_B;
        case Qt::Key_C    : return cgt::KeyEvent::K_C;
        case Qt::Key_D    : return cgt::KeyEvent::K_D;
        case Qt::Key_E    : return cgt::KeyEvent::K_E;
        case Qt::Key_F    : return cgt::KeyEvent::K_F;
        case Qt::Key_G    : return cgt::KeyEvent::K_G;
        case Qt::Key_H    : return cgt::KeyEvent::K_H;
        case Qt::Key_I    : return cgt::KeyEvent::K_I;
        case Qt::Key_J    : return cgt::KeyEvent::K_J;
        case Qt::Key_K    : return cgt::KeyEvent::K_K;
        case Qt::Key_L    : return cgt::KeyEvent::K_L;
        case Qt::Key_M    : return cgt::KeyEvent::K_M;
        case Qt::Key_N    : return cgt::KeyEvent::K_N;
        case Qt::Key_O    : return cgt::KeyEvent::K_O;
        case Qt::Key_P    : return cgt::KeyEvent::K_P;
        case Qt::Key_Q    : return cgt::KeyEvent::K_Q;
        case Qt::Key_R    : return cgt::KeyEvent::K_R;
        case Qt::Key_S    : return cgt::KeyEvent::K_S;
        case Qt::Key_T    : return cgt::KeyEvent::K_T;
        case Qt::Key_U    : return cgt::KeyEvent::K_U;
        case Qt::Key_V    : return cgt::KeyEvent::K_V;
        case Qt::Key_W    : return cgt::KeyEvent::K_W;
        case Qt::Key_X    : return cgt::KeyEvent::K_X;
        case Qt::Key_Y    : return cgt::KeyEvent::K_Y;
        case Qt::Key_Z    : return cgt::KeyEvent::K_Z;

        case Qt::Key_BracketLeft  : return cgt::KeyEvent::K_LEFTBRACKET;
        case Qt::Key_Backslash    : return cgt::KeyEvent::K_BACKSLASH;
        case Qt::Key_BracketRight     : return cgt::KeyEvent::K_RIGHTBRACKET;
            // case Qt::Key_AsciiCircum  : return cgt::KeyEvent::K_;
        case Qt::Key_Underscore   : return cgt::KeyEvent::K_UNDERSCORE;
            // case Qt::Key_QuoteLeft    : return cgt::KeyEvent::K_;
            // case Qt::Key_BraceLeft    : return cgt::KeyEvent::K_;
            // case Qt::Key_Bar  : return cgt::KeyEvent::K_;
            // case Qt::Key_BraceRight   : return cgt::KeyEvent::K_;
            // case Qt::Key_AsciiTilde   : return cgt::KeyEvent::K_;
            // case Qt::Key_nobreakspace     : return cgt::KeyEvent::K_;
            // case Qt::Key_exclamdown   : return cgt::KeyEvent::K_;
            // case Qt::Key_cent     : return cgt::KeyEvent::K_;
            // case Qt::Key_sterling     : return cgt::KeyEvent::K_;
            // case Qt::Key_currency     : return cgt::KeyEvent::K_;
            // case Qt::Key_yen  : return cgt::KeyEvent::K_;
            // case Qt::Key_brokenbar    : return cgt::KeyEvent::K_;
            // case Qt::Key_section  : return cgt::KeyEvent::K_;
            // case Qt::Key_diaeresis    : return cgt::KeyEvent::K_;
            // case Qt::Key_copyright    : return cgt::KeyEvent::K_;
            // case Qt::Key_ordfeminine  : return cgt::KeyEvent::K_;
            // case Qt::Key_guillemotleft    : return cgt::KeyEvent::K_;
            // case Qt::Key_notsign  : return cgt::KeyEvent::K_;
            // case Qt::Key_hyphen   : return cgt::KeyEvent::K_;
            // case Qt::Key_registered   : return cgt::KeyEvent::K_;
            // case Qt::Key_macron   : return cgt::KeyEvent::K_;
            // case Qt::Key_degree   : return cgt::KeyEvent::K_;
            // case Qt::Key_plusminus    : return cgt::KeyEvent::K_;
            // case Qt::Key_twosuperior  : return cgt::KeyEvent::K_;
            // case Qt::Key_threesuperior    : return cgt::KeyEvent::K_;
            // case Qt::Key_acute    : return cgt::KeyEvent::K_;
            // case Qt::Key_mu   : return cgt::KeyEvent::K_;
            // case Qt::Key_paragraph    : return cgt::KeyEvent::K_;
            // case Qt::Key_periodcentered   : return cgt::KeyEvent::K_;
            // case Qt::Key_cedilla  : return cgt::KeyEvent::K_;
            // case Qt::Key_onesuperior  : return cgt::KeyEvent::K_;
            // case Qt::Key_masculine    : return cgt::KeyEvent::K_;
            // case Qt::Key_guillemotright   : return cgt::KeyEvent::K_;
            // case Qt::Key_onequarter   : return cgt::KeyEvent::K_;
            // case Qt::Key_onehalf  : return cgt::KeyEvent::K_;
            // case Qt::Key_threequarters    : return cgt::KeyEvent::K_;
            // case Qt::Key_questiondown     : return cgt::KeyEvent::K_;
            // case Qt::Key_Agrave   : return cgt::KeyEvent::K_;
            // case Qt::Key_Aacute   : return cgt::KeyEvent::K_;
            // case Qt::Key_Acircumflex  : return cgt::KeyEvent::K_;
            // case Qt::Key_Atilde   : return cgt::KeyEvent::K_;
            // case Qt::Key_Adiaeresis   : return cgt::KeyEvent::K_;
            // case Qt::Key_Aring    : return cgt::KeyEvent::K_;
            // case Qt::Key_AE   : return cgt::KeyEvent::K_;
            // case Qt::Key_Ccedilla     : return cgt::KeyEvent::K_;
            // case Qt::Key_Egrave   : return cgt::KeyEvent::K_;
            // case Qt::Key_Eacute   : return cgt::KeyEvent::K_;
            // case Qt::Key_Ecircumflex  : return cgt::KeyEvent::K_;
            // case Qt::Key_Ediaeresis   : return cgt::KeyEvent::K_;
            // case Qt::Key_Igrave   : return cgt::KeyEvent::K_;
            // case Qt::Key_Iacute   : return cgt::KeyEvent::K_;
            // case Qt::Key_Icircumflex  : return cgt::KeyEvent::K_;
            // case Qt::Key_Idiaeresis   : return cgt::KeyEvent::K_;
            // case Qt::Key_ETH  : return cgt::KeyEvent::K_;
            // case Qt::Key_Ntilde   : return cgt::KeyEvent::K_;
            // case Qt::Key_Ograve   : return cgt::KeyEvent::K_;
            // case Qt::Key_Oacute   : return cgt::KeyEvent::K_;
            // case Qt::Key_Ocircumflex  : return cgt::KeyEvent::K_;
            // case Qt::Key_Otilde   : return cgt::KeyEvent::K_;
            // case Qt::Key_Odiaeresis   : return cgt::KeyEvent::K_;
            // case Qt::Key_multiply     : return cgt::KeyEvent::K_;
            // case Qt::Key_Ooblique     : return cgt::KeyEvent::K_;
            // case Qt::Key_Ugrave   : return cgt::KeyEvent::K_;
            // case Qt::Key_Uacute   : return cgt::KeyEvent::K_;
            // case Qt::Key_Ucircumflex  : return cgt::KeyEvent::K_;
            // case Qt::Key_Udiaeresis   : return cgt::KeyEvent::K_;
            // case Qt::Key_Yacute   : return cgt::KeyEvent::K_;
            // case Qt::Key_THORN    : return cgt::KeyEvent::K_;
            // case Qt::Key_ssharp   : return cgt::KeyEvent::K_;
            // case Qt::Key_division     : return cgt::KeyEvent::K_;
            // case Qt::Key_ydiaeresis   : return cgt::KeyEvent::K_;
            // case Qt::Key_Multi_key    : return cgt::KeyEvent::K_;
            // case Qt::Key_Codeinput    : return cgt::KeyEvent::K_;
            // case Qt::Key_SingleCandidate  : return cgt::KeyEvent::K_;
            // case Qt::Key_MultipleCandidate    : return cgt::KeyEvent::K_;
            // case Qt::Key_PreviousCandidate    : return cgt::KeyEvent::K_;
            // case Qt::Key_Mode_switch  : return cgt::KeyEvent::K_;
            // case Qt::Key_Kanji    : return cgt::KeyEvent::K_;
            // case Qt::Key_Muhenkan     : return cgt::KeyEvent::K_;
            // case Qt::Key_Henkan   : return cgt::KeyEvent::K_;
            // case Qt::Key_Romaji   : return cgt::KeyEvent::K_;
            // case Qt::Key_Hiragana     : return cgt::KeyEvent::K_;
            // case Qt::Key_Katakana     : return cgt::KeyEvent::K_;
            // case Qt::Key_Hiragana_Katakana    : return cgt::KeyEvent::K_;
            // case Qt::Key_Zenkaku  : return cgt::KeyEvent::K_;
            // case Qt::Key_Hankaku  : return cgt::KeyEvent::K_;
            // case Qt::Key_Zenkaku_Hankaku  : return cgt::KeyEvent::K_;
            // case Qt::Key_Touroku  : return cgt::KeyEvent::K_;
            // case Qt::Key_Massyo   : return cgt::KeyEvent::K_;
            // case Qt::Key_Kana_Lock    : return cgt::KeyEvent::K_;
            // case Qt::Key_Kana_Shift   : return cgt::KeyEvent::K_;
            // case Qt::Key_Eisu_Shift   : return cgt::KeyEvent::K_;
            // case Qt::Key_Eisu_toggle  : return cgt::KeyEvent::K_;
            // case Qt::Key_Hangul   : return cgt::KeyEvent::K_;
            // case Qt::Key_Hangul_Start     : return cgt::KeyEvent::K_;
            // case Qt::Key_Hangul_End   : return cgt::KeyEvent::K_;
            // case Qt::Key_Hangul_Hanja     : return cgt::KeyEvent::K_;
            // case Qt::Key_Hangul_Jamo  : return cgt::KeyEvent::K_;
            // case Qt::Key_Hangul_Romaja    : return cgt::KeyEvent::K_;
            // case Qt::Key_Hangul_Jeonja    : return cgt::KeyEvent::K_;
            // case Qt::Key_Hangul_Banja     : return cgt::KeyEvent::K_;
            // case Qt::Key_Hangul_PreHanja  : return cgt::KeyEvent::K_;
            // case Qt::Key_Hangul_PostHanja     : return cgt::KeyEvent::K_;
            // case Qt::Key_Hangul_Special   : return cgt::KeyEvent::K_;
            // case Qt::Key_Dead_Grave   : return cgt::KeyEvent::K_;
            // case Qt::Key_Dead_Acute   : return cgt::KeyEvent::K_;
            // case Qt::Key_Dead_Circumflex  : return cgt::KeyEvent::K_;
            // case Qt::Key_Dead_Tilde   : return cgt::KeyEvent::K_;
            // case Qt::Key_Dead_Macron  : return cgt::KeyEvent::K_;
            // case Qt::Key_Dead_Breve   : return cgt::KeyEvent::K_;
            // case Qt::Key_Dead_Abovedot    : return cgt::KeyEvent::K_;
            // case Qt::Key_Dead_Diaeresis   : return cgt::KeyEvent::K_;
            // case Qt::Key_Dead_Abovering   : return cgt::KeyEvent::K_;
            // case Qt::Key_Dead_Doubleacute     : return cgt::KeyEvent::K_;
            // case Qt::Key_Dead_Caron   : return cgt::KeyEvent::K_;
            // case Qt::Key_Dead_Cedilla     : return cgt::KeyEvent::K_;
            // case Qt::Key_Dead_Ogonek  : return cgt::KeyEvent::K_;
            // case Qt::Key_Dead_Iota    : return cgt::KeyEvent::K_;
            // case Qt::Key_Dead_Voiced_Sound    : return cgt::KeyEvent::K_;
            // case Qt::Key_Dead_Semivoiced_Sound    : return cgt::KeyEvent::K_;
            // case Qt::Key_Dead_Belowdot    : return cgt::KeyEvent::K_;
            // case Qt::Key_Dead_Hook    : return cgt::KeyEvent::K_;
            // case Qt::Key_Dead_Horn    : return cgt::KeyEvent::K_;
            // case Qt::Key_Back     : return cgt::KeyEvent::K_;
            // case Qt::Key_Forward  : return cgt::KeyEvent::K_;
            // case Qt::Key_Stop     : return cgt::KeyEvent::K_;
            // case Qt::Key_Refresh  : return cgt::KeyEvent::K_;
            // case Qt::Key_VolumeDown   : return cgt::KeyEvent::K_;
            // case Qt::Key_VolumeMute   : return cgt::KeyEvent::K_;
            // case Qt::Key_VolumeUp     : return cgt::KeyEvent::K_;
            // case Qt::Key_BassBoost    : return cgt::KeyEvent::K_;
            // case Qt::Key_BassUp   : return cgt::KeyEvent::K_;
            // case Qt::Key_BassDown     : return cgt::KeyEvent::K_;
            // case Qt::Key_TrebleUp     : return cgt::KeyEvent::K_;
            // case Qt::Key_TrebleDown   : return cgt::KeyEvent::K_;
            // case Qt::Key_MediaPlay    : return cgt::KeyEvent::K_;
            // case Qt::Key_MediaStop    : return cgt::KeyEvent::K_;
            // case Qt::Key_MediaPrevious    : return cgt::KeyEvent::K_;
            // case Qt::Key_MediaNext    : return cgt::KeyEvent::K_;
            // case Qt::Key_MediaRecord  : return cgt::KeyEvent::K_;
            // case Qt::Key_HomePage     : return cgt::KeyEvent::K_;
            // case Qt::Key_Favorites    : return cgt::KeyEvent::K_;
            // case Qt::Key_Search   : return cgt::KeyEvent::K_;
            // case Qt::Key_Standby  : return cgt::KeyEvent::K_;
            // case Qt::Key_OpenUrl  : return cgt::KeyEvent::K_;
            // case Qt::Key_LaunchMail   : return cgt::KeyEvent::K_;
            // case Qt::Key_LaunchMedia  : return cgt::KeyEvent::K_;
            // case Qt::Key_Launch0  : return cgt::KeyEvent::K_;
            // case Qt::Key_Launch1  : return cgt::KeyEvent::K_;
            // case Qt::Key_Launch2  : return cgt::KeyEvent::K_;
            // case Qt::Key_Launch3  : return cgt::KeyEvent::K_;
            // case Qt::Key_Launch4  : return cgt::KeyEvent::K_;
            // case Qt::Key_Launch5  : return cgt::KeyEvent::K_;
            // case Qt::Key_Launch6  : return cgt::KeyEvent::K_;
            // case Qt::Key_Launch7  : return cgt::KeyEvent::K_;
            // case Qt::Key_Launch8  : return cgt::KeyEvent::K_;
            // case Qt::Key_Launch9  : return cgt::KeyEvent::K_;
            // case Qt::Key_LaunchA  : return cgt::KeyEvent::K_;
            // case Qt::Key_LaunchB  : return cgt::KeyEvent::K_;
            // case Qt::Key_LaunchC  : return cgt::KeyEvent::K_;
            // case Qt::Key_LaunchD  : return cgt::KeyEvent::K_;
            // case Qt::Key_LaunchE  : return cgt::KeyEvent::K_;
            // case Qt::Key_LaunchF  : return cgt::KeyEvent::K_;
            // case Qt::Key_MediaLast    : return cgt::KeyEvent::K_;
            // case Qt::Key_unknown  : return cgt::KeyEvent::K_;
            // case Qt::Key_Call     : return cgt::KeyEvent::K_;
            // case Qt::Key_Context1     : return cgt::KeyEvent::K_;
            // case Qt::Key_Context2     : return cgt::KeyEvent::K_;
            // case Qt::Key_Context3     : return cgt::KeyEvent::K_;
            // case Qt::Key_Context4     : return cgt::KeyEvent::K_;
            // case Qt::Key_Flip     : return cgt::KeyEvent::K_;
            // case Qt::Key_Hangup   : return cgt::KeyEvent::K_;
            // case Qt::Key_No   : return cgt::KeyEvent::K_;
            // case Qt::Key_Select   : return cgt::KeyEvent::K_;
            // case Qt::Key_Yes : return cgt::KeyEvent::K_;
        default: return cgt::KeyEvent::K_UNKNOWN;
    }
}

void QtCanvas::setSize(ivec2 newSize) {
    // pass size change command through Qt's event messaging system to enforce execution in GUI thread.
    // (setSize() may be called externally from a different thread).
    emit s_sizeChangedExternally(newSize.x, newSize.y);
}

QSize QtCanvas::sizeHint() const {
    const ivec2& size = getSize();

    return QSize(size.x, size.y);
}

void QtCanvas::acquireAsCurrentContext() {
    QGLWidget::makeCurrent();
}

void QtCanvas::releaseAsCurrentContext() {
    QGLWidget::doneCurrent();
}

void QtCanvas::sizeChangedExternally(int w, int h) {
    QWidget::resize(w, h);
}


} // namespace
