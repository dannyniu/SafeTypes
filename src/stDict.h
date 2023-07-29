/* DannyNiu/NJF, 2023-07-16. Public Domain. */

#ifndef SafeTypes_Dict_H
#define SafeTypes_Dict_H 1

#include "stData.h"
#include "stContainer.h"

enum st_dict_member_flags {
    st_dict_member_null = 0, // unset.
    st_dict_member_set = 1, // strong reference.
    st_dict_member_collision = 2, // collision in hashed key.
};

// Using SipHash-2-4-128.
#define ST_DICT_HASH_MAX 16

typedef struct stDict stDict_t;

struct st_dict_member {
    int flags;
    stDict_t *collection;
    stData_t *key;
    union {
        stObj_t *value;
        struct st_dict_table *nested;
    };
};

struct st_dict_table {
    int level; // 0 at root. 
    struct st_dict_member members[256];
};

struct stDict {
    stObj_t hdrObj;
    int iterlevel;
    int iterpos[ST_DICT_HASH_MAX];
    struct st_dict_table root;
};

void siphash_setkey(void const *restrict in, size_t inlen);

stDict_t *stDictInit(stDict_t *dict);

// ``refcnt'' and ``keptcnt'' are not incremented, because
// they're still available from the dict (i.e. not unset).
// They have to be explicitly retained.
int stDictGet(stDict_t *dict, stData_t *key, stObj_t **out);
int stDictSet(stDict_t *dict, stData_t *key, stObj_t *value, int semantic);
int stDictUnset(stDict_t *dict, stData_t *key);

int stDictIterate(stDict_t *dict);

void stDictFinal(stDict_t *dict);

#endif /* SafeTypes_Dict_H */
