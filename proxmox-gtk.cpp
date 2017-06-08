#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <string.h>
#include "images/computer-running.xpm"
#include "images/computer-stopped.xpm"
#include "pveapi.h"

class Palette
{
  private:
    GtkWidget	*palette;
    PVEAPI	*pve;

  public:
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
      return pve->Authentication(&login);
    }

    void Init()
    {
      std::list<PVE_NODE *> nodes = pve->Nodes();
      for (std::list<PVE_NODE *>::iterator i = nodes.begin(); i != nodes.end(); i++)
      {
        GtkWidget *node = gtk_tool_item_group_new( (*i)->get("name").toString().c_str() );
        gtk_container_add (GTK_CONTAINER (palette), node);

        std::list<PVE_VM *> vms = pve->Vms(*i);
        for (std::list<PVE_VM *>::iterator j = vms.begin(); j != vms.end(); j++)
        {
          GtkWidget *image;
          char      *label;
          if (pve->Status(*j) == VM_RUNNING)
            image = gtk_image_new_from_pixbuf( gdk_pixbuf_new_from_xpm_data ( computer_running) );
          else
            image = gtk_image_new_from_pixbuf( gdk_pixbuf_new_from_xpm_data ( computer_stopped) );
          label = g_strdup_printf( "%s\n(%d)", (*j)->get("name").toString().c_str(), (*j)->get("id").toInt() );
          GtkToolItem *item = gtk_tool_button_new (image, label );
          gtk_tool_item_group_insert (GTK_TOOL_ITEM_GROUP( node ), item, -1 );
          g_free (label);
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
  gtk_window_set_title(GTK_WINDOW(window), "Proxmox PI");

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
