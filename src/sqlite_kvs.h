#ifndef __SQLITE_KVS_H__
#define __SQLITE_KVS_H__

#include <sqlite3.h>

#include <string>
#include <cassert>

template<typename T>
struct mapping_traits {};

template<>
struct mapping_traits<int64_t>{
    static int bind(sqlite3_stmt *stmt, const int &idx, const int64_t &val) {
        return sqlite3_bind_int64(stmt, idx, val);
    }

    static int64_t get_column(sqlite3_stmt *stmt, const int &idx) {
        return sqlite3_column_int64(stmt, idx);
    };
};

template<>
struct mapping_traits<std::string>{
	static int bind(sqlite3_stmt *stmt, const int &idx, const std::string &val) {
		return sqlite3_bind_text(stmt, idx, val.c_str(), val.length() + 1, SQLITE_TRANSIENT);
	}

	static std::string get_column(sqlite3_stmt *stmt, const int &idx) {
		return std::string{(const char *) sqlite3_column_text(stmt, idx)}; // nulls???
	}
};

template<typename K, typename V>
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

        void upsert(const K &key, const V &value){
            const int r_reset= sqlite3_reset(_upsert_sql);
            assert(r_reset == SQLITE_OK);

            const int r_key_bind = mapping_traits<K>::bind(_upsert_sql, 1, key);
            assert(r_key_bind == SQLITE_OK);

            const int r_value_bind = mapping_traits<V>::bind(_upsert_sql, 2, value);
            assert(r_value_bind == SQLITE_OK);

            const int r_exec = sqlite3_step(_upsert_sql);
            assert(r_exec == SQLITE_DONE);

            return;
        };

        V get(const K &key){
            const int r_reset = sqlite3_reset(_get_sql);
            assert(r_reset == SQLITE_OK);

            const int r_key_bind = mapping_traits<K>::bind(_get_sql, 1, key);
            assert(r_key_bind == SQLITE_OK);

            const int r_exec = sqlite3_step(_get_sql);
            if(r_exec == SQLITE_DONE){
                assert(false);
            }
            else{
                const V value = mapping_traits<V>::get_column(_get_sql, 0);
                return value;
            }

        };

    private:

        sqlite3 *_db;

        sqlite3_stmt *_upsert_sql;
        sqlite3_stmt *_get_sql;

};

#endif
