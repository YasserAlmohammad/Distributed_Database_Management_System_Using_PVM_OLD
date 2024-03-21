
/*  the client program:
	-	the client deals with the database without knowing any details about clusters
		or thier distribution
	-	when the client first work it probes the PVM for all available clusters
		on all registered computers
		but first of all it checks his computer to see if it has a cluster or not
		the first cluster it finds(the local has priority) it will connect to it
		of course this will be done without user knowledge

	- only one cluster a client can deal directly. the one it deals with interacts
	  with other clusters. 
    - to do operations on a database you must first make it active, so operations are done
	  on the active database by default, the active database belongs to a cluster, which might
	  be your cluster or other cluster in the network, so the active cluster is the one holding
	  the active database( your cluster is the one the client is connected to)
	-  database objects are shared so a client can make a connect but can't disconnect the database
	   the connect only makes the database object available it doesn't make an open connection 
	   or something.
	   so many users can deal with the same database all over the network
	   many clients can be activated even in the same computer but all clients in the same computer 
	   connects to the first availble cluster(if one exists on the local machine then all of them
	   connects to the same cluster)

*/



#include <pvm3.h>
#include <iostream.h>
#include <string.h>

char* copy(char* str);

int main(int argc,char* argv[]){

	cout<<"******** the database client program ************\n";
	//first probe PVM for available clusters starting with the local host
	
	int clusterID=-1;
	int Pvmd_tid=pvm_tidtohost(pvm_mytid()); //get your daemon id
	int ntasks=0;
	struct pvmtaskinfo* tasks;
	
	int info=pvm_tasks(Pvmd_tid,&ntasks,&tasks);
	//when runing the simulator program ntasks will be more than one in the same computer
	if(info<0){
		cout<<"error: check PVM if it's runing or not\n"
			"press any number to exit...\n";
		cin>>info;
		return -1;
	}

	for(int i=0; i<ntasks; i++){
		if(strcmp("dbCluster.exe",tasks[i].ti_a_out)==0){ //just find
			clusterID=tasks[i].ti_tid;
			cout<<"a cluster running on your machine was found and connected to...\n";
			break;
		}
	}
	
	if(clusterID==-1){ //search all the PVM netwrok
		info=pvm_tasks(0,&ntasks,&tasks);
		if(info<0){
			cout<<"error: check PVM if it's runing or not\n"
				"press any number to exit...\n";
			cin>>info;
			return -1;
		}
		for(int i=0; i<ntasks; i++){
			if(strcmp("dbCluster.exe",tasks[i].ti_a_out)==0){ //just fine
				clusterID=tasks[i].ti_tid;
				cout<<"a cluster running on another machine was found and connected to...\n";
				break;
			}
		}
	}

	if(clusterID<0){ //hasn't changed]
		cout<<"no clusters found on the network program will not run.\n"
			"press any number to exit...\n";
		cin>>info;
		return -1;
	}

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
	char* activeDB=NULL;


	int sub=0; //sub choice
	int code=0; //code to send to cluster
	int choice=0;
	int rowID=1;
	int answer=-1; //0 success, 1 error

	do{
		cout<<"\n|---- MAIN MENU -----|\n";
		cout<<"input: 0 exit\n"
			  "     : 1 (subMenu)operations on active database\n"
			  "     : 2 create new database\n"
			  "     : 3 create database from existing file\n"
			  "     : 4 delete existing database\n"
			  "     : 5 view all databases in your machine\n"
			  "     : 6 to view all databases in the network\n"
			  "     : 7 set the active database(only one database can be active at a time)\n"
			  "     : 8 to refresh content manually:\n";
		
		cin>>choice;

		

		// after each send operation a bloching recieve is waiting the answer from the engine
		//if the answer is 1(failure) then a string parameter passing the error reason is recieved too
		
		pvm_initsend(PvmDataDefault);

		switch(choice){
			case 0: //exit
				break;
			case 1:
				if(activeDB==NULL){
					cout<<"you need an active database to activate this sub menu\n"
					"(see main menu incase you didn't choose one)\n";
					break;
				}
				do{
					

					cout<<"\n|---- SUB MENU -----|\n";
					cout<<"input: 0 to return to the main menu\n"
						"     : 1 connect database\n"
						"     : 2 create new table\n"
						"     : 3 create column\n"
	   					"     : 4 insert row\n"
						"     : 5 drop table\n"
						"     : 6 delete row\n"
						"     : 7 view table names\n"
						"     : 8 view table definition\n"
						"     : 9 view table data\n"
						"     : 10 query the database\n";
				//		"     : 11 disconnect:\n";
					cin>>sub;
					pvm_initsend(PvmDataDefault);

					switch(sub){
						case 0:
							break;
						case 1: //connect the active database
							code=sub;
							pvm_pkint(&code,1,1);
							pvm_pkstr(activeDB);
							pvm_send(clusterID,1);

							pvm_recv(clusterID,1);
							pvm_upkint(&answer,1,1);	
							
							if(answer==1){ //error
								pvm_upkstr(errorCode);
								cout<<errorCode;
								break;
							}
							
							cout<<"connected...\n";
							break;
						case 2: //create new table
							cout<<"input table name:";
							cin>>tableName;
							code=sub;
							pvm_pkint(&code,1,1);
							pvm_pkstr(activeDB);
							pvm_pkstr(tableName);				
							pvm_send(clusterID,1);

							pvm_recv(clusterID,1);
							pvm_upkint(&answer,1,1);				
							if(answer==1){ //error
								pvm_upkstr(errorCode);
								cout<<errorCode;
								break;
							}

							cout<<"table created\n";
							break;
						case 3: //create column
							code=sub;
							cout<<"input existing tableName:";
							cin>>tableName;
							cout<<"input column Name:";
							cin>>colName;
							cout<<"input dataType (int|string|double):";
							cin>>dataType;
							cout<<"input constraint(PK|UK|NULL) where: PK is primary key, UK is unique, NULL no constraints:";
							cin>>cons;

							pvm_pkint(&code,1,1);
							pvm_pkstr(activeDB);
							pvm_pkstr(tableName);
							pvm_pkstr(colName);
							pvm_pkstr(dataType);
							pvm_pkstr(cons);				
							pvm_send(clusterID,1);

							pvm_recv(clusterID,1);
							pvm_upkint(&answer,1,1);
							if(answer==1){//error
								pvm_upkstr(errorCode);
								cout<<errorCode;
								break;
							}
							cout<<"column created\n";
							break;
						case 4: //insert row
							code=sub;
							cout<<"input table name:";
							cin>>tableName;
							cout<<"input data in the form: col1data ... colndata: input | to end input\n";
							cout.flush();
							cin.getline(row,100,'|'); //use getline

							pvm_pkint(&code,1,1);
							pvm_pkstr(activeDB);
							pvm_pkstr(tableName);
							pvm_pkstr(row);				
							pvm_send(clusterID,1);

							pvm_recv(clusterID,1);
							pvm_upkint(&answer,1,1);
							if(answer==1){//error
								pvm_upkstr(errorCode);
								cout<<errorCode;
								break;
							}
							
							cout<<"row has been inserted\n";				
							break;
						case 5: //drop table
							code=sub;
							cout<<"input table name to drop( all data will be removed):";
							cin>>tableName;
							
							pvm_pkint(&code,1,1);
							pvm_pkstr(activeDB);
							pvm_pkstr(tableName);				
							pvm_send(clusterID,1);

							pvm_recv(clusterID,1);
							pvm_upkint(&answer,1,1);
							if(answer==1){//error
								pvm_upkstr(errorCode);
								cout<<errorCode;
								break;
							}
							cout<<"table droped\n";

							break;
						case 6: //delete row
							code=sub;
							cout<<"input table name:";
							cin>>tableName;
							cout<<"input row number:";
							cin>>rowID;

							pvm_pkint(&code,1,1);
							pvm_pkstr(activeDB);
							pvm_pkstr(tableName);
							pvm_pkint(&rowID,1,1);								
							pvm_send(clusterID,1);

							pvm_recv(clusterID,1);
							pvm_upkint(&answer,1,1);
							if(answer==1){//error
								pvm_upkstr(errorCode);
								cout<<errorCode;
								break;
							}
							cout<<"row deleted\n";
							break;
						case 7: //view table names
							
							code=sub;
							pvm_pkint(&code,1,1);
							pvm_pkstr(activeDB);
							pvm_send(clusterID,1);

							pvm_recv(clusterID,1);
							pvm_upkint(&answer,1,1);
							if(answer==1){//error
								pvm_upkstr(errorCode);
								cout<<errorCode;
								break;
							}
							pvm_upkstr(namesBuff);
							cout<<namesBuff;
							break;
						case 8: //view table definition
							code=sub;
							cout<<"input table name to view it's definition:";
							cin>>tableName;

							pvm_pkint(&code,1,1);
							pvm_pkstr(activeDB);
							pvm_pkstr(tableName);				
							pvm_send(clusterID,1);

							pvm_recv(clusterID,1);
							pvm_upkint(&answer,1,1);
							if(answer==1){//error
								pvm_upkstr(errorCode);
								cout<<errorCode;
								break;
							}
							pvm_upkstr(buffer);
							cout<<buffer;
							break;
						case 9: //view table data
							code=sub;
							cout<<"input table name:";
							cin>>tableName;
							
							pvm_pkint(&code,1,1);
							pvm_pkstr(activeDB);
							pvm_pkstr(tableName);				
							pvm_send(clusterID,1);

							pvm_recv(clusterID,1);
							pvm_upkint(&answer,1,1);
							if(answer==1){//error
								pvm_upkstr(errorCode);
								cout<<errorCode;
								break;
							}
							pvm_upkstr(buffer);
							cout<<buffer;
							break;
						case 10:
							code=15; //code 15 is query
							cout<<"only a small select functionality is achieved and that is:\n"
								"SELECT col1,...,coln\n"
								"FROM OneTablName\n"
								"WHERE OneAnyCol<|>|=value ;\n"
								"- the ; ends the input:\n"
								"- column names must be concatenated by ,\n"
								"- you can write the query on the same line or multible lines\n"
								"- you can use Tabs, new lines, or spaces to separate cluases\n"
								"- don't separate the operation from the operators in the condition clause\n";
								
							cout.flush();
							cin.getline(query,1000,';');
							
							pvm_pkint(&code,1,1);
							pvm_pkstr(activeDB);
							pvm_pkstr(query);				
							pvm_send(clusterID,1);

							pvm_recv(clusterID,1);
							pvm_upkint(&answer,1,1);
							if(answer==1){
								pvm_upkstr(errorCode);
								cout<<errorCode;
								break;
							}
							pvm_upkstr(buffer);
							cout<<buffer;
							break;
							/*
						case 11:
							code=18; //disconnect
							pvm_pkint(&code,1,1);
							pvm_pkstr(activeDB);
							pvm_send(clusterID,1);

							pvm_recv(clusterID,1);
							pvm_upkint(&answer,1,1);	
							
							if(answer==1){ //error
								pvm_upkstr(errorCode);
								cout<<errorCode;
								break;
							}
							
							cout<<"disconnected.\n";
							break;
							*/
						default:
							cout<<"input a valid choice\n";
							break;
					}
				}while(sub!=0);

				break;
			case 2: //create new database
				code=10; //code 10 creates new database
				cout<<"input database name:";
				cin>>dbName;				
				cout<<"input filePath to create in:";
				cin>>filePath;
				
				pvm_pkint(&code,1,1);
				pvm_pkstr(dbName);
				pvm_pkstr(filePath);								
				pvm_send(clusterID,1);

				pvm_recv(clusterID,1);
				pvm_upkint(&answer,1,1);
				if(answer==1){//error
					pvm_upkstr(errorCode);
					cout<<errorCode;
					break;
				}
				cout<<"database created\n";	
				break;
			case 3: //create database from existing file
				code=11; //code 11 to cluster
				cout<<"input database DataFile Path to create from :";
				cin>>filePath;

				pvm_pkint(&code,1,1);
				pvm_pkstr(filePath);				
				pvm_send(clusterID,1);

				pvm_recv(clusterID,1);
				pvm_upkint(&answer,1,1);
				if(answer==1){//error
					pvm_upkstr(errorCode);
					cout<<errorCode;
					break;
				}
				pvm_upkstr(dbName);
				cout<<dbName<<" database created from:"<<filePath<<endl;
				break;
			case 4: //delete existing database
				code=12; //code 12
				cout<<"input database name:\n";
				cin>>dbName;
				pvm_pkint(&code,1,1);
				pvm_pkstr(dbName);				
				pvm_send(clusterID,1);

				pvm_recv(clusterID,1);
				pvm_upkint(&answer,1,1);
				if(answer==1){//error
					pvm_upkstr(errorCode);
					cout<<errorCode;
					break;
				}
				cout<<"database has been deleted(datafile remains)\n";
				break;
			case 5: //view all databases
				code=13; //code 13
				pvm_pkint(&code,1,1);
				pvm_send(clusterID,1);

				pvm_recv(clusterID,1);
				pvm_upkint(&answer,1,1);
				if(answer==1){//error
					pvm_upkstr(errorCode);
					cout<<errorCode;
					break;
				}
				pvm_upkstr(namesBuff);
				cout<<"databases registered on your machine:\n"<<namesBuff;
				break;
			case 6: //view databases on the network
				code=16; //code 16
				pvm_pkint(&code,1,1);
				pvm_send(clusterID,1);

				pvm_recv(clusterID,1);
				pvm_upkint(&answer,1,1);
				if(answer==1){//error
					pvm_upkstr(errorCode);
					cout<<errorCode;
					break;
				}
				pvm_upkstr(buffer);
				cout<<"databases registered on the whole netwrok:\n"<<buffer;
				code=13; //also the database on your machine
				pvm_initsend(PvmDataDefault);
				pvm_pkint(&code,1,1);
				pvm_send(clusterID,1);

				pvm_recv(clusterID,1);
				pvm_upkint(&answer,1,1);
				if(answer==1){//error
					pvm_upkstr(errorCode);
					cout<<errorCode;
					break;
				}
				pvm_upkstr(namesBuff);
				cout<<namesBuff<<endl;;				
				break;
			case 7: //set active database
				//check if it exists
				code=14; //code 14
				cout<<"input database name:\n";
				cin>>dbName;
				pvm_pkint(&code,1,1);
				pvm_pkstr(dbName);				
				pvm_send(clusterID,1);

				pvm_recv(clusterID,1);
				pvm_upkint(&answer,1,1);
				if(answer==1){//error
					pvm_upkstr(errorCode);
					cout<<errorCode;
					break;
				}
				cout<<dbName<<" is now the active database\n";
				activeDB=copy(dbName);
				break;
			case 8: //reresh
				code=17; //refresh
				pvm_pkint(&code,1,1);
				pvm_send(clusterID,1);
				break;
			default:
				cout<<"input a valid choice\n";
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

	return 0;
}


char* copy(char* str){
	char* dup=new char[strlen(str)+1];
	strcpy(dup,str);
	dup[strlen(str)]='\0';
	return dup;
}