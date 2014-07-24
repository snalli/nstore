#ifndef OPT_WAL_ENGINE_H_
#define OPT_WAL_ENGINE_H_

#include <vector>
#include <string>
#include <thread>
#include <queue>
#include <sstream>

#include "engine.h"
#include "nstore.h"
#include "transaction.h"
#include "record.h"
#include "utils.h"
#include "workload.h"
#include "database.h"
#include "pthread.h"
#include "plist.h"

using namespace std;

class opt_wal_engine : public engine {
 public:
  opt_wal_engine(const config& _conf);
  ~opt_wal_engine();

  std::string select(const statement& st);
  void update(const statement& st);
  void insert(const statement& t);
  void remove(const statement& t);

  void generator(const workload& load, bool stats);
  void runner();
  void execute(const transaction& t);

  void recovery();
  void group_commit();

  //private:
  const config& conf;
  database* db;
  std::vector<std::thread> executors;

  plist<char*>* pm_log;
  std::hash<std::string> hash_fn;

  std::stringstream entry_stream;
  std::string entry_str;

  pthread_rwlock_t txn_queue_rwlock = PTHREAD_RWLOCK_INITIALIZER;
  std::queue<transaction> txn_queue;
  std::atomic<bool> done;

  std::vector<void*> commit_free_list;
  pthread_rwlock_t log_rwlock = PTHREAD_RWLOCK_INITIALIZER;

  std::atomic_bool ready;
  int looper = 0;
};

#endif /* OPT_WAL_ENGINE_H_ */