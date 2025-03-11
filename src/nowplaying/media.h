#ifndef _MEDIA_H
#define _MEDIA_H

//structs and types
struct players {
};
struct media {
};

//prototypes
struct players *get_players();
void free_players(struct players *players);
struct media *get_current_media(int flags);
void free_current_media(struct media *media);

#endif
