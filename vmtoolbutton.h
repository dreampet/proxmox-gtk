#ifndef __VM_TOOL_BUTTON_H__
#define __VM_TOOL_BUTTON_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS
#define TYPE_VM_TOOL_BUTTON             (vm_tool_button_get_type ())
#define VM_TOOL_BUTTON(o)               (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_VM_TOOL_BUTTON, VmToolButton))
#define VM_TOOL_BUTTON_CLASS(k)         (G_TYPE_CHECK_CLASS_CAST((k), TYPE_VM_TOOL_BUTTON, VmToolButtonClass))
#define IS_VM_TOOL_BUTTON(o)            (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_VM_TOOL_BUTTON))
#define IS_VM_TOOL_BUTTON_CLASS(k)      (G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_VM_TOOL_BUTTON))
#define VM_TOOL_BUTTON_GET_CLASS(o)     (G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_VM_TOOL_BUTTON, VmToolButtonClass))

typedef struct _VmToolButtonClass   VmToolButtonClass;
typedef struct _VmToolButton        VmToolButton;

struct _VmToolButton
{
  GtkToolButton parent;
  void *user_data;
};

struct _VmToolButtonClass
{
  GtkToolButtonClass parent_class;
};

GDK_AVAILABLE_IN_ALL
GType         vm_tool_button_get_type       (void) G_GNUC_CONST;
GDK_AVAILABLE_IN_ALL
VmToolButton   *vm_tool_button_new           (GtkWidget   *icon_widget, const char *label);
G_END_DECLS

#endif /* __VM_TOOL_BUTTON_H__ */
