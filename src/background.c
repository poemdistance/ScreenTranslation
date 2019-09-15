#include "common.h"

GtkWidget * getBackground( gint width, gint height, WinData *wd  ) 
{
    /* 类似于这样的图片比较大，可以提前加载完成，不过意义不大，
     * 因为翻译结果的获取才是最慢的那个*/
    if ( wd->srcBackgroundImage == NULL )
        wd->srcBackgroundImage = gdk_pixbuf_new_from_file ( "/home/rease/.stran/background.jpg" , NULL);

    GdkPixbuf *dst = gdk_pixbuf_scale_simple ( wd->srcBackgroundImage, width, height, GDK_INTERP_BILINEAR  );

    GtkWidget *image = gtk_image_new_from_pixbuf ( dst );

    g_object_unref ( dst );

    gtk_widget_set_opacity ( image, 0.13 );

    return image;
}

GtkWidget *syncImageSize ( GtkWidget *window, gpointer *data ) 
{
    gint width, height;
    gtk_window_get_size ( (GtkWindow*)window, &width, &height );

    /* FIXME:与或操作换了，先测试看问题是不是在这里*/
    if ( ((WinData*)data)->width > width ||  ((WinData*)data)->height > height ) {

        width  = ((WinData*)data)->width ;
        height  = ((WinData*)data)->height ;
    }
    
    //printf("\033[0;31mSyncImageSize current win size %d %d \033[0m\n", width, height);

    /* 销毁原来的背景图片*/
    if ( ((WinData*)data)->oldImage != NULL )
        gtk_widget_destroy ( ((WinData*)data)->oldImage );

    /* 获取新尺寸的背景图*/
    GtkWidget *image = getBackground ( width, height, (WinData*)data);
    GtkLayout *layout = (GtkLayout*)((WinData*)data)->layout;

    gtk_widget_set_size_request ( image, width, height );
    gtk_widget_queue_draw (image);
    gtk_widget_queue_draw ( ((WinData*)data)->window );

    gtk_layout_put ( layout, image, 0, 0 );

    gtk_widget_show (image);
    gtk_widget_show (((WinData*)data)->window);

    ((WinData*)data)->oldImage = image;

    return image;
}
