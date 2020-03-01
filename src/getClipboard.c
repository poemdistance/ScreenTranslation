/*
 * 本程序功能：
 * 连接X server，获取剪贴板内容
 *
 * 注意Xdisplay 不能以root用户连接，否则报错:No protocol specified*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <X11/Xlib.h>
#include "common.h"

void show_utf8_prop(Display *display, Window w, Atom p, char *text)
{
    Atom da, incr, type;
    int di;
    unsigned long size, dul;
    unsigned char *prop_ret = NULL;

    /* Dummy call to get type and size. */
    XGetWindowProperty(display, w, p, 0, 0, False, AnyPropertyType,
            &type, &di, &dul, &size, &prop_ret);
    XFree(prop_ret);

    incr = XInternAtom(display, "INCR", False);
    if (type == incr)
    {
        fprintf(stderr,"Data too large and INCR mechanism not implemented\n");
        fprintf(stderr,"MARK\n");
        return;
    }

    XGetWindowProperty(display, w, p, 0, size, False, AnyPropertyType,
            &da, &di, &dul, &dul, &prop_ret);

    /*TODO:如果是截图的时候发送了Ctrl-C到了这里会崩溃掉
     * 应该在判断聚焦窗口的时候排除掉截图软件*/
    strcpy(text, (char*)prop_ret);

    XFree(prop_ret);

    /* Signal the selection owner that we have successfully read the
     * data. */
    XDeleteProperty(display, w, p);
}

int getClipboard(char *text)
{
    Display *display;
    Window owner, target_window, root;
    int screen;
    Atom sel, target_property, utf8;
    XEvent ev;
    XSelectionEvent *sev;

    /* 返回一个结构体：
     * 包含连接到xserver的全部信息*/
    display = XOpenDisplay(NULL);
    if (!display)
    {
        fprintf(stderr, "Could not open X display\n");
        return 1;
    }

    /*返回默认的被xopenDisplay引用的screen number*/
    screen = DefaultScreen(display);

    /*返回root window， 用于绘图或顶层窗口*/
    root = RootWindow(display, screen);


    /*
     * Atom XInternAtom(display, atom_name, only_if_exists)
     * Display *display;
     * char *atom_name;
     * Bool only_if_exists;
     *
     * return an atom for a given name
     * 返回原子标记符
     *
     * if only_if_exists is FALSE , the atom is created if 
     * it doesn't exists
     */
    //sel = XInternAtom(display, "CLIPBOARD", False);
    sel = XInternAtom(display, "PRIMARY", False);
    utf8 = XInternAtom(display, "UTF8_STRING", False);

    /*
     * returns the window id associated(关联) with the window
     * that currently owns the specified selection*/
    owner = XGetSelectionOwner(display, sel);

    if (owner == None)
    {
        fprintf(stderr,"'PRIMARY' has no owner\n");
        return 1;
    }

    /* The selection owner will store the data in a property on this
     * window: 
     *
     * crates a window that inherits its attribtes from its parant window*/
    target_window = XCreateSimpleWindow(display, root, -10, -10, 1, 1, 0, 0, 0);

    /*requests the X server report the events associated with the specificed 
     * event mask */
    XSelectInput(display, target_window, SelectionNotify);

    /* That's the property used by the owner. Note that it's completely
     * arbitrary.(随意地) */
    target_property = XInternAtom(display, "Alien", False);

    /* Request conversion to UTF-8. Not all owners will be able to
     * fulfill that request. 
     *
     * XConvertSelection requests that the specified selection be 
     * converted to the specified target type:
     * If the specified selection has an owner, the X server
     * sends a SelectionRequest event to that owner.
     * If no owner for the specified selection exists, the 
     * X server generates a SelectionNotify event to the requestor with property None.
     *

     *prototype:

     * XConvertSelection (display, selection, target, property, requestor, time)
     * Display *display;
     * Atom selection, target;
     * Atom property;
     * Window requestor;
     * Time time;
     */
    XConvertSelection(display, sel, utf8, target_property, target_window,
            CurrentTime);

    for (;;)
    {
        /*
         * copies the first event from the event queue into the specified 
         * XEvent structure and then removes it from the queue*/
        XNextEvent(display, &ev);

        /* SelectionNotify:
         * SelectionNotify would be sent by the owner to requestor when
         * a  selection has benn converted and stored as a property*/

        switch (ev.type)
        {
            case SelectionNotify:
                sev = (XSelectionEvent*)&ev.xselection;
                if (sev->property == None)
                {
                    fprintf(stderr,"Conversion could not be performed.\n");
                    XCloseDisplay(display);
                    return -1;

                } else {
                    char *name;
                    name = XGetAtomName(display, ((XSelectionRequestEvent*)&ev)->target);
                    XFree(name);
                    show_utf8_prop(display, target_window, target_property, text);
                    XCloseDisplay(display);
                    return 0;
                }
                break;

            default: break;
        }

        break;
    }

    XCloseDisplay ( display );
    return -1;
}
