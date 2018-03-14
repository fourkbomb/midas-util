#include "llist.h"

struct widget {
    char *name;
    int intval;
    char *strval;
};

struct mixer_setting {
    char *name;

    /* widget settings for enabling this mixer path */
    node_t *en_widgets;

    /* widget settings for disabling this mixer path */
    node_t *dis_widgets;
};

struct mixer_config {
    /* mixer paths for speaker, headset, earpiece, etc. */
    node_t *settings;
  
    /* widget settings that are always set */
    node_t *default_widgets;
};

