#include <sqlite3.h>

#include <string>
#include <cassert>

class Sqlite_KVS{

    public:

        Sqlite_KVS(const std::string &filename, const std::string &table){
            const int r_open = sqlite3_open(filename.c_str(), &_db);
            assert(r_open == 0);

			//Create a table to store our shit in
			[](sqlite3 *db, const std::string &table){
				const std::string query("CREATE TABLE IF NOT EXISTS " + table + "(key PRIMARY KEY, value)");
				sqlite3_stmt *compiled_query;

				const int r_prep = sqlite3_prepare(db, query.c_str(), query.size(), &compiled_query, nullptr);
				assert(r_prep == 0);

				const int r_exec = sqlite3_step(compiled_query);
				assert(r_exec == SQLITE_DONE);

				return;
			}(_db, table);

			_upsert_sql = [](sqlite3 *db, const std::string &table){
				const std::string query = "INSERT OR REPLACE INTO " + table + "(key, value) VALUES (?, ?)";
				sqlite3_stmt *compiled_query;

				const int prep_status = sqlite3_prepare(db, query.c_str(), query.size(), &compiled_query, nullptr);
				assert(prep_status == 0);

                return compiled_query;
			}(_db, table);

			_get_sql = [](sqlite3 *db, const std::string &table){
				const std::string query = "SELECT value FROM " + table + " WHERE key = ?";
				sqlite3_stmt *compiled_query;

				const int prep_status = sqlite3_prepare(db, query.c_str(), query.size(), &compiled_query, nullptr);
				assert(prep_status == 0);

                return compiled_query;
			}(_db, table);

        };

		~Sqlite_KVS(){
			sqlite3_finalize(_get_sql);
			sqlite3_finalize(_upsert_sql);
            sqlite3_close(_db);
            return;
		};

        void upsert(const std::string &key, const int64_t &value){
            const int r_reset= sqlite3_reset(_upsert_sql);
            assert(r_reset == SQLITE_OK);

            const int r_key_bind = sqlite3_bind_text(_upsert_sql, 1, key.c_str(), key.size(), SQLITE_STATIC);
            assert(r_key_bind == SQLITE_OK);

            const int r_value_bind = sqlite3_bind_int64(_upsert_sql, 2, value);
            assert(r_value_bind == SQLITE_OK);

            const int r_exec = sqlite3_step(_upsert_sql);
            assert(r_exec == SQLITE_DONE);

            return;
        };

        int64_t get(const std::string &key){
            const int r_reset = sqlite3_reset(_get_sql);
            assert(r_reset == SQLITE_OK);

            const int r_key_bind = sqlite3_bind_text(_get_sql, 1, key.c_str(), key.size(), SQLITE_STATIC);
            assert(r_key_bind == SQLITE_OK);

            const int r_exec = sqlite3_step(_get_sql);
            if(r_exec == SQLITE_DONE){
                assert(false);
            }
            else{
                const int64_t value = sqlite3_column_int64(_get_sql, 0);
                return value;
            }

        };

    private:

        sqlite3 *_db;

        sqlite3_stmt *_upsert_sql;
        sqlite3_stmt *_get_sql;

};
