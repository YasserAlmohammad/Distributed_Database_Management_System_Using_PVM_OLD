
/* call DB methods to create the database, connect...

*/


#include "db.h"
#include "utils.h" 
#include "pvm3.h"
#include <iostream.h>



/* the best thing is to send error or success code 
   when sending error code engine sends error message too
   the Engine it self is an interface to the DB class(for database operations)
*/


void wlog(char*); //write to log file

int main(int argc,char* argv[]){

	wlog("**************** ENGINE BEGIN *****************\n");
	DBList list; //will hold maps of name-->database

	DB* db=NULL; //the active database
	DB* temp=NULL;

	//all frequentlry used strings are buffered, so we must use a copy
	//of them inside any method that use them

	char* tableName=new char[100];
	char* colName=new char[100];
	char* dataType=new char[100];
	char* cons=new char[100]; //constraint
	char* row=new char[500];
	char* filePath=new char[250];
	char* dbName=new char[100];
	char* namesBuff=new char[500];
	char* queryResults;
	char* query=new char[1000];

	int choice=0;
	int rowID=1;
	int success=0;
	int error=1;

	int parentID;
	int mytid;

	parentID=pvm_parent();
	mytid=pvm_mytid();


	//parent must be available or you can't cintinue
	if(parentID<0 && parentID!=PvmNoParent)
		return -1; //errror in pvm
	if(parentID==PvmNoParent)
		return -2; //no parent
	if(mytid<0)
		return -1; //error in pvm
	
	do{
		/* 	-this is the Engine
			-if we got time we'll enable UPDATE,MODIFY queries
			-multi data base management is available
			-all to_do commands in the bellow list is available and error free
		
		    a recieved message always has the form: msg1:int msg2..msgn:params
			the first message is integer value indicating the choice
			other messages are parameters
			*/
		
		pvm_recv(parentID,1); //blocking recieve from parent(cluster) for messages of tag 1
		pvm_upkint(&choice,1,1);

		switch(choice){
			case 0:  //exit
				list.clear();
				pvm_initsend(PvmDataDefault);
				pvm_pkint(&success,1,1);
				break;
			case 1: //connect database
				pvm_upkstr(dbName);
				db=list.findDB(dbName);
				if(db==NULL){
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&error,1,1);
					pvm_pkstr("database not found\n");
					break;
				}
				db->connectDB();
				pvm_initsend(PvmDataDefault);
				pvm_pkint(&success,1,1);
				//success code to notify the cluster
				break;
			case 2: //create new table
				pvm_upkstr(dbName);
				db=list.findDB(dbName);
				if(db==NULL){
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&error,1,1);
					pvm_pkstr("database not found\n");
					break;
				}
				if(!db->isConnected()){
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&error,1,1);
					pvm_pkstr("database is offline, connect first using 1\n");
					break;
				}
				pvm_upkstr(tableName);

				if(!db->createTable(tableName)){
					 pvm_initsend(PvmDataDefault);
					 pvm_pkint(&error,1,1);
					 pvm_pkstr(db->getErrCode());
					 break;
				}
				pvm_initsend(PvmDataDefault);
				pvm_pkint(&success,1,1);		
				break;
			case 3: //create column
				pvm_upkstr(dbName);
				db=list.findDB(dbName);
				if(db==NULL){
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&error,1,1);
					pvm_pkstr("database not found\n");
					break;
				}
				if(!db->isConnected()){
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&error,1,1);
					pvm_pkstr("database is offline, connect first using 1\n");
					break;
				}
				
				pvm_upkstr(tableName);
				pvm_upkstr(colName);
				pvm_upkstr(dataType);
				pvm_upkstr(cons);

				if(!db->createCol(tableName,colName,dataType,cons)){
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&error,1,1);
					pvm_pkstr(db->getErrCode());
					break;
				}
				pvm_initsend(PvmDataDefault);
				pvm_pkint(&success,1,1);	
				break;
			case 4: //insert row
				pvm_upkstr(dbName);
				db=list.findDB(dbName);
				if(db==NULL){
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&error,1,1);
					pvm_pkstr("database not found\n");
					break;
				}
				if(!db->isConnected()){
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&error,1,1);
					pvm_pkstr("database is offline, connect first using 1\n");							
					break;
				}

				pvm_upkstr(tableName);
				pvm_upkstr(row);				
				
				if(!db->insertRow(tableName,row)){
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&error,1,1);
					pvm_pkstr(db->getErrCode());					
					break;
				}
				pvm_initsend(PvmDataDefault);
				pvm_pkint(&success,1,1);					
				break;
			case 5: //drop table
				pvm_upkstr(dbName);
				db=list.findDB(dbName);
				if(db==NULL){
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&error,1,1);
					pvm_pkstr("database not found\n");					
					break;
				}
				if(!db->isConnected()){
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&error,1,1);
					pvm_pkstr("database is offline, connect first using 1\n");					
					break;
				}

				pvm_upkstr(tableName);
				if(!db->dropTable(tableName)){
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&error,1,1);
					pvm_pkstr(db->getErrCode());					
					break;
				}
				pvm_initsend(PvmDataDefault);
				pvm_pkint(&success,1,1);	
				break;
			case 6: //delete row
				pvm_upkstr(dbName);
				db=list.findDB(dbName);
				if(db==NULL){
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&error,1,1);
					pvm_pkstr("database not found\n");					
					break;
				}
				if(!db->isConnected()){
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&error,1,1);
					pvm_pkstr("database is offline, connect first using 1\n");					
					break;
				}

				pvm_upkstr(tableName);
				pvm_upkint(&rowID,1,1);
				if(!db->deleteRow(tableName,rowID)){
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&error,1,1);
					pvm_pkstr(db->getErrCode());					
					break;
				}
				pvm_initsend(PvmDataDefault);
				pvm_pkint(&success,1,1);	
				break;
			case 7: //view table names
				pvm_upkstr(dbName);
				db=list.findDB(dbName);
				if(db==NULL){
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&error,1,1);
					pvm_pkstr("database not found\n");					
					break;
				}
				if(!db->isConnected()){
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&error,1,1);
					pvm_pkstr("database is offline, connect first using 1\n");					
					break;
				}

				pvm_initsend(PvmDataDefault);
				pvm_pkint(&success,1,1);
				pvm_pkstr(db->getTableNames());				
				break;
			case 8: //view table definition
				pvm_upkstr(dbName);
				db=list.findDB(dbName);
				if(db==NULL){
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&error,1,1);	
					pvm_pkstr("database not found\n");					
					break;
				}
				if(!db->isConnected()){
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&error,1,1);
					pvm_pkstr("database is offline, connect first using 1\n");					
					break;
				}

				pvm_upkstr(tableName);
				pvm_initsend(PvmDataDefault);
				pvm_pkint(&success,1,1);
				pvm_pkstr(db->getTableDefinition(tableName));					
				break;
			case 9: //view table data
				pvm_upkstr(dbName);
				db=list.findDB(dbName);
				if(db==NULL){
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&error,1,1);
					pvm_pkstr("database not found\n");					
					break;
				}
				if(!db->isConnected()){
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&error,1,1);
					pvm_pkstr("database is offline, connect first using 1\n");					
					break;
				}

				pvm_upkstr(tableName);
				pvm_initsend(PvmDataDefault);
				pvm_pkint(&success,1,1);	
				pvm_pkstr(db->getTableData(tableName));				
				break;
			case 10: //create new database
				//first check if a database with the same name already exists
				pvm_upkstr(dbName);
				temp=list.findDB(dbName);
				
				if(temp!=NULL){
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&error,1,1);	
					pvm_pkstr("database with the same name already exists\n");									
					break;
				}
				pvm_upkstr(filePath);
				temp=new DB(filePath,dbName);
				
				if(temp->hasError()){
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&error,1,1);	
					pvm_pkstr("couldn't create database, check name or file path\n");					
					temp->~DB();
					break;
				}
				//add to map					
				list.insertDB(dbName,temp);
				pvm_initsend(PvmDataDefault);
				pvm_pkint(&success,1,1);	
				temp=NULL;
				break;
			case 11: //create database from existing file
				pvm_upkstr(filePath);
				temp=DB::createFromFile(filePath);
				if(temp==NULL){
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&error,1,1);
					pvm_pkstr("couldn't create database, check name or file path\n");						
					break;
				}
				
				//first check if a database with the same name already exists
				if(list.findDB(temp->getDBName())!=NULL){
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&error,1,1);	
					pvm_pkstr("database with the same name already exists\n");					
					delete temp;
					temp=NULL;
					break;
				}		
				//add to map
				list.insertDB(temp->getDBName(),temp);
				pvm_initsend(PvmDataDefault);
				pvm_pkint(&success,1,1);	
				pvm_pkstr(temp->getDBName()); //send name read from loaded file								
				temp=NULL;
				break;
			case 12: //delete existing database
				pvm_upkstr(dbName);
				temp=list.findDB(dbName);
				if(temp==NULL){					
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&error,1,1);		
					pvm_pkstr(strcat(dbName," doesn't exists\n"));					
					break; //always break after error
				}

				list.deleteDB(dbName);
				pvm_initsend(PvmDataDefault);
				pvm_pkint(&success,1,1);	
				break;
			case 13: //view all databases
				list.getNames(namesBuff);
				pvm_initsend(PvmDataDefault);
				pvm_pkint(&success,1,1);
				pvm_pkstr(namesBuff); //send names				
				break;
			case 14: //does a database exists
				pvm_upkstr(dbName);
				db=list.findDB(dbName);

				pvm_initsend(PvmDataDefault);
				if(db==NULL){
					pvm_pkint(&error,1,1);
					pvm_pkstr("database doesn't exists\n");
				}
				else
					pvm_pkint(&success,1,1);			
				
				break;
			case 15: //query
				pvm_upkstr(dbName);
				db=list.findDB(dbName);
				if(db==NULL){
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&error,1,1);
					pvm_pkstr("database not found\n");					
					break;
				}
				if(!db->isConnected()){
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&error,1,1);
					pvm_pkstr("database is offline, connect first using 1\n");					
					break;
				}
				pvm_upkstr(query);
				queryResults=db->query(query);
				if(queryResults==NULL){
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&error,1,1);
					pvm_pkstr(db->getErrCode());							
					break;
				}
				
				pvm_initsend(PvmDataDefault);
				pvm_pkint(&success,1,1);
				pvm_pkstr(queryResults);			
				break;
			case 18: //disconnect
				pvm_upkstr(dbName);
				db=list.findDB(dbName);
				if(db==NULL){
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&error,1,1);
					pvm_pkstr("database not found\n");
					break;
				}
				db->disConnectDB();
				pvm_initsend(PvmDataDefault);
				pvm_pkint(&success,1,1);
				//success code to notify the cluster
				break;
			default:
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&error,1,1);
					pvm_pkstr("unavailable choice coose another\n");					
					break;
		}
		pvm_send(parentID,1/*message tag*/);
	}while(choice!=0);

	delete [] tableName;
	delete [] row;
	delete [] dataType;
	delete [] cons;
	delete [] colName;
	delete []dbName;
	delete []filePath;
	delete []namesBuff;
	delete []query;

	wlog("**************** ENGINE END *****************\n");
	pvm_exit();
 
	return 0; 
}

void wlog(char* message){
	FILE* log=fopen("c:\\dblog.txt","a");
	if(log!=NULL){
		fputs(message,log);
		fclose(log);
	}
}
