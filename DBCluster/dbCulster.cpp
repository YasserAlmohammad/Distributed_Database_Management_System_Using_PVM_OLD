
/*  call DB methods to create the database, connect...
    cluster waits for any message from any source(client or cluster is what we are interested in)
	when a message arrives from other clusters (or the client) it asks for the source tid,
	proccess the message(either internally or by passing it down to the engine), then send response back
	it communicates freely with other clusters to process data externally without user knowlege so all
	databases in other clusters are also available(through clusters only)
	unlimited clusters can be running on other computers(aslo on the same computer using simulator only)
	- the cluster can be generated either by a switcher or the simulator.
	- the client program doen't activate clusters, if no cluster is found on the network the client exits
	  only switchers can activate clutsers.
	- when dealing with engine the program is two way message sender:
	  [[  sender(cluster|client)<----> dbCluster <---->dbEngine  ]]
	- a cluster connected to a client must direct client calls to the right cluster or to his engine.
	- only one database is active at a moment for the client so he can make all operations on it
	  by default.
	- a special map is created to hold maps of the form: <database names, clusterID> so a call to 
	  a database is searched in the map to get the cluster it runs on, this is faster, the map represents 
	  a cached list of cluster and database maps on the whole network, each cluster has his own list
	  this map is refreshed(updated) each time new database is created, a new cluster is added
	  to the network(code 17), when viewing databases, or with a direct call to refresh
	- if database operated on belongs to this cluster, calls are directed to engine else to the owner
	  node cluster.
    - a cluster keeps runing even if the client program was not runing
   
	   
*/

#include <iostream.h>
#include "utils.h"



