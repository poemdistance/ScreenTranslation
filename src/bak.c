
///*放置退出按钮到指定位置*/
//GtkWidget *fixed;
//fixed = gtk_fixed_new();
//gtk_container_add(GTK_CONTAINER(lbox), fixed);

//GtkWidget *button = gtk_button_new_with_label("exit");
//gtk_widget_set_size_request(button, 10,6);
//gtk_fixed_put(GTK_FIXED(fixed), button, 0, 0);
//g_signal_connect(button, "clicked", G_CALLBACK(destroy_newwin_by_clicked), newWin);
//g_signal_connect(newWin, "destroy", G_CALLBACK(gtk_main_quit), newWin);
//gtk_fixed_put(GTK_FIXED(fixed), view, 0, 40);
//
//printf("原始数据:%s\n", &shmaddr[1]);
//
//
//char *p = &shmaddr[1];
//int index=1; /*同*p一致，指向第一个字符*/
//int chNum = 0;
//int i[2] = {0};

//while(*p) {
//    //if ( *p == '\n' ) *p = '.';
//    if ( *p == '|') {
//        *p = '\0';

//        if ( chNum >= 3 ) /*只截取到第三个分隔符，剩下翻译结果丢弃*/
//            break;
//        i[chNum++] = index+1;  /*记录下标*/
//    }
//    p++;
//    index++;
//}

//char *result = &shmaddr[1];
//char *explain = NULL;
//char *other = NULL;

//if ( result != NULL )
//    printf("result:%s\n", result);

//if ( i[0] != 0 ) 
//    explain = &shmaddr[i[0]];
//if ( i[1] != 0)
//    other = &shmaddr[i[1]];

//if ( explain != NULL )
//    printf(">>>>>>>>>>>>>>>>>>explain:%s\n", explain); 
//if ( other != NULL )
//    printf(">>>>>>>>>>>>>>>>>>other:%s\n", other);

//gtk_text_buffer_set_text(buf, &shmaddr[1], -1);

//GtkWidget *btnTxt = gtk_button_new_with_label(&shmaddr[1]);
//gtk_fixed_put(GTK_FIXED(fixed), btnTxt, 3, 90);
