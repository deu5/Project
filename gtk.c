#include <gtk/gtk.h>
void quit (GtkWidget *window, gpointer data)
{
gtk_main_quit();
}
int main (int argc, char *argv[])
{
char a[] = "abc\n";



GtkWidget *window;
GtkWidget *label1, *label2, *label3;
GtkWidget *vbox1, *vbox2; //세로등분
GtkWidget *hbox1, *hbox2, *hbox3;//가로등분
GtkWidget *entry;
GtkWidget *message_label;
GtkWidget *button1, *button2, *button3, *button4, *button5;
GtkWidget *separator;

gtk_init (&argc, &argv);
window = gtk_window_new (GTK_WINDOW_TOPLEVEL);//화면만드는거??

gtk_window_set_title (GTK_WINDOW (window), "걸걸이"); //박스이름
gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER); //??위치시킴??
gtk_window_set_default_size (GTK_WINDOW (window), 500, 300); //박스크기 가로,세로


g_signal_connect (GTK_OBJECT(window), "destroy",GTK_SIGNAL_FUNC (quit), NULL);//윈도우 닫을때 시그널발생

vbox1 = gtk_vbox_new (TRUE, 15);
vbox2 = gtk_vbox_new (TRUE, 15);
hbox1 = gtk_hbox_new (TRUE, 15);
hbox2 = gtk_hbox_new (TRUE, 15);
hbox3 = gtk_hbox_new (TRUE, 15);

separator = gtk_hseparator_new();


message_label = gtk_label_new (a);

entry = gtk_entry_new ();

button1 = gtk_button_new_with_label ("보내기1");
button2 = gtk_button_new_with_label ("보내기2");
button3 = gtk_button_new_with_label ("보내기3");
button4 = gtk_button_new_with_label ("보내기4");
button5 = gtk_button_new_with_label ("보내기5");

gtk_widget_set_size_request(hbox1 , 480, 200);
gtk_widget_set_size_request(message_label, 450, 100);

gtk_widget_set_size_request(hbox2 , 480, 30);
gtk_widget_set_size_request(entry, 450, 20);
gtk_widget_set_size_request(hbox3 , 480, 30);

///////////////(부모박스/ 자식박스 / T-박스내 빈공간을 자식이채움 /T-<같음 / 여백크기)
//////////////                       F-박스가 자식크기에 맞게     /F-박스의 빈공간을 여백으로 채움
//h수평  v수직

gtk_box_pack_start (GTK_BOX(vbox1), hbox1, FALSE, FALSE, 0);
gtk_box_pack_start (GTK_BOX(hbox1), message_label, FALSE, FALSE, 0);


gtk_box_pack_start_defaults (GTK_BOX(vbox1), separator); 
gtk_box_pack_start (GTK_BOX(vbox1), vbox2, FALSE, FALSE, 0);
gtk_box_pack_start (GTK_BOX(vbox2), hbox2, FALSE, FALSE, 0);
gtk_box_pack_start (GTK_BOX(hbox2), entry, FALSE, FALSE, 0);

gtk_box_pack_start_defaults (GTK_BOX(vbox2), separator); 
gtk_box_pack_start (GTK_BOX(vbox2), hbox3, FALSE, FALSE, 0);
gtk_box_pack_start_defaults (GTK_BOX(hbox3), button1);
gtk_box_pack_start_defaults (GTK_BOX(hbox3), button2);
gtk_box_pack_start_defaults (GTK_BOX(hbox3), button3);
gtk_box_pack_start_defaults (GTK_BOX(hbox3), button4);
gtk_box_pack_start_defaults (GTK_BOX(hbox3), button5);



gtk_container_add (GTK_CONTAINER (window), vbox1);

gtk_widget_show_all (window);

gtk_main ();

return (0);
}
