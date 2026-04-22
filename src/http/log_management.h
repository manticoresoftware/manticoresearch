
static nljson GetSystemTable()
{
    nljson tSystemTable = nljson::parse ( R"({})" );

tSystemTable["_xpack"] = nljson::parse ( R"(
{
  "license": {
    "type": "basic",
    "mode": "basic",
    "status": "active"
  },
  "features": {
    "analytics": {
      "available": false,
      "enabled": false
    },
    "ccr": {
      "available": false,
      "enabled": false
    },
    "data_frame": {
      "available": false,
      "enabled": false
    },
    "flattened": {
      "available": false,
      "enabled": true
    },
    "frozen_indices": {
      "available": false,
      "enabled": false
    },
    "graph": {
      "available": false,
      "enabled": false
    },
    "ilm": {
      "available": false,
      "enabled": false
    },
    "logstash": {
      "available": true,
      "enabled": true
    },
    "ml": {
      "available": false,
      "enabled": false
    },
    "monitoring": {
      "available": false,
      "enabled": false
    },
    "rollup": {
      "available": false,
      "enabled": false
    },
    "security": {
      "available": false,
      "enabled": false
    },
    "spatial": {
      "available": false,
      "enabled": false
    },
    "sql": {
      "available": true,
      "enabled": true
    },
    "vectors": {
      "available": false,
      "enabled": false
    },
    "voting_only": {
      "available": false,
      "enabled": false
    },
    "watcher": {
      "available": false,
      "enabled": false
    }
  },
  "tagline": "You know, for X"
}
)" );


tSystemTable["_cluster"] = nljson::parse ( R"(
{
  "settings" : {
  "persistent": {},
  "transient": {},
  "defaults": {}
  }
}
)" );  


tSystemTable["_nodes"] = nljson::parse ( R"(
{
}
)" );  


tSystemTable["master"] = nljson::parse ( R"(
{
}
)" );

tSystemTable["templates"] = nljson::parse ( R"({})" );

return tSystemTable;
}
