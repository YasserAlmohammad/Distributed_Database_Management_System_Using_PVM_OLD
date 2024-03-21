#include <pvm3.h>

//return an allocated copy of a string
char* copy(char* str);
int contains(const char* line, char* word,char* sep=", \t\n");

void wlog(char*); //write to log file

class ClusterNode{
public:
	ClusterNode();
	ClusterNode(char* dbNames,int cid,ClusterNode* next);
	char* dbNames; //all databases in the cluster that has the id of cid
	int cid;
	ClusterNode* next;
};


// maps of: <all databases,ClusterID>
	/*	the list has the form bellow
		[dbName1]
		[dbName2]
		[...	]<----->[clusterID]
		[...	]
		[dbNamen]	^
					|
					|
					|
					|
		[dbName1]	V
		[dbName2]
		[...	]<----->[clusterID]
		[...	]
		[dbNamen]
	*/


class ClusterList{
public:
	ClusterList();
	~ClusterList();

	void insertC(char* name,int cid);
	int findC(char* dbName); //-1 if not found(gets a cluster id using a database name)
	void getDBNames(char* buffer); //get all databases in the network
	void getDBNames(int cid,char* buffer); //get databases in the specified cluster
	void refresh(); //update list
	int* gettids(); //returns array of cluster ids(especially for cast operation);
	int getCount(){return count;}
	void clear();
	
private:
	ClusterNode* head;
	int count;
	
};
