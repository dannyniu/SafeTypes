/* DannyNiu/NJF, 2023-07-16. Public Domain. */

#include "stDict.h"
#include "siphash.h"

// 2024-06-15:
// The "TODO: errno" comments are altogether removed.
// According to Single Unix Specification, Rationales
// Volume, System Interfaces General Information, Error
// Numbers, it is more preferable that new functions
// report errors through return values, and this is
// the approach taken in SafeTypes.

typedef void (*rcdt_destructor_func)(void *);

static uint8_t key_siphash[16];

void siphash_setkey(void const *in, size_t inlen)
{
    // implicitly truncates or zero-extends to 16 bytes.
    size_t t;

    for(t=0; t<inlen && t<sizeof(key_siphash); t++)
        key_siphash[t] = ((uint8_t const *)in)[t];

    for(; t<sizeof(key_siphash); t++)
        key_siphash[t] = 0;
}

stDict_t *stDictInit(stDict_t *dict)
{
    int i;
    if( !stObjInit(&dict->hdrObj) ) return NULL;

    dict->hdrObj.type = ST_OBJ_TYPE_DICT;
    dict->hdrObj.finalf = (stFinalFunc_t)stDictFinal;
    dict->hdrObj.iterf = (stIterFunc_t)stDictIterate;
    
    dict->root.level = 0;
    
    for(i=0; i<256; i++)
    {
        dict->root.members[i].flags = st_dict_member_null;
        dict->root.members[i].collection = dict;
        dict->root.members[i].key = NULL;
        dict->root.members[i].value = NULL;
    }

    for(i=0; i<ST_DICT_HASH_MAX; i++)
    {
        dict->iterpos[i] = 0;
    }

    return dict;
}

int stDictGet(stDict_t *dict, stData_t *key, stObj_t **out)
{
    size_t klen = stDataLen(key);
    uint8_t hash[ST_DICT_HASH_MAX];
    siphash_t hx;
    int level = 0;

    struct st_dict_table *T;
    struct st_dict_member *M;

    SipHash_o128_Init(&hx, key_siphash, sizeof(key_siphash));
    SipHash_c2_Update(&hx, stDataMap(key, 0, klen), klen);
    SipHash_c2d4o128_Final(&hx, hash, ST_DICT_HASH_MAX);
    stDataUnmap(key);

    T = &dict->root;
    while( true )
    {
        M = T->members + hash[level];

        switch( M->flags ){
        case st_dict_member_null:
            *out = NULL;
            return st_access_nullval;
            break;

        case st_dict_member_set:
            if( klen == stDataLen(M->key) &&
                !memcmp(key->buf, M->key->buf, klen) )
            {
                *out = M->value;
                return st_access_success;
            }
            else
            {
                *out = NULL;
                return st_access_nullval;
            }
            break;

        case st_dict_member_collision:
            if( ++level < ST_DICT_HASH_MAX ) T = M->nested; else
            {
                *out = NULL;
                return st_access_error;
            }
            break;


        default:
            *out = NULL;
            return st_access_error;
            break;
        }
    }
}

// Unset is logically similar to Get. Implement it first.
int stDictUnset(stDict_t *dict, stData_t *key)
{
    size_t klen = stDataLen(key);
    uint8_t hash[ST_DICT_HASH_MAX];
    siphash_t hx;
    int level = 0;

    struct st_dict_table *T;
    struct st_dict_member *M;

    SipHash_o128_Init(&hx, key_siphash, sizeof(key_siphash));
    SipHash_c2_Update(&hx, stDataMap(key, 0, klen), klen);
    SipHash_c2d4o128_Final(&hx, hash, ST_DICT_HASH_MAX);
    stDataUnmap(key);

    T = &dict->root;
    while( true )
    {
        M = T->members + hash[level];

        switch( M->flags ){
        case st_dict_member_null:
            return st_access_nullval;
            break;

        case st_dict_member_set:
            if( klen != stDataLen(M->key) ||
                memcmp(key->buf, M->key->buf, klen) )
                return st_access_nullval;
            
            stObjLeave((stObj_t *)M->value);
            stObjRelease((stObj_t *)M->key);
            M->flags = st_dict_member_null;
            M->value = NULL;
            return st_access_success;
            break;
            
        case st_dict_member_collision:
            if( ++level < ST_DICT_HASH_MAX ) T = M->nested; else
            {
                return st_access_error;
            }
            break;


        default:
            return st_access_error;
            break;
        }
    }
}

