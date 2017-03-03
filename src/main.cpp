// TESTER

#include <cassert>

#include "config.h"
#include "utils.h"
#include "libpm.h"
#include "coordinator.h"

namespace storage {

  extern struct static_info *sp;  // global persistent memory structure
  int level = 2;  // verbosity level
  extern bool pm_stats;

  static void usage_exit(FILE *out) {
    fprintf(out, "Command line options : nstore <options> \n"
            "   -h --help              :  Print help message \n"
            "   -x --num-txns          :  Number of transactions \n"
            "   -k --num-keys          :  Number of keys \n"
            "   -e --num-executors     :  Number of executors \n"
            "   -f --fs-path           :  Path for FS \n"
            "   -g --gc-interval       :  Group commit interval \n"
            "   -a --wal-enable        :  WAL enable (traditional) \n"
            "   -w --opt-wal-enable    :  OPT WAL enable \n"
            "   -s --sp-enable         :  SP enable (traditional) \n"
            "   -c --opt-sp-enable     :  OPT SP enable \n"
            "   -m --lsm-enable        :  LSM enable (traditional) \n"
            "   -l --opt-lsm-enable    :  OPT LSM enable \n"
            "   -y --ycsb              :  YCSB benchmark \n"
            "   -t --tpcc              :  TPCC benchmark \n"
            "   -d --test              :  TEST benchmark \n"
            "   -p --per-writes        :  Percent of writes \n"
            "   -u --ycsb-update-one   :  Update one field \n"
            "   -q --ycsb_zipf_skew    :  Zipf Skew \n"
            "   -z --storage_stats     :  Collect storage stats \n"
            "   -o --tpcc_stock-level  :  TPCC stock level only \n"
            "   -r --recovery          :  Recovery mode \n"
            "   -b --load-batch-size   :  Load batch size \n"
            "   -j --test_b_mode       :  Test benchmark mode \n"
            "   -i --multi-executors   :  Multiple executors \n"
            "   -n --enable-trace      :  E[n]able trace [default:0]\n");
    exit(EXIT_FAILURE);
  }

  static struct option opts[] = {
    { "enable-trace", optional_argument, NULL, 'n' },
    { "log-enable", no_argument, NULL, 'a' },
    { "sp-enable", no_argument, NULL, 's' },
    { "lsm-enable", no_argument, NULL, 'm' },
    { "opt-wal-enable", no_argument, NULL, 'w' },
    { "opt-sp-enable", no_argument, NULL, 'c' },
    { "opt-lsm-enable", no_argument, NULL, 'l' },
    { "test", no_argument, NULL, 'd' },
    { "ycsb", no_argument, NULL, 'y' },
    { "tpcc", no_argument, NULL, 't' },
    { "fs-path", optional_argument, NULL, 'f' },
    { "num-txns", optional_argument, NULL, 'x' },
    { "num-keys", optional_argument, NULL, 'k' },
    { "num-executors", optional_argument, NULL, 'e' },
    { "ycsb_per_writes", optional_argument, NULL, 'w' },
    { "ycsb_skew", optional_argument, NULL, 'q' },
    { "gc-interval", optional_argument, NULL, 'g' },
    { "verbose", no_argument, NULL, 'v' },
    { "help", no_argument, NULL, 'h' },
    { "test-mode", optional_argument, NULL, 'j' },
    { "ycsb-update-one", no_argument, NULL, 'u' },
    { NULL, 0, NULL, 0 } };

