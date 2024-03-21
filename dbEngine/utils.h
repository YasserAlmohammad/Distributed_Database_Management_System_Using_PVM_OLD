
#include "db.h"

//return an allocated copy of a string
char* copy(char* str);

class DBNode{
public:
	DBNode();
	DBNode(char* name,DB* val,DBNode* next);
	char* name;
	DB* dataBase;
	DBNode* next;
};


//  to hold maps of the form <databaseName,databaseObject>

class DBList{
public:
	DBList(){head=NULL;}
	~DBList();

	void insertDB(char* name,DB* dataBase);
	bool deleteDB(char* name);
	DB* findDB(char* name); //NULL if not found
	void getNames(char* buffer);
	void clear();
	
private:
	DBNode* head;
	
};
