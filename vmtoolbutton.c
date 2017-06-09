#include "vmtoolbutton.h"

G_DEFINE_TYPE( VmToolButton, vm_tool_button, GTK_TYPE_TOOL_BUTTON )

static void
vm_tool_button_class_init (VmToolButtonClass *klass)
{
}

static void
vm_tool_button_init (VmToolButton *button)
{
}

VmToolButton *
vm_tool_button_new (GtkWidget   *icon_widget,
                          const char *label)
{
  VmToolButton *button;

  button = g_object_new (TYPE_VM_TOOL_BUTTON, NULL);

  if (label)
    gtk_tool_button_set_label (GTK_TOOL_BUTTON (button), label);

  if (icon_widget)
    gtk_tool_button_set_icon_widget (GTK_TOOL_BUTTON (button), icon_widget);

  return button;
}
