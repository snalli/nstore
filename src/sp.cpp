#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <functional>
#include <algorithm>
#include <sstream>
#include <utility>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <unordered_map>
#include <memory>

#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include <chrono>
#include <random>

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>

using namespace std;

#define NUM_KEYS 10
#define NUM_TXNS 10

#define VALUE_SIZE 4

#define MASTER_LOC 0x01a00000
#define TABLE_LOC  0x01b00000
#define DIR1_LOC   0x01c00000
#define DIR2_LOC   0x01d00000

int log_enable = 0 ;

// UTILS
std::string random_string( size_t length ){
    auto randchar = []() -> char
    {
        const char charset[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[ rand() % max_index ];
    };
    std::string str(length,0);
    std::generate_n( str.begin(), length, randchar );
    return str;
}

// TRANSACTION

class txn{
    public:
        txn(unsigned long _txn_id, std::string _txn_type, unsigned int _key, std::string _value) :
            txn_id(_txn_id),
            txn_type(_txn_type),
            key(_key),
            value(_value)
    {
        //start = std::chrono::system_clock::now();
    }

        //private:
        unsigned long txn_id;
        std::string txn_type;

        unsigned int key;
        std::string value;

        std::chrono::time_point<std::chrono::system_clock> start, end;
};

// TUPLE + TABLE + INDEX

class record{
    public:
        record(unsigned int _key, std::string _value, char* _location) :
            key(_key),
            value(_value),
            location(_location){}

        friend ostream& operator<<(ostream& out, const record& rec){
            out << "|" << rec.key << "|" << rec.value << "|";
            return out;
        }

        friend istream& operator>>(istream& in, record& rec){
            in.ignore(1); // skip delimiter

            in >> rec.key;
            in.ignore(1); // skip delimiter
            in >> rec.value;
            in.ignore(1); // skip delimiter
            //in.ignore(1); // skip endline

            return in;
        }


        //private:
        unsigned int key;
        std::string value;
        char* location;
};

boost::shared_mutex table_access;

unordered_map<unsigned int, record*> table_index;

// MMAP
class mmap_fd{
	public:
		//mmap_fd();

		mmap_fd(std::string table_name) :
			name(table_name)
		{
			if ((fp = fopen(name.c_str(), "w+")) == NULL) {
				cout<<"fopen failed "<<name<<" \n";
				exit(EXIT_FAILURE);
			}

			fd = fileno(fp);

			cout<<"table fd :"<<fd << endl;

			struct stat sbuf;

		    if (stat(name.c_str(), &sbuf) == -1) {
				cout<<"stat failed "<<name<<" \n";
		        exit(EXIT_FAILURE);
		    }

		    cout<<"size :"<< sbuf.st_size << endl;

		    caddr_t location = (caddr_t) TABLE_LOC;

		    // new file check
		    if(sbuf.st_size == 0){
			    off_t offset = 0;
			    off_t len = 1024*1024;

		    	if(fallocate(fd, 0, offset, len) == -1){
					cout<<"fallocate failed "<<name<<" \n";
			        exit(EXIT_FAILURE);
		    	}

		    	if (stat(name.c_str(), &sbuf) == -1) {
		    		cout << "stat failed " << name << " \n";
		    		exit(EXIT_FAILURE);
		    	}

			    cout<<"after fallocate: size :"<< sbuf.st_size << endl;

		    	offset = 0;
		    }

		    // XXX Fix -- scan max pointer from clean dir
	    	offset = 0;

		    if ((data = (char*) mmap(location, sbuf.st_size, PROT_WRITE, MAP_SHARED, fd, 0)) == (caddr_t)(-1)) {
		    	perror(" mmap_error ");
				cout<<"mmap failed "<<name<<endl;
		        exit(EXIT_FAILURE);
		    }

		    cout<<"data :"<< data << endl;
		}

	char* push_back_record (const record& rec){
        stringstream rec_stream;
        string rec_str;

        rec_stream << rec.key << rec.value;
        rec_str = rec_stream.str();

        char* cur_offset = (data + offset);
		memcpy(cur_offset, rec_str.c_str(), rec_str.size());

		offset += rec_str.size();

		return cur_offset;
	}

	void push_back_dir_entry(const record& rec){
		stringstream rec_stream;
		string rec_str;

		rec_stream << rec.key << static_cast<void *>(rec.location);
		rec_str = rec_stream.str();

		char* cur_offset = (data + offset);
		memcpy(cur_offset, rec_str.c_str(), rec_str.size());

		offset += rec_str.size();
	}

	void sync(){
		int ret = 0;

		cout<<"data: "<<data<<" offset: "<<offset<<endl;

		ret = msync(data, offset, MS_SYNC);
		if(ret == -1){
			perror("msync failed");
	        exit(EXIT_FAILURE);
		}

		cout<<"msync table"<<endl;
	}

	//private:
		FILE* fp = NULL;
		int fd;
		std::string name;
		char* data;
		off_t offset;

};

mmap_fd table("usertable");


// MASTER

class master{
	public:

	master(std::string table_name) :
		name(table_name),
		fd_dir1(name + "_dir1"),
		fd_dir2(name + "_dir2")
	{
		fd_dir_master = &fd_dir1;
	}

	mmap_fd* dirty(){
		return fd_dir_master;
	}

	void toggle(){
		if(fd_dir_master == &fd_dir1){
			fd_dir_master = &fd_dir2;
		}
		else{
			fd_dir_master = &fd_dir1;
		}
	}

	//private:
	std::string name;
	mmap_fd fd_dir1;
	mmap_fd fd_dir2;
	mmap_fd* fd_dir_master;
};

master m("usertable");

// LOGGING

class entry{
    public:
        entry(txn _txn, record* _before_image, record* _after_image) :
            transaction(_txn),
            before_image(_before_image),
            after_image(_after_image){}

        //private:
        txn transaction;
        record* before_image;
        record* after_image;
};


class logger {
    public:

        void push(entry e){
            boost::upgrade_lock< boost::shared_mutex > lock(log_access);
            boost::upgrade_to_unique_lock< boost::shared_mutex > uniqueLock(lock);
            // exclusive access

            log_queue.push_back(e);
        }

        void clear(){
        	log_queue.clear();
        }

    private:

        boost::shared_mutex log_access;
        vector<entry> log_queue;
};

logger _undo_buffer;

// GROUP COMMIT

void logger_func(const boost::system::error_code& /*e*/, boost::asio::deadline_timer* t){
    std::cout << "Syncing log !\n"<<endl;

    // sync table
    table.sync();

    t->expires_at(t->expires_at() + boost::posix_time::milliseconds(100));
    t->async_wait(boost::bind(logger_func, boost::asio::placeholders::error, t));
}

void group_commit(){
    if(log_enable){
        boost::asio::io_service io;
        boost::asio::deadline_timer t(io, boost::posix_time::milliseconds(100));

        t.async_wait(boost::bind(logger_func, boost::asio::placeholders::error, &t));
        io.run();
    }
}

// TRANSACTION OPERATIONS

int update(txn t){
    int key = t.key;

    if(table_index.count(t.key) == 0) // key does not exist
        return -1;

    record* before_image;
    record* after_image = new record(t.key, t.value, NULL);

    {
        boost::upgrade_lock< boost::shared_mutex > lock(table_access);
        // shared access
        before_image = table_index.at(t.key);

        boost::upgrade_to_unique_lock< boost::shared_mutex > uniqueLock(lock);
        // exclusive access

        //table.push_back(*after_image);
        table_index[key] = after_image;
    }

    // Add log entry
    entry e(t, before_image, after_image);
    _undo_buffer.push(e);

    return 0;
}

std::string read(txn t){
    int key = t.key;
    if (table_index.count(t.key) == 0) // key does not exist
        return "";

    std::string val = "" ;

    {
        boost::upgrade_lock<boost::shared_mutex> lock(table_access);
        // shared access

        record* r = table_index[key];
        if(r != NULL){
            val = r->value;
        }
    }

    return val;
}

// RUNNER + LOADER

int num_threads = 4;

long num_keys = NUM_KEYS ;
long num_txn  = NUM_TXNS ;
long num_wr   = 50 ;

void runner(){
    std::string val;

    for(int i=0 ; i<num_txn ; i++){
        long r = rand();
        std::string val = random_string(VALUE_SIZE);
        long key = r%num_keys;

        if(r % 100 < num_wr){
            txn t(i, "Update", key, val);
            update(t);
        }
        else{
            txn t(i, "Read", key, val);
            val = read(t);
        }
    }

}

void check(){

    //for (std::vector<record>::iterator it = table.begin() ; it != table.end(); ++it){
    //    cout << *it << endl;
    //}

}

void load(){
    size_t ret;
    stringstream buffer_stream;
    string buffer;
    char* tuple_ptr;

    for(int i=0 ; i<num_keys ; i++){
        int key = i;
        string value = random_string(VALUE_SIZE);
        record* after_image = new record(key, value, NULL);

        {
            boost::upgrade_lock<boost::shared_mutex> lock(table_access);
            boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);
            // exclusive access

            tuple_ptr = table.push_back_record(*after_image);
            after_image->location = tuple_ptr;
            m.dirty()->push_back_dir_entry(*after_image);

            table_index[key] = after_image;
        }

        // Add log entry
        txn t(0, "Insert", key, value);

        entry e(t, NULL, after_image);
        _undo_buffer.push(e);
    }

    table.sync();
    m.dirty()->sync();
    m.toggle();

}

int main(){
    std::chrono::time_point<std::chrono::system_clock> start, end;

    // Loader
    load();
    std::cout<<"Loading finished "<<endl;

    boost::thread group_committer(group_commit);

    // Runner
    /*
    boost::thread_group th_group;
    for(int i=0 ; i<num_threads ; i++)
        th_group.create_thread(boost::bind(runner));

    th_group.join_all();
    */

    //check();


    return 0;
}