  static void parse_arguments(int argc, char* argv[], config& state) {

    // Default Values
    mtm_enable_trace = 0;
    state.is_trace_enabled = 0;
    state.fs_path = std::string("/dev/shm/");

    state.num_keys = 10;
    state.num_txns = 10;

    state.single = false;
    state.num_executors = 8;

    state.verbose = false;

    state.gc_interval = 5;
    state.ycsb_per_writes = 0.1;

    state.merge_interval = 10000;
    state.merge_ratio = 0.05;

    state.etype = engine_type::WAL;
    state.btype = benchmark_type::YCSB;

    state.read_only = false;
    state.recovery = false;

    state.ycsb_skew = 0.1;
    state.ycsb_update_one = false;
    state.ycsb_field_size = 100;
    state.ycsb_tuples_per_txn = 1;
    state.ycsb_num_val_fields = 5;

    state.tpcc_num_warehouses = 2;
    state.tpcc_stock_level_only = false;

    state.active_txn_threshold = 10;
    state.load_batch_size = 100;
    state.storage_stats = false;

    state.test_benchmark_mode = 0;

    // Parse args
    int debug_fd = -1, ret = 0;
    while (1) {
      int idx = 0;
      int c = getopt_long(argc, argv, "n:f:x:k:e:p:g:q:b:j:svwascmhludytzori", opts,
                          &idx);

      if (c == -1)
        break;

      switch (c) {
      case 'n':
	state.is_trace_enabled = atoi(optarg);
	if(!state.is_trace_enabled)
		break;
	assert(debug_fd == -1);
	assert(trace_marker == -1);
	assert(tracing_on == -1);

	/* Turn off tracing from previous sessions */
	debug_fd = open("/sys/kernel/debug/tracing/tracing_on", O_WRONLY);
	if(debug_fd != -1){ ret = write(debug_fd, "0", 1); }
	else{ ret = 1; goto fail; }
	close(debug_fd);

	/* Emtpy trace buffer */
	debug_fd = open("/sys/kernel/debug/tracing/current_tracer", O_WRONLY);
	if(debug_fd != -1){ ret = write(debug_fd, "nop", 3); }
	else{ ret = 2; goto fail; }
	close(debug_fd);

	/* Pick a routine that EXISTS but will never be called, VVV IMP !*/
	debug_fd = open("/sys/kernel/debug/tracing/set_ftrace_filter", O_WRONLY);
	if(debug_fd != -1){ ret = write(debug_fd, "pmfs_mount", 10); }
	else{ ret = 3; goto fail; }
	close(debug_fd);

	/* Enable function tracer */
	debug_fd = open("/sys/kernel/debug/tracing/current_tracer", O_WRONLY);
	if(debug_fd != -1){ ret = write(debug_fd, "function", 8); }
	else{ ret = 4; goto fail; }
	close(debug_fd);

	trace_marker = open("/sys/kernel/debug/tracing/trace_marker", O_WRONLY);
	if(trace_marker == -1){ ret = 5; goto fail; }

	tracing_on = open("/sys/kernel/debug/tracing/tracing_on", O_WRONLY);
	if(tracing_on == -1){ ret = 6; goto fail; }

        std::cerr << "mtm_enable_trace: " << state.is_trace_enabled << std::endl;
        break;
fail:
	fprintf(stderr, "failed to open trace mechanism. need to be root.\n");
	exit(ret);

      case 'f':
        state.fs_path = std::string(optarg);
        std::cerr << "fs_path: " << state.fs_path << std::endl;
        break;
      case 'x':
        state.num_txns = atoi(optarg);
        std::cerr << "num_txns: " << state.num_txns << std::endl;
        break;
      case 'k':
        state.num_keys = atoi(optarg);
        std::cerr << "num_keys: " << state.num_keys << std::endl;
        break;
      case 'e':
        state.num_executors = atoi(optarg);
        std::cerr << "num_executors: " << state.num_executors << std::endl;
        break;
      case 'v':
        state.verbose = true;
        level = 3;
        break;
      case 'p':
        state.ycsb_per_writes = atof(optarg);
        assert(state.ycsb_per_writes >= 0 && state.ycsb_per_writes <= 1);

        if (state.ycsb_per_writes == 0)
          state.read_only = true;
        std::cerr << "per_writes: " << state.ycsb_per_writes << std::endl;
        break;
      case 'g':
        state.gc_interval = atoi(optarg);
        std::cerr << "gc_interval: " << state.gc_interval << std::endl;
        break;
      case 'a':
        state.etype = engine_type::WAL;
        std::cerr << "wal_enable: " << std::endl;
        break;
      case 'w':
        state.etype = engine_type::OPT_WAL;
        std::cerr << "opt_wal_enable " << std::endl;
        break;
      case 's':
        state.etype = engine_type::SP;
        std::cerr << "sp_enable  " << std::endl;
        break;
      case 'c':
        state.etype = engine_type::OPT_SP;
        std::cerr << "opt_sp_enable " << std::endl;
        break;
      case 'm':
        state.etype = engine_type::LSM;
        std::cerr << "lsm_enable " << std::endl;
        break;
      case 'l':
        state.etype = engine_type::OPT_LSM;
        std::cerr << "opt_lsm_enable " << std::endl;
        break;
      case 'd':
        state.btype = benchmark_type::TEST;
        std::cerr << "test_benchmark " << std::endl;
        break;
      case 'y':
        state.btype = benchmark_type::YCSB;
        std::cerr << "ycsb_benchmark " << std::endl;
        break;
      case 't':
        state.btype = benchmark_type::TPCC;
        std::cerr << "tpcc_benchmark " << std::endl;
        break;
      case 'j':
        state.test_benchmark_mode = atoi(optarg);
        std::cerr << "test_benchmark_mode: " << state.test_benchmark_mode << std::endl;
        break;
      case 'q':
        state.ycsb_skew = atof(optarg);
        std::cerr << "skew: " << state.ycsb_skew << std::endl;
        break;
      case 'u':
        state.ycsb_update_one = true;
        std::cerr << "ycsb_update_one " << std::endl;
        break;
      case 'z':
        state.storage_stats = true;
        std::cerr << "storage_stats " << std::endl;
        break;
      case 'o':
        state.tpcc_stock_level_only = true;
        std::cerr << "tpcc_stock_level " << std::endl;
        break;
      case 'r':
        state.recovery = true;
        std::cerr << "recovery " << std::endl;
        break;
      case 'b':
        state.load_batch_size = atoi(optarg);
        std::cerr << "load_batch_size: " << state.load_batch_size << std::endl;
        break;
      case 'i':
        state.single = false;
        state.num_executors = 2;
        std::cerr << "multiple executors " << std::endl;
        break;
      case 'h':
        usage_exit(stderr);
        break;
      default:
        fprintf(stderr, "\nUnknown option: -%c-\n", c);
        usage_exit(stderr);
      }
    }
  }

}

