#include "sphinx.hpp"

// *** MAIN ***

int main(int argc, char **argv)
{
	CSphIndex_VLN *idx;
	CSphSource_MySQL *src;
	CSphConfig conf;
	CSphHash *confCommon, *confIndexer;

	if (!(conf.open("sphinx.conf"))) {
		fprintf(stderr, "ERROR: unable to open '%s'\n", "sphinx.conf");
		return 1;
	}
	confCommon = conf.loadSection("common");
	confIndexer = conf.loadSection("indexer");

	#define CHECK_CONF(hash, key) \
		if (!hash->get(key)) { \
			fprintf(stderr, "ERROR: '%s' is not specified in config file\n", key); \
			return 1; \
		}

	CHECK_CONF(confIndexer, "sql_host");
	CHECK_CONF(confIndexer, "sql_user");
	CHECK_CONF(confIndexer, "sql_pass");
	CHECK_CONF(confIndexer, "sql_db");
	CHECK_CONF(confIndexer, "sql_query");
	CHECK_CONF(confCommon,  "index_path");

	src = new CSphSource_MySQL(confIndexer->get("sql_query"));
	if (!src->connect(
		confIndexer->get("sql_host"),
		confIndexer->get("sql_user"),
		confIndexer->get("sql_pass"),
		confIndexer->get("sql_db"),
		confIndexer->get("sql_port") ? atoi(confIndexer->get("sql_port")) : 3306,
		confIndexer->get("sql_sock")))
	{
		fprintf(stderr, "ERROR: unable to connect to MySQL\n");
		return 1;
	}
	idx = new CSphIndex_VLN(confCommon->get("index_path"));
	idx->build(new CSphDict_CRC32(), src);
	delete idx;
}
