#include <gst/gst.h>

int main(int argc, char *argv[]) {
  /* init GStreamer */
  gst_init(&argc, &argv);

  GstElement *element;

#if 1

  /* create element */
  element = gst_element_factory_make("fakesrc", "source");
  if (!element) {
    g_print("Failed to create element of type 'fakesrc'\n");
    return -1;
  }

#else
  GstElementFactory *factory;

  /* create element, method #2 */
  factory = gst_element_factory_find("fakesrc");
  if (!factory) {
    g_print("Failed to find factory of type 'fakesrc'\n");
    return -1;
  }
  element = gst_element_factory_create(factory, "source");
  if (!element) {
    g_print("Failed to create element, even though its factory exists!\n");
    return -1;
  }

  gst_object_unref(GST_OBJECT(factory));

#endif

  /* get name */
  gchar *name;
  g_object_get(G_OBJECT(element), "name", &name, NULL);
  g_print("The name of the element is '%s'.\n", name);
  g_free(name);

  gst_object_unref(GST_OBJECT(element));
  return 0;
}