int main(int argc, char **argv) {
  /* Get to DAX FS */
  const char* path = "/dev/shm/zfile";

  #ifdef _ENABLE_TRACE 
  gettimeofday(&glb_time, NULL);
  glb_tv_sec  = glb_time.tv_sec;
  glb_tv_usec = glb_time.tv_usec;
  glb_start_time = glb_tv_sec * 1000000 + glb_tv_usec;

  pthread_spin_init(&tbuf_lock, PTHREAD_PROCESS_SHARED);
  /* tbuf = (char*)malloc(MAX_TBUF_SZ); To avoid interaction with M's hoard */
  tbuf = (char*)mmap(0, MAX_TBUF_SZ, PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  /* MAZ_TBUF_SZ influences how often we compress and hence the overall execution speed. */
  if(!tbuf) {
  	fprintf(m_err, "Failed to allocate trace buffer. Abort now.");
	die();
  }
  #endif
  pthread_spin_init(&tot_epoch_lock, PTHREAD_PROCESS_SHARED);

  size_t pmp_size = PMSIZE;
  if ((storage::pmp = storage::pmemalloc_init(path, pmp_size)) == NULL)
    std::cerr << "pmemalloc_init on :" << path << std::endl;

  storage::sp = (storage::static_info *) storage::pmemalloc_static_area();

// Start
  storage::config state;
  parse_arguments(argc, argv, state);
  state.sp = storage::sp;
  storage::coordinator cc(state);

  #ifdef _ENABLE_TRACE
  pthread_spin_lock(&tbuf_lock);
  if(tbuf_sz && mtm_enable_trace)
  {
  	tbuf_ptr = 0;
        while(tbuf_ptr < tbuf_sz)
        {
        	/*
                 * for some reason tbuf[tbuf] doesn't work
                 * although i thot tbuf + tbuf is same as
                 * tbuf[tbuf];
                 */
                 tbuf_ptr = tbuf_ptr + 1 + fprintf(m_out, "%s", tbuf + tbuf_ptr);
                 /* tbuf_ptr += TSTR_SZ; */
        }
        	tbuf_sz = 0;
  }
  pthread_spin_unlock(&tbuf_lock);
  pthread_spin_destroy(&tbuf_lock);
  #endif
  pthread_spin_destroy(&tot_epoch_lock);

  cc.eval(state);

  //std::cerr<<"STATS : "<<std::endl;
  //std::cerr<<"PCOMMIT : "<<storage::pcommit<<std::endl;
  //std::cerr<<"CLFLUSH : "<<storage::clflush<<std::endl;
  std::cerr << "TOTAL EPOCHS : " << get_tot_epoch_count();

  return 0;
}