int stDictSet(stDict_t *dict, stData_t *key, stObj_t *value, int semantic)
{
    size_t klen = stDataLen(key);
    uint8_t hash[ST_DICT_HASH_MAX], h2[ST_DICT_HASH_MAX];
    int level = 0;

    siphash_t hx;

    struct st_dict_table *U;
    struct st_dict_table *T;
    struct st_dict_member *M;
    void const *M_key;
    size_t M_key_len;

    assert( semantic == st_setter_kept ||
            semantic == st_setter_gave ||
            semantic == st_setter_fast );
    
    SipHash_o128_Init(&hx, key_siphash, sizeof(key_siphash));
    SipHash_c2_Update(&hx, stDataMap(key, 0, klen), klen);
    SipHash_c2d4o128_Final(&hx, hash, ST_DICT_HASH_MAX);
    stDataUnmap(key);

    T = &dict->root;
    while( level >= 0 )
    {
        M = T->members + hash[level];

        switch( M->flags ){
        case st_dict_member_null:
            level = -1;
            break;

        case st_dict_member_set:
            M_key = stDataMap(M->key, 0, 0);
            M_key_len = stDataLen(M->key);
            
            if( klen == M_key_len &&
                memcmp(key, M_key, M_key_len) == 0 )
            {
                stDataUnmap(M->key);
                goto replace_value_prepare;
            }
            
            // hash collision. do the following:
            // 1. increment ``level''.
            // 2. calculate h2 from the key of M if it hasn't been done.
            // 3. create subtable U.
            // 4. move M to U.
            // 5. attach U to T.
            // 6. test for collision:
            // 6.1. if collision, restart with U as T.
            // 6.2. otherwise, add ``value'' to U, and done.

            U = NULL;

            while( true )
            {
                if( ++level >= ST_DICT_HASH_MAX )
                {
                    stDataUnmap(M->key);
                    return st_access_error;
                    /* NOTREACHED */
                }
                
                if( !U )
                {
                    SipHash_o128_Init(
                        &hx, key_siphash, sizeof(key_siphash));
                            
                    SipHash_c2_Update(
                        &hx, M_key, M_key_len);
                            
                    SipHash_c2d4o128_Final(
                        &hx, h2, ST_DICT_HASH_MAX);
                    
                    stDataUnmap(M->key);
                    // siphash24(M->key, strlen(M->key), h2);
                }
                
                U = calloc(1, sizeof(struct st_dict_table));
                if( !U )
                {
                    return st_access_error;
                }
                U->level = level;
                
                memcpy(U->members + h2[level], M,
                       sizeof(struct st_dict_member));

                M->flags = st_dict_member_collision;
                M->nested = U;
                M->key = NULL;

                T = U;
                M = T->members + hash[level];
                    
                if( hash[level] == h2[level] )
                {
                    continue;
                }
                else break;
            }

        replace_value_prepare:
            level = -1;
            break;

        case st_dict_member_collision:
            if( ++level < ST_DICT_HASH_MAX ) T = M->nested; else
            {
                return st_access_error;
            }
            break;

        default:
            return st_access_error;
            break;
        }
    }

    assert(M);

    if( M->flags == st_dict_member_set )
        stObjLeave(M->value);

    M->flags = st_dict_member_set;
    M->collection = dict;
    if( !M->key )
    {
        M->key = calloc(1, sizeof(stData_t));
        stDataInit(M->key, klen);
        
        memcpy(
            stDataMap(M->key, 0, klen),
            stDataMap(key, 0, klen), klen);
        stDataUnmap(M->key);
        stDataUnmap(key);
    }
    M->value = value;

    switch( semantic ){
    case st_setter_kept:
        stObjKeep(value);
        break;

    case st_setter_gave:
        stObjKeep(value);
        stObjRelease(value);
        break;
        
    case st_setter_fast:
        ++ value->keptcnt;
        -- value->refcnt;
        break;
    }
    
    return st_access_success;
}

int stDictIterate(stDict_t *dict)
{
    int i, level;
    struct st_dict_table *T;

    if( !dict->hdrObj.iterp )
    {
        for(i=0; i<ST_DICT_HASH_MAX; i++)
            dict->iterpos[i] = 0;
    }

descend_in:
    level = dict->iterlevel;
    T = &dict->root;
    for(i=0; i<level; i++)
    {
        T = T->members[dict->iterpos[i]].nested;
    }

dive_in:
    i = dict->iterpos[level];

    if( i >= 256 )
    {
        dict->iterpos[level] = 0;
        if( level -- == 0 )
        {
            dict->hdrObj.iterp = NULL;

            dict->iterlevel = 0;
            for(i=0; i<ST_DICT_HASH_MAX; i++)
            {
                dict->iterpos[i] = 0;
            }
            
            return 0;
        }
        else
        {
            dict->iterlevel = level;
            dict->iterpos[level] ++;
            goto descend_in;
        }
    }
    
    if( T->members[i].flags == st_dict_member_collision )
    {
        T = T->members[i].nested;
        ++ level;
        assert( level < ST_DICT_HASH_MAX );
        
        dict->iterlevel = level;
        dict->iterpos[level] = 0;
        goto dive_in;
    }
    else if( T->members[i].flags == st_dict_member_null )
    {
        dict->iterpos[level] ++;
        goto dive_in;
    }
    else if( T->members[i].flags == st_dict_member_set )
    {
        dict->hdrObj.iterp   = T->members[i].value;
        dict->hdrObj.iterk.p = T->members[i].key;
        dict->iterpos[level] ++;
        return 0;
    }

    return 0; // Shoud be -1 ?
}

static void stDict_FreeTable(struct st_dict_table *table);
static void stDict_FreeMember(struct st_dict_member *m)
{
    switch( m->flags ){
    case st_dict_member_null:
        break;

    case st_dict_member_set:
        stObjRelease((stObj_t *)m->key);
        stObjLeave((stObj_t *)m->value);
        break;
        
    case st_dict_member_collision:
        stDict_FreeTable(m->nested);
        break;
            
    default:
        break;
    }
}

static void stDict_FreeTable(struct st_dict_table *table)
{
    int i;

    for(i=0; i<256; i++)
    {
        stDict_FreeMember(table->members + i);
    }

    free(table);
}

void stDictFinal(stDict_t *dict)
{
    int i;

    for(i=0; i<256; i++)
    {
        stDict_FreeMember(dict->root.members + i);
    }

    memset((uint8_t *)dict + sizeof(stObj_t), 0,
           sizeof(stDict_t) - sizeof(stObj_t));
    stObjFinalSuper(&dict->hdrObj);
}
