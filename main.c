/*
* MIT License
*
* Copyright (c) 2026 ArthurPV
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <X11/Xlib.h>

#define LOG_ERROR(msg, ...) \
	fprintf(stderr, "Error(%s:%d): "msg, __FILE__, __LINE__, ##__VA_ARGS__); \
	exit(1);

#define RGB(r, g, b) ((r) << 16 | (g) << 8 | (b))

struct Vec2 {
	int x, y;
};

// 1========2
// |        |
// |        |
// |        |
// 3========4
struct Square {
	struct Vec2 _1, _2, _3, _4;
};

static void draw_square__3DCube(int x, int y, int c, struct Square *square_out);

static void join_squares__3DCube(const struct Square *s1, const struct Square *s2);

static void draw__3DCube(int x, int y, int c);

static void handle_events__3DCube();

static void close__3DCube();

static Display *display = NULL;
static GC window_gc = {0};
static Window window = -1;
static Atom wmDeleteWindow = {0};
static Atom wmProtocols = {0};
static const unsigned int window_width = 500;
static const unsigned int window_height = 500;

void draw_square__3DCube(int x, int y, int c, struct Square *square_out)
{
	int xi1 = x - c;
	int xi2 = xi1 + c;
	int yi = y - c;

	square_out->_1.x = xi1;
	square_out->_1.y = yi;

	square_out->_2.x = xi2;
	square_out->_2.y = yi;

	square_out->_3.x = xi1;
	square_out->_3.y = yi + c;

	square_out->_4.x = xi2;
	square_out->_4.y = yi + c;

	XDrawLine(display, window, window_gc, square_out->_1.x, square_out->_1.y, square_out->_2.x, square_out->_2.y);
	XDrawLine(display, window, window_gc, square_out->_2.x, square_out->_2.y, square_out->_4.x, square_out->_4.y);
	XDrawLine(display, window, window_gc, square_out->_1.x, square_out->_1.y, square_out->_3.x, square_out->_3.y);
	XDrawLine(display, window, window_gc, square_out->_3.x, square_out->_3.y, square_out->_4.x, square_out->_4.y);
}

void join_squares__3DCube(const struct Square *s1, const struct Square *s2)
{
	XDrawLine(display, window, window_gc, s1->_1.x, s1->_1.y, s2->_1.x, s2->_1.y);
	XDrawLine(display, window, window_gc, s1->_2.x, s1->_2.y, s2->_2.x, s2->_2.y);
	XDrawLine(display, window, window_gc, s1->_3.x, s1->_3.y, s2->_3.x, s2->_3.y);
	XDrawLine(display, window, window_gc, s1->_4.x, s1->_4.y, s2->_4.x, s2->_4.y);
}

void draw__3DCube(int x, int y, int c)
{
	XClearWindow(display, window);
	XSetForeground(display, window_gc, RGB(255, 255, 255));

	struct Square s1 = {0}, s2 = {0};

	draw_square__3DCube(x, y, c, &s1);
	draw_square__3DCube(x + c / 2, y - c / 2, c, &s2);
	join_squares__3DCube(&s1, &s2);
}

void handle_events__3DCube()
{
	XEvent event;
	bool running = true;

	while (running) {
		XNextEvent(display, &event);

		switch (event.type) {
			case ClientMessage:
				if (event.xclient.message_type == wmProtocols &&
					(Atom)event.xclient.data.l[0] == wmDeleteWindow) {
					running = false;
				}

				break;
			case Expose:
				draw__3DCube(window_width / 2, window_height / 2, 100);

				break;
			default:
				break;
		}
	}
}

void close__3DCube() {
	XFreeGC(display, window_gc);
	XDestroyWindow(display, window);
	XCloseDisplay(display);
}

int main() {
	if (!(display = XOpenDisplay(NULL))) {
		LOG_ERROR("unable to open display\n");
	}

	Window window_root = XDefaultRootWindow(display);

	window = XCreateSimpleWindow(display, window_root, 0, 0, window_width, window_height, 0, RGB(0, 0, 0), RGB(0, 0, 0));
	window_gc = XCreateGC(display, window, 0, NULL);

	wmProtocols = XInternAtom(display, "WM_PROTOCOLS", False);
	wmDeleteWindow = XInternAtom(display, "WM_DELETE_WINDOW", False);

	XSetWMProtocols(display, window, &wmDeleteWindow, 1);
	XSelectInput(display, window, ExposureMask);

	XMapWindow(display, window);
	handle_events__3DCube();
	close__3DCube();
}
