#include "bson.h"
#include "mongoc.h"
#include "lmongo.h"
#include <unordered_map>
#include "iostream"
#include "memory"

namespace lmongoc {

#define getColByName(colName) \
	mongoc_collection_t *collection = getCollection(colName); \
	if (!collection) \
	{\
		printf("%s not found!\n", colName); \
		return 0;\
	}\
	

typedef std::unordered_map<std::string, mongoc_collection_t*> colmap;

static mongoc_client_t *client = NULL;
static colmap* collectionMap = NULL;

bool connect_mongo(const char* url)
{
	mongoc_uri_t *uri;
	bson_error_t error;
	bson_t *command, reply;
	bool retval;
	char *str;

	mongoc_init();
	//client = mongoc_client_new(url);
	
	uri = mongoc_uri_new_with_error(url, &error);	
	if (!uri)
	{
		fprintf(stderr, "failed to parse URI:%s,error msg:%s\n", url, error.message);
		return false;
	}
	client = mongoc_client_new_from_uri(uri);
	if (!client)
	{
		printf("mongo connect fail.%s\n",url);
		return false;
	}
	command = BCON_NEW("ping", BCON_INT32(1));
	retval = mongoc_client_command_simple(client, "admin", command, NULL, &reply, &error);
	if (!retval)
	{
		fprintf(stderr, "ping error:%s\n", error.message);
		return false;
	}
	str = bson_as_json(&reply, NULL);
	printf("%s\n", str);

	printf("mongo connected.%s\n",url);
	collectionMap = new colmap();
	return true;
}

static mongoc_collection_t* try_init_collection(const char* col)
{
	if (!client)
	{
		return NULL;
	}

	mongoc_collection_t *collection = mongoc_client_get_collection(client, "testMongo", col);
	if (!collection)
	{
		return NULL;
	}

	(*collectionMap)[col] = collection;
	printf("initialized col:%s\n", col);
	return collection;
}

static mongoc_collection_t* getCollection(const char* col)
{
	colmap::iterator it = collectionMap->find(col);
	if (it == collectionMap->end())
	{
		return try_init_collection(col);
	}
	return it->second;
}

bool insert()
{
	mongoc_collection_t *collection;
	bson_error_t error;
	bson_oid_t oid;
	bson_t *doc;

	collection = getCollection("testcol");

	doc = bson_new();
	bson_oid_init(&oid, NULL);
	BSON_APPEND_OID(doc, "_id", &oid);
	BSON_APPEND_UTF8(doc, "hello", "world");

	if (!mongoc_collection_insert_one(collection, doc, NULL, NULL, &error))
	{
		fprintf(stderr, "%s\n", error.message);
	}

	bson_destroy(doc);
	return true;
}

void find()
{
	mongoc_collection_t *collection;	
	mongoc_cursor_t *cursor;
	const bson_t *doc;
	bson_t *query;
	char *str;

	collection = getCollection("userCol");
	query = bson_new();
	cursor = mongoc_collection_find_with_opts(collection, query, NULL, NULL);

	while (mongoc_cursor_next(cursor, &doc))
	{
		str = bson_as_canonical_extended_json(doc, NULL);
		printf("%s\n", str);
		bson_free(str);
	}

	bson_destroy(query);
	mongoc_cursor_destroy(cursor);
	printf("find end=======\n");
}

void deleting()
{
	mongoc_collection_t *collection;	
	bson_error_t error;	
	bson_t *doc;

	doc = bson_new();
	BSON_APPEND_UTF8(doc, "hello", "world33");

	collection = getCollection("testcol");
	if (!mongoc_collection_delete_one(collection, doc, NULL, NULL, &error))
	{
		fprintf(stderr, "Delete failed:%s\n", error.message);
	}
	bson_destroy(doc);
}

void update()
{
	mongoc_collection_t *collection;	
	bson_error_t error;	
	bson_t *query = BCON_NEW("hello", "world22");
	bson_t *update = BCON_NEW("$set", "{", "hello", BCON_UTF8("world33"), "}");
	
	collection = getCollection("testcol");
	if (!mongoc_collection_update_one(collection, query, update, NULL, NULL, &error))
	{
		fprintf(stderr, "%s\n", error.message);
	}

	bson_destroy (query);
	bson_destroy (update);
}

void executing()
{
	mongoc_collection_t *collection;	
	bson_error_t error;
	bson_t *command;
	bson_t reply;
	char *str;

	collection = getCollection("testcol");

	command = BCON_NEW ("collStats", BCON_UTF8 ("testcol"));
	if (mongoc_collection_command_simple (
				collection, command, NULL, &reply, &error)) {
		str = bson_as_canonical_extended_json (&reply, NULL);
		printf ("%s\n", str);
		bson_free (str);
	} else {
		fprintf (stderr, "Failed to run command: %s\n", error.message);
	}

	bson_destroy (command);
	bson_destroy (&reply);
}

void testDoc()
{
	mongoc_collection_t *collection;	
	bson_t bcon;	
	bson_t child;	
	bson_t father;

	bson_init (&bcon);
	bson_init (&child);
	bson_init (&father);
	bson_error_t error;

	BSON_ASSERT (bson_append_document_begin(&father, "father", -1, &bcon));

	//bson_append_utf8 (&child, "valueKey", -1, "value", -1);
	//bson_append_document (&bcon, "dockey", -1, &child);

	BSON_ASSERT (bson_append_document_end(&father, &bcon));	

	collection = getCollection("testcol");
	if (!mongoc_collection_insert_one(collection, &father, NULL, NULL, &error))
	{
		fprintf(stderr, "%s\n", error.message);
	}

	bson_destroy (&bcon);
	bson_destroy (&child);
}

void createDoc()
{
	struct tm   born = { 0 };
	struct tm   died = { 0 };
	const char *lang_names[] = {"MATH-MATIC", "FLOW-MATIC", "COBOL"};
	const char *schools[] = {"Vassar", "Yale"};
	const char *degrees[] = {"BA", "PhD"};
	uint32_t    i;
	char        buf[16];
	const       char *key;
	size_t      keylen;
	bson_t     *document;
	bson_t      child;
	bson_t      child2;
	char       *str;

	document = bson_new ();

	/*
	 *     * Append { "born" : ISODate("1906-12-09") } to the document.
	 *         * Passing -1 for the length argument tells libbson to calculate the string length.
	 *             */
	born.tm_year = 6;  /* years are 1900-based */
	born.tm_mon = 11;  /* months are 0-based */
	born.tm_mday = 9;
	bson_append_date_time (document, "born", -1, mktime (&born) * 1000);

	/*
	 *     * Append { "died" : ISODate("1992-01-01") } to the document.
	 *         */
	died.tm_year = 92;
	died.tm_mon = 0;
	died.tm_mday = 1;

	/*
	 *     * For convenience, this macro passes length -1 by default.
	 *         */
	BSON_APPEND_DATE_TIME (document, "died", mktime (&died) * 1000);

	/*
	 *     * Append a subdocument.
	 *         */
	BSON_APPEND_DOCUMENT_BEGIN (document, "name", &child);
	BSON_APPEND_UTF8 (&child, "first", "Grace");
	BSON_APPEND_UTF8 (&child, "last", "Hopper");
	bson_append_document_end (document, &child);

	/*
	 *     * Append array of strings. Generate keys "0", "1", "2".
	 *         */
	BSON_APPEND_ARRAY_BEGIN (document, "languages", &child);
	for (i = 0; i < sizeof lang_names / sizeof (char *); ++i) {
		keylen = bson_uint32_to_string (i, &key, buf, sizeof buf);
		bson_append_utf8 (&child, key, (int) keylen, lang_names[i], -1);
	}
	bson_append_array_end (document, &child);

	/*
	 *     * Array of subdocuments:
	 *         *    degrees: [ { degree: "BA", school: "Vassar" }, ... ]
	 *             */
	BSON_APPEND_ARRAY_BEGIN (document, "degrees", &child);
	for (i = 0; i < sizeof degrees / sizeof (char *); ++i) {
		keylen = bson_uint32_to_string (i, &key, buf, sizeof buf);
		bson_append_document_begin (&child, key, (int) keylen, &child2);
		BSON_APPEND_UTF8 (&child2, "degree", degrees[i]);
		BSON_APPEND_UTF8 (&child2, "school", schools[i]);
		bson_append_document_end (&child, &child2);
	}
	bson_append_array_end (document, &child);

	/*
	 *     * Print the document as a JSON string.
	 *         */
	str = bson_as_canonical_extended_json (document, NULL);
	printf ("%s\n", str);
	bson_free (str);

	/*
	 *     * Clean up allocated bson documents.
	 *         */
	bson_destroy (document);
}

void release()
{
	for (colmap::iterator it = collectionMap->begin(); it != collectionMap->end(); it++)
	{
		mongoc_collection_destroy(it->second);
	}	
	collectionMap->clear();
	delete collectionMap;

	if (client)
	{
		mongoc_client_destroy(client);
		printf("destroy mongo client.\n");
	}

	mongoc_cleanup();
	printf("mongoc cleanup.\n");
}

static bool do_append(lua_State*L, bson_t *child)
{
	lua_pushnil(L);
	while (lua_next(L, -2) != 0)
	{
		int vt = lua_type(L, -1);		
		int kt = lua_type(L, -2);

		if (kt == LUA_TNUMBER)
		{
			char kstr[LUAI_MAXNUMBER2STR+1] = {};
			kstr[0] = '@';
			int num = lua_tonumber(L, -2);
			sprintf(kstr+1, "%d", num);

			if (vt == LUA_TTABLE)
			{
				bson_t doc;
				bson_append_document_begin(child, kstr, -1, &doc);
				do_append(L, &doc);
				bson_append_document_end(child, &doc);
				bson_destroy (&doc);
			}	
			else
			{
				switch (vt)
				{
					case LUA_TSTRING:
						bson_append_utf8(child, kstr, -1, lua_tostring(L, -1), -1);
						break;
					case LUA_TNUMBER:
						bson_append_double(child, kstr, -1, lua_tonumber(L, -1));
						break;
					case LUA_TBOOLEAN:
						bson_append_bool(child, kstr, -1, lua_toboolean(L, -1));
						break;
					default:
						printf("do_append value error! %d\n", vt);
						lua_pop(L, 1);
						return false;
				}	
			}

		}
		else if (kt == LUA_TSTRING)
		{
			if (vt == LUA_TTABLE)
			{
				bson_t doc;
				bson_append_document_begin(child, lua_tostring(L, -2), -1, &doc);
				do_append(L, &doc);
				bson_append_document_end(child, &doc);
				bson_destroy (&doc);
			}	
			else
			{
				switch (vt)
				{
					case LUA_TSTRING:
						bson_append_utf8(child, lua_tostring(L, -2), -1, lua_tostring(L, -1), -1);
						break;
					case LUA_TNUMBER:
						bson_append_double(child, lua_tostring(L, -2), -1, lua_tonumber(L, -1));
						break;
					case LUA_TBOOLEAN:
						bson_append_bool(child, lua_tostring(L, -2), -1, lua_toboolean(L, -1));
						break;
					default:
						printf("do_append2 value error! %d\n", vt);
						bson_append_utf8(child, lua_tostring(L, -2), -1, "error", -1);
						break;
				}	
			}
		}
		else
		{
			printf("do_append key error! %d\n", kt);
			lua_pop(L, 1);
			return false;
		}
		lua_pop(L, 1);
	}
	
	return true;
}

static bool deep_append(lua_State*L, bson_t *child, int pos)
{
	if (lua_type(L, pos) != LUA_TTABLE)
	{
		printf("deep append error!\n");
		return false;
	}

	lua_pushvalue(L, pos);
	bool ret = do_append(L, child);
	lua_pop(L, 1);
	return ret;
}

static int insert_col(lua_State *L) 
{
	const char* colName = lua_tostring(L, 1);
	if (!colName)
	{
		lua_pushboolean(L, false);
		return 1;
	}
	getColByName(colName);	

	if (lua_type(L, 2) != LUA_TTABLE)
	{
		lua_pushboolean(L, false);
		return 1;
	}

	bson_error_t error;
	mongoc_bulk_operation_t *bulk;
	bson_t *query;

	/* Remove everything */
	bulk = mongoc_collection_create_bulk_operation_with_opts(collection, NULL);
	query = bson_new();
	mongoc_bulk_operation_remove(bulk, query);	
	bson_destroy(query);
	

	// top is 2.
	/* table is in the stack at index 't' */
	lua_pushnil(L);  /* first key */
	while (lua_next(L, -2) != 0) {

		/* uses 'key' (at index -2) and 'value' (at index -1) */
		// printf("%s - %s\n",lua_typename(L, lua_type(L, -2)),lua_typename(L, lua_type(L, -1)));
		/* removes 'value'; keeps 'key' for next iteration */

		bson_t doc;
		bson_init (&doc);

		int kt = lua_type(L, -2);
		if (kt == LUA_TSTRING)
		{
			bson_append_utf8 (&doc, "_id", -1, lua_tostring(L, -2), -1);
		}
		else if (kt == LUA_TNUMBER)
		{
			bson_append_double (&doc, "_id", -1, lua_tonumber(L, -2));
		}
		else
		{
			printf("key type not found! %d\n", kt);
			lua_pop(L, 1);
			bson_destroy(&doc);
			continue;
		}

		int vt = lua_type(L, -1);
		if (vt == LUA_TTABLE)
		{
			bson_t child;
			bson_init(&child);
			if (deep_append(L, &child, -1))
			{
				bson_append_document(&doc, "dat", -1, &child);
				bson_destroy(&child);
			}
			else
			{
				printf("table value error! %d\n", vt);
				bson_destroy(&doc);
				return 0;	
			}
		}
		else if (vt == LUA_TSTRING)
		{
			bson_append_utf8 (&doc, "dat", -1, lua_tostring(L, -1), -1);
		}
		else if (vt == LUA_TNUMBER)
		{
			bson_append_double (&doc, "dat", -1, lua_tonumber(L, -1));
		}
		else if (vt == LUA_TBOOLEAN)
		{
			bson_append_bool (&doc, "dat", -1, lua_toboolean(L, -1));
		}
		else
		{
			printf("value type not found! %d\n", vt);
			bson_destroy(&doc);
			return 0;	
		}
		mongoc_bulk_operation_insert(bulk, &doc);	
		bson_destroy(&doc);
		lua_pop(L, 1);
	}	
	// top is 2. 每次lua_next会先pop 1
	
	if (!mongoc_bulk_operation_execute (bulk, NULL, &error))
	{
		mongoc_bulk_operation_destroy (bulk);
		printf("insert Error: %s\n", error.message);
		return 0;
	}

	mongoc_bulk_operation_destroy (bulk);
	lua_pushboolean(L, true);
	return 1;
}

static void iter_bson(lua_State* L, bson_iter_t* it)
{
	lua_newtable(L);
	while (bson_iter_next(it))
	{
		const char* key = bson_iter_key(it);
		if (key[0] == '@')
		{
			int num = strtol(key+1, NULL, 10);	
			lua_pushnumber(L, num);
		}
		else
		{
			lua_pushstring(L, key);
		}

		bson_type_t vt = bson_iter_type(it);	
		if (vt == BSON_TYPE_DOCUMENT)
		{
			bson_iter_t sub_it;
			if (bson_iter_recurse (it, &sub_it))
			{
				iter_bson(L, &sub_it);	
			}
			else
			{
				printf("sub it error!\n");
				return;
			}
		}
		else if (vt == BSON_TYPE_UTF8)
		{
			uint32_t value_len = 0;
			const char* value_str = bson_iter_utf8 (it, &value_len);
			lua_pushlstring(L, value_str, value_len);
		}
		else
		{
			switch (vt)
			{
				case BSON_TYPE_DOUBLE:
					lua_pushnumber(L, bson_iter_double (it));
					break;
				case BSON_TYPE_BOOL:
					lua_pushboolean(L, bson_iter_bool (it));
					break;
				default:
					lua_pushstring(L, "error");
					printf("find Col value error! %d\n", vt);
					break;
			}
		}	
		lua_rawset(L, -3);
	}
}

static int find_col(lua_State *L) 
{
	const char* colName = lua_tostring(L, 1);
	if (!colName)
	{
		return 0;
	}
	getColByName(colName);	

	mongoc_cursor_t *cursor;
	bson_t *query;
	const bson_t *doc;

	lua_newtable(L);
	query = bson_new();
	cursor = mongoc_collection_find_with_opts(collection, query, NULL, NULL);
	while (mongoc_cursor_next(cursor, &doc))
	{
		bson_iter_t it;	
		bson_iter_init(&it, doc);	
		iter_bson(L, &it); // {_id=xxx, dat={}}

		lua_pushstring(L, "_id");	
		lua_rawget(L, -2);
		lua_insert(L, -2);

		lua_pushstring(L, "dat");
		lua_rawget(L, -2);
		lua_insert(L, -2);

		lua_pop(L, 1);
		lua_rawset(L, -3);
	}

	bson_destroy(query);
	mongoc_cursor_destroy(cursor);
	lua_pushboolean(L, true);
	return 2;
}

const luaL_reg lualib[] =
{
	{"insertCol", insert_col},
	{"findCol", find_col},
	{NULL, NULL},
};

void luaopen_mongoc(lua_State* L)
{
	luaL_register(L, "lmongoc", lualib);	
}


}