int main(int argc,char* argv[]){

	wlog("********** CLUSTER BEGIN ************\n");
	ClusterList list; //contains info about others no you
	/*	the list has the form bellow
		[dbName1]
		[dbName2]
		[...	]<----->[clusterID]
		[...	]
		[dbNamen]	A
					|
					|
					.
					.
					|
					|
					V
		[dbName1]
		[dbName2]
		[...	]<----->[clusterID]
		[...	]
		[dbNamen]

		so when a message come for using the a database or a new database we reference this list
		and refresh when neccessary or when updating
	*/
	
	//first spawn the Engine
	int engineID;
	int c=pvm_spawn("dbEngine",(char**)0,0,"",1,&engineID);
	if(c!=1){
		wlog("couldn't spawn engine\n");
		cin>>engineID;
		return -1;
	}

	if(engineID<0) //not supposed to happen
		return -1;
	
	
	char* tableName=new char[100];
	char* colName=new char[100];
	char* dataType=new char[100];
	char* cons=new char[100]; //constraint
	char* row=new char[500];
	char* filePath=new char[250];
	char* dbName=new char[100];
	char* namesBuff=new char[500];
	char* query=new char[1000];
	char* errorCode=new char[100];
	char* buffer=new char[50000]; //5KB genereal buffer for data retrieval like a * query

	int choice=0;
	int rowID=1;
	int answer=-1; //0 success, 1 error
	int msgTag=1; //always 1 from our programs
	int source=-1; //the sender tid to send response back to him
	int success=0;
	int error=1;
	int temp=-1;
	int mytid=pvm_mytid();
	int UpdateMessage=17;



	do{

		int buffID=pvm_recv(-1,msgTag); //from any source
		pvm_bufinfo(buffID,(int*)0,&msgTag,&source);

		pvm_upkint(&choice,1,1);

		/*(asynchronized messageing with engine only)
		  after each send operation a bloching recieve is waiting the answer from the engine
		  if the answer is 1(failure) then a string parameter passing the error reason is recieved too
		*/
		sprintf(dbName,"cluster choice:%d\n",choice);
		wlog(dbName);

		switch(choice){
			case 0: //exit
				pvm_initsend(PvmDataDefault); //to engine
				pvm_pkint(&choice,1,1);
				pvm_send(engineID,1);

				pvm_recv(engineID,1);
				pvm_upkint(&answer,1,1);

				if(answer==error) //if we couldn't stop engine this way we'll kill it
					pvm_kill(engineID);				
				
				pvm_initsend(PvmDataDefault); //to source
				pvm_pkint(&success,1,1);
				pvm_send(source,1);
				break;
			case 1: //connect a database
				pvm_upkstr(dbName);
				temp=list.findC(dbName); //find it's node
				if(temp<0){ 
					//to the engine					
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&choice,1,1); //it will be deleted after deleteing the active database from engine
					pvm_pkstr(dbName);
					pvm_send(engineID,1); //to engine
					pvm_recv(engineID,1);
				}
				else{ //to another cluster
					//send the message to the owner cluster 
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&choice,1,1);
					pvm_pkstr(dbName);
					pvm_send(temp,1);
					pvm_recv(temp,1);	
				}
					
				
				pvm_upkint(&answer,1,1);	
				if(answer==error){
					pvm_upkstr(errorCode);
					wlog(errorCode);

					pvm_initsend(PvmDataDefault); //to source
					pvm_pkint(&error,1,1);
					pvm_pkstr(errorCode);
					pvm_send(source,1);
					break;
				}
				
				wlog("connected...\n");
				pvm_initsend(PvmDataDefault); //to source
				pvm_pkint(&success,1,1);
				pvm_send(source,1);
				break;
			case 2: //create new table
				pvm_upkstr(dbName);
				pvm_upkstr(tableName);
				temp=list.findC(dbName); //find it's node
				if(temp<0){ 
					//to the engine					
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&choice,1,1); //it will be deleted after deleteing the active database from engine
					pvm_pkstr(dbName);
					pvm_pkstr(tableName);
					pvm_send(engineID,1); //to engine
					pvm_recv(engineID,1);
				}
				else{ //to another cluster
					//send the message to the owner cluster 
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&choice,1,1);
					pvm_pkstr(dbName);
					pvm_pkstr(tableName);
					pvm_send(temp,1);
					pvm_recv(temp,1);	
				}
					
				
				pvm_upkint(&answer,1,1);	
				if(answer==error){
					pvm_upkstr(errorCode);
					wlog(errorCode);

					pvm_initsend(PvmDataDefault); //to source
					pvm_pkint(&error,1,1);
					pvm_pkstr(errorCode);
					pvm_send(source,1);
					break;
				}

				wlog("table created\n");
				pvm_initsend(PvmDataDefault); //to source
				pvm_pkint(&success,1,1);
				pvm_send(source,1);
				break;
			case 3: //create column
				pvm_upkstr(dbName);
				pvm_upkstr(tableName);
				pvm_upkstr(colName);
				pvm_upkstr(dataType);
				pvm_upkstr(cons);
				temp=list.findC(dbName); //find it's node
				if(temp<0){ 
					//to the engine					
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&choice,1,1); //it will be deleted after deleteing the active database from engine
					pvm_pkstr(dbName);
					pvm_pkstr(tableName);
					pvm_pkstr(colName);
					pvm_pkstr(dataType);
					pvm_pkstr(cons);	
					pvm_send(engineID,1); //to engine
					pvm_recv(engineID,1);
				}
				else{ //to another cluster
					//send the message to the owner cluster 
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&choice,1,1);
					pvm_pkstr(dbName);
					pvm_pkstr(tableName);
					pvm_pkstr(colName);
					pvm_pkstr(dataType);
					pvm_pkstr(cons);	
					pvm_send(temp,1);
					pvm_recv(temp,1);	
				}
					
				
				pvm_upkint(&answer,1,1);	
				if(answer==error){
					pvm_upkstr(errorCode);
					wlog(errorCode);

					pvm_initsend(PvmDataDefault); //to source
					pvm_pkint(&error,1,1);
					pvm_pkstr(errorCode);
					pvm_send(source,1);
					break;
				}

				wlog("column created\n");
				pvm_initsend(PvmDataDefault); //to source
				pvm_pkint(&success,1,1);
				pvm_send(source,1);
				break;
			case 4: //insert row
				pvm_upkstr(dbName);
				pvm_upkstr(tableName);
				pvm_upkstr(row);

				temp=list.findC(dbName); //find it's node
				if(temp<0){ 
					//to the engine					
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&choice,1,1); //it will be deleted after deleteing the active database from engine
					pvm_pkstr(dbName);
					pvm_pkstr(tableName);
					pvm_pkstr(row);
					pvm_send(engineID,1); //to engine
					pvm_recv(engineID,1);
				}
				else{ //to another cluster
					//send the message to the owner cluster 
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&choice,1,1);
					pvm_pkstr(dbName);
					pvm_pkstr(tableName);
					pvm_pkstr(row);
					pvm_send(temp,1);
					pvm_recv(temp,1);	
				}
					
				
				pvm_upkint(&answer,1,1);	
				if(answer==error){
					pvm_upkstr(errorCode);
					wlog(errorCode);

					pvm_initsend(PvmDataDefault); //to source
					pvm_pkint(&error,1,1);
					pvm_pkstr(errorCode);
					pvm_send(source,1);
					break;
				}
				
				wlog("row has been inserted\n");	
				pvm_initsend(PvmDataDefault); //to source
				pvm_pkint(&success,1,1);
				pvm_send(source,1);
				break;
			case 5: //drop table
				pvm_upkstr(dbName);
				pvm_upkstr(tableName);
				temp=list.findC(dbName); //find it's node
				if(temp<0){ 
					//to the engine					
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&choice,1,1); //it will be deleted after deleteing the active database from engine
					pvm_pkstr(dbName);
					pvm_pkstr(tableName);	
					pvm_send(engineID,1); //to engine
					pvm_recv(engineID,1);
				}
				else{ //to another cluster
					//send the message to the owner cluster 
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&choice,1,1);
					pvm_pkstr(dbName);
					pvm_pkstr(tableName);	
					pvm_send(temp,1);
					pvm_recv(temp,1);	
				}
					
				
				pvm_upkint(&answer,1,1);	
				if(answer==error){
					pvm_upkstr(errorCode);
					wlog(errorCode);

					pvm_initsend(PvmDataDefault); //to source
					pvm_pkint(&error,1,1);
					pvm_pkstr(errorCode);
					pvm_send(source,1);
					break;
				}
				
				wlog("table droped\n");
				pvm_initsend(PvmDataDefault); //to source
				pvm_pkint(&success,1,1);
				pvm_send(source,1);

				break;
			case 6: //delete row
				pvm_upkstr(dbName);
				pvm_upkstr(tableName);
				pvm_upkint(&rowID,1,1);
				temp=list.findC(dbName); //find it's node
				if(temp<0){ 
					//to the engine					
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&choice,1,1); //it will be deleted after deleteing the active database from engine
					pvm_pkstr(dbName);
					pvm_pkstr(tableName);
					pvm_pkint(&rowID,1,1);	
					pvm_send(engineID,1); //to engine
					pvm_recv(engineID,1);
				}
				else{ //to another cluster
					//send the message to the owner cluster 
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&choice,1,1);
					pvm_pkstr(dbName);
					pvm_pkstr(tableName);
					pvm_pkint(&rowID,1,1);	
					pvm_send(temp,1);
					pvm_recv(temp,1);	
				}
					
				
				pvm_upkint(&answer,1,1);	
				if(answer==error){
					pvm_upkstr(errorCode);
					wlog(errorCode);

					pvm_initsend(PvmDataDefault); //to source
					pvm_pkint(&error,1,1);
					pvm_pkstr(errorCode);
					pvm_send(source,1);
					break;
				}
				
				wlog("row deleted\n");
				pvm_initsend(PvmDataDefault); //to source
				pvm_pkint(&success,1,1);
				pvm_send(source,1);
				break;
			case 7: //view table names

				pvm_upkstr(dbName);
				temp=list.findC(dbName); //find it's node
				if(temp<0){ 
					//to the engine					
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&choice,1,1); //it will be deleted after deleteing the active database from engine
					pvm_pkstr(dbName);
					pvm_send(engineID,1); //to engine
					pvm_recv(engineID,1);
				}
				else{ //to another cluster
					//send the message to the owner cluster 
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&choice,1,1);
					pvm_pkstr(dbName);
					pvm_send(temp,1);
					pvm_recv(temp,1);	
				}
					
				
				pvm_upkint(&answer,1,1);	
				if(answer==error){
					pvm_upkstr(errorCode);
					wlog(errorCode);

					pvm_initsend(PvmDataDefault); //to source
					pvm_pkint(&error,1,1);
					pvm_pkstr(errorCode);
					pvm_send(source,1);
					break;
				}

				pvm_upkstr(namesBuff);
				wlog("tableNames viewed\n");
				pvm_initsend(PvmDataDefault); //to source
				pvm_pkint(&success,1,1);
				pvm_pkstr(namesBuff);
				pvm_send(source,1);

				break;
			case 8: //view table definition			
				pvm_upkstr(dbName);
				pvm_upkstr(tableName);
				temp=list.findC(dbName); //find it's node
				if(temp<0){ 
					//to the engine					
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&choice,1,1); //it will be deleted after deleteing the active database from engine
					pvm_pkstr(dbName);
					pvm_pkstr(tableName);
					pvm_send(engineID,1); //to engine
					pvm_recv(engineID,1);
				}
				else{ //to another cluster
					//send the message to the owner cluster 
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&choice,1,1);
					pvm_pkstr(dbName);
					pvm_pkstr(tableName);
					pvm_send(temp,1);
					pvm_recv(temp,1);	
				}
					
				
				pvm_upkint(&answer,1,1);	
				if(answer==error){
					pvm_upkstr(errorCode);
					wlog(errorCode);

					pvm_initsend(PvmDataDefault); //to source
					pvm_pkint(&error,1,1);
					pvm_pkstr(errorCode);
					pvm_send(source,1);
					break;
				}
				
				wlog("table definition viewd\n");
				pvm_upkstr(buffer);
				pvm_initsend(PvmDataDefault); //to source
				pvm_pkint(&success,1,1);
				pvm_pkstr(buffer);
				pvm_send(source,1);
				break;
			case 9: //view table data
				pvm_upkstr(dbName);
				pvm_upkstr(tableName);
				temp=list.findC(dbName); //find it's node
				if(temp<0){ 
					//to the engine					
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&choice,1,1); //it will be deleted after deleteing the active database from engine
					pvm_pkstr(dbName);
					pvm_pkstr(tableName);
					pvm_send(engineID,1); //to engine
					pvm_recv(engineID,1);
				}
				else{ //to another cluster
					//send the message to the owner cluster 
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&choice,1,1);
					pvm_pkstr(dbName);
					pvm_pkstr(tableName);
					pvm_send(temp,1);
					pvm_recv(temp,1);	
				}
					
				
				pvm_upkint(&answer,1,1);	
				if(answer==error){
					pvm_upkstr(errorCode);
					wlog(errorCode);

					pvm_initsend(PvmDataDefault); //to source
					pvm_pkint(&error,1,1);
					pvm_pkstr(errorCode);
					pvm_send(source,1);
					break;
				}

				wlog("dable data viewed\n");
				pvm_upkstr(buffer);
				pvm_initsend(PvmDataDefault); //to source
				pvm_pkint(&success,1,1);
				pvm_pkstr(buffer);
				pvm_send(source,1);
				break;
			case 10: //create new database on your cluster
				pvm_upkstr(dbName);
				pvm_upkstr(filePath);
				
				pvm_initsend(PvmDataDefault);
				pvm_pkint(&choice,1,1);
				pvm_pkstr(dbName);
				pvm_pkstr(filePath);								
				pvm_send(engineID,1);

				pvm_recv(engineID,1);
				pvm_upkint(&answer,1,1);
				if(answer==error){
					pvm_upkstr(errorCode);
					wlog(errorCode);

					pvm_initsend(PvmDataDefault); //to source
					pvm_pkint(&error,1,1);
					pvm_pkstr(errorCode);
					pvm_send(source,1);
					break;
				}
				wlog("database created\n");	
				pvm_initsend(PvmDataDefault); //to source
				pvm_pkint(&success,1,1);
				pvm_send(source,1);

				pvm_initsend(PvmDataDefault);
				pvm_pkint(&UpdateMessage,1,1);
				pvm_mcast(list.gettids(),list.getCount(),1); //notfiy all clusters in the network
				list.refresh();
				break;
			case 11: //create database from existing file				
				pvm_upkstr(filePath);

				pvm_initsend(PvmDataDefault); //to engine
				pvm_pkint(&choice,1,1);
				pvm_pkstr(filePath);				
				pvm_send(engineID,1);

				pvm_recv(engineID,1);
				pvm_upkint(&answer,1,1);

				if(answer==error){
					pvm_upkstr(errorCode);
					wlog(errorCode);

					pvm_initsend(PvmDataDefault); //to source
					pvm_pkint(&error,1,1);
					pvm_pkstr(errorCode);
					pvm_send(source,1);
					break;
				}
				pvm_upkstr(dbName);
				wlog("database created from file\n");

				pvm_initsend(PvmDataDefault); //to source
				pvm_pkint(&success,1,1);
				pvm_pkstr(dbName);
				pvm_send(source,1);
				/*
				pvm_initsend(PvmDataDefault);
				pvm_pkint(&UpdateMessage,1,1);
				pvm_mcast(list.gettids(),list.getCount(),1); //notfiy all clusters in the network
				*/

				break;
			case 12: //delete existing database,allow deleting databases on any place in the network
				pvm_upkstr(dbName);

				temp=list.findC(dbName);
				if(temp<0){ 
					//try your engine
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&choice,1,1); //it will be deleted after deleteing the active database from engine
					pvm_pkstr(dbName);
					pvm_send(engineID,1); //to engine
					pvm_recv(engineID,1);
				}
				else{ //delete it from another cluster
					pvm_initsend(PvmDataDefault); //to engine
					pvm_pkint(&choice,1,1);
					pvm_pkstr(dbName);				
					pvm_send(temp,1); //to another cluster
					pvm_recv(temp,1);
				}

				pvm_upkint(&answer,1,1); 
				if(answer==error){ //database doesn't exists
					pvm_initsend(PvmDataDefault); //to source
					pvm_pkint(&error,1,1);
					pvm_pkstr("database doesn't exist\n");
					pvm_send(source,1);
					break;
				}
				pvm_initsend(PvmDataDefault); //to source
				pvm_pkint(&success,1,1);
				pvm_send(source,1);
				list.refresh();
				wlog("database deleted\n");
				//cast
				break;
			case 13: //view all databases in this cluster only

				pvm_initsend(PvmDataDefault); //to engine
				pvm_pkint(&choice,1,1);
				pvm_send(engineID,1);

				pvm_recv(engineID,1);
				pvm_upkint(&answer,1,1);
				if(answer==error){
					pvm_upkstr(errorCode);
					wlog(errorCode);

					pvm_initsend(PvmDataDefault); //to source
					pvm_pkint(&error,1,1);
					pvm_pkstr(errorCode);
					pvm_send(source,1);
					break;
				}
				pvm_upkstr(namesBuff);

				wlog("viewing registered databases on local host\n");
				
				pvm_initsend(PvmDataDefault); //to source
				pvm_pkint(&success,1,1);
				pvm_pkstr(namesBuff);
				pvm_send(source,1);

				break;
			case 14: //does a database exists
				pvm_upkstr(dbName);
				list.refresh();
				temp=list.findC(dbName);
				if(temp<0){ 
					//search the engine
					
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&choice,1,1); //it will be deleted after deleteing the active database from engine
					pvm_pkstr(dbName);
					pvm_send(engineID,1); //to engine
					pvm_recv(engineID,1);
					pvm_upkint(&answer,1,1); 
					if(answer==error){ //database doesn't exists
						pvm_initsend(PvmDataDefault); //to source
						pvm_pkint(&error,1,1);
						pvm_pkstr("database doesn't exist\n");
						pvm_send(source,1);
						break;
					}
				}
				else
					wlog("other cluster\n");

				wlog(dbName);
				wlog(":find database\n");
				pvm_initsend(PvmDataDefault); //to source
				pvm_pkint(&success,1,1);
				pvm_send(source,1);
				break;
			case 15: //query
				pvm_upkstr(dbName);
				pvm_upkstr(query);
				temp=list.findC(dbName); //find it's node
				if(temp<0){ 
					//to the engine					
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&choice,1,1); //it will be deleted after deleteing the active database from engine
					pvm_pkstr(dbName);
					pvm_pkstr(query);	
					pvm_send(engineID,1); //to engine
					pvm_recv(engineID,1);
				}
				else{ //to another cluster
					//send the message to the owner cluster 
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&choice,1,1);
					pvm_pkstr(dbName);
					pvm_pkstr(query);	
					pvm_send(temp,1);
					pvm_recv(temp,1);	
				}
					
				
				pvm_upkint(&answer,1,1);	
				if(answer==error){
					pvm_upkstr(errorCode);
					wlog(errorCode);

					pvm_initsend(PvmDataDefault); //to source
					pvm_pkint(&error,1,1);
					pvm_pkstr(errorCode);
					pvm_send(source,1);
					break;
				}

				pvm_upkstr(buffer);
				wlog("query applied\n");

				pvm_initsend(PvmDataDefault); //to source
				pvm_pkint(&success,1,1);
				pvm_pkstr(buffer);
				pvm_send(source,1);
				break;

			case 16: //view all databases in the network
				list.refresh();
				list.getDBNames(namesBuff);

				wlog("viewing registered databases on the network\n");
				
				pvm_initsend(PvmDataDefault); //to source
				pvm_pkint(&success,1,1);
				pvm_pkstr(namesBuff);
				pvm_send(source,1);

				break;
			case 17://a new cluster added, this message comes only from other clusters
				list.refresh(); 
				break;
			case 18: //disconnect active database
				pvm_upkstr(dbName);
				temp=list.findC(dbName); //find it's node
				if(temp<0){ 
					//to the engine					
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&choice,1,1); //it will be deleted after deleteing the active database from engine
					pvm_pkstr(dbName);
					pvm_send(engineID,1); //to engine
					pvm_recv(engineID,1);
				}
				else{ //to another cluster
					//send the message to the owner cluster 
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&choice,1,1);
					pvm_pkstr(dbName);
					pvm_send(temp,1);
					pvm_recv(temp,1);	
				}
					
				
				pvm_upkint(&answer,1,1);	
				if(answer==error){
					pvm_upkstr(errorCode);
					wlog(errorCode);

					pvm_initsend(PvmDataDefault); //to source
					pvm_pkint(&error,1,1);
					pvm_pkstr(errorCode);
					pvm_send(source,1);
					break;
				}

				wlog("disconnected.\n");
				pvm_initsend(PvmDataDefault); //to source
				pvm_pkint(&success,1,1);
				pvm_send(source,1);
				break;
			default:
					wlog("unrecognized choice\n");

					pvm_initsend(PvmDataDefault); //to source
					pvm_pkint(&error,1,1);
					pvm_pkstr("unrecognized choice\n");
					pvm_send(source,1);
					break;
		}
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
	delete []errorCode;
	delete []buffer;
	
	wlog("********** CLUSTER BEGIN ************\n");
	pvm_exit();

	return 0;
}


