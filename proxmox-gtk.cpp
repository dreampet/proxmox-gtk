#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <string.h>
#include "images/computer-running.xpm"
#include "images/computer-stopped.xpm"
#include "pveapi.h"
#include "vmtoolbutton.h"

class Palette
{
  private:
    GtkWidget	*palette;
    PVEAPI	*pve;
    PVE_VM	*current;

  public:
    void SetCurrent( PVE_VM *_current )
    {
      current = _current;
    }
    PVE_VM *GetCurrent()
    {
      return current;
    }

    Palette()
    {
      palette = gtk_tool_palette_new ();
      gtk_tool_palette_set_style( GTK_TOOL_PALETTE(palette), GTK_TOOLBAR_BOTH );
      gtk_orientable_set_orientation ( GTK_ORIENTABLE( palette), GTK_ORIENTATION_VERTICAL );
      pve = new PVEAPI();
    }
    ~Palette()
    {
      g_free(palette);
      delete pve;
    }
    
    gboolean Login()
    {
      Jzon::Parser parser;
      PVE_LOGIN	login = parser.parseFile("config.json");
      return Palette::pve->Authentication(&login);
    }

    //  callback this when popmenu item was clicked
    void static Callback_Menu( GtkMenuItem *menuitem, gpointer user_data )
    {
      Palette *palette = (Palette *)user_data;
      PVE_VM	*vm = palette->GetCurrent();

      std::cout << vm->get("name").toString();
      std::cout << gtk_menu_item_get_label( menuitem ) << std::endl;
    }

    //  callback this when palette items was clicked
    void static Callback_Button( VmToolButton *button, gpointer user_data )
    {
      Palette *palette = (Palette *)user_data;
      PVE_VM	*vm = (PVE_VM *)button->user_data;

      palette->SetCurrent( vm );

      PVEAPI_VM_STATUS	status = (PVEAPI_VM_STATUS) vm->get("status").toInt();
      const char *items_str_running[] = {"Shutdown", "Stop", "Reset"};
      const char *items_str_stopped[] = {"Start"};
      const char **items_str = status == VM_RUNNING ? items_str_running : items_str_stopped;

      GtkWidget	*menu;
      menu = gtk_menu_new();
      for (int i = 0; i < sizeof( items_str ) / sizeof( char * ); i++)
      {
        GtkWidget	*item;
        item = gtk_menu_item_new_with_label( items_str[i] );
        gtk_widget_show( item );
        g_signal_connect (item, "activate", G_CALLBACK (Callback_Menu), user_data );
        gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
      }
      gtk_menu_popup_at_pointer( GTK_MENU(menu), NULL );
    }

    // you should do authention before here
    void Init()
    {
      std::list<PVE_NODE *> nodes = pve->Nodes();
      for (std::list<PVE_NODE *>::iterator i = nodes.begin(); i != nodes.end(); i++)
      {
        GtkWidget *group = gtk_tool_item_group_new( (*i)->get("name").toString().c_str() );
        gtk_container_add( GTK_CONTAINER (palette), group );

        std::list<PVE_VM *> items = pve->Vms(*i);
        for (std::list<PVE_VM *>::iterator j = items.begin(); j != items.end(); j++)
        {
          VmToolButton	*item;
          PVEAPI_VM_STATUS status = pve->Status( *j );
          const char	**icon = status == VM_RUNNING ? computer_running : computer_stopped;
          GtkWidget *image = gtk_image_new_from_pixbuf( gdk_pixbuf_new_from_xpm_data( icon ) );
          char  *label = g_strdup_printf( "%s\n(%d)", (*j)->get("name").toString().c_str(), (*j)->get("id").toInt() );
          item = vm_tool_button_new ( image, label );
          item->user_data = *j;
          g_free (label);

          g_signal_connect (item, "clicked", G_CALLBACK (Callback_Button), this);

          gtk_tool_item_group_insert (GTK_TOOL_ITEM_GROUP( group ), GTK_TOOL_ITEM(item), -1 );
        }
      }
    }

    GtkWidget *GetPalette()
    {
      return palette;
    }
};

int main (int argc, char *argv[])
{
  GtkWidget *window;

  gtk_init (&argc, &argv);

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
//  gtk_window_set_screen (GTK_WINDOW (window), NULL);
  gtk_window_set_title(GTK_WINDOW(window), "Proxmox VE");

  gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);
  gtk_container_set_border_width (GTK_CONTAINER (window), 8);
//  gtk_window_set_geometry_hints(GTK_WINDOW(window), NULL, NULL, 0);

  g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

  Palette	*palette;
  palette = new Palette();
  if (palette->Login())
    palette->Init();
  else
  {
    std::cerr << "Login error: please check you config file" << std::endl;
    gtk_main_quit();
  }

  GtkWidget *palette_scroller = gtk_scrolled_window_new( NULL, NULL );
  gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(palette_scroller), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_container_set_border_width( GTK_CONTAINER( palette_scroller), 6 );
  gtk_widget_set_hexpand( palette_scroller, TRUE );

  gtk_container_add( GTK_CONTAINER (palette_scroller), palette->GetPalette() );
  gtk_container_add( GTK_CONTAINER (window), palette_scroller );

  gtk_widget_show_all (window);

  gtk_main ();

  return 0;
}
