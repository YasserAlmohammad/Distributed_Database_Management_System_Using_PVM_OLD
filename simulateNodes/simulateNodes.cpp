/*  to test the cluster communication on the same computer we added this program
	which generates 5 clusters and provides them with 3 databases for each
	when the client wants ro create a database he creates it in his cluster
	- yet it could be easier to let the client creates the databases on other
	clusters, but the other clusters should be hided from the client as well
	as the engine -
	we'll spawn 5 clusters, each cluster spawns it's own engine
	then add 5 databases to each.
	
	this program can either be run stand alone or spawned using other program

	- we already created one database called source in source.txt, (it must exist in the
	  same directory) so we'll duplicate this database as needed for the simulation 
	  with changing the name only
*/

#include <pvm3.h>
#include <iostream.h>

//dupliacate database: create new database file as a copy from source, only change name
bool duplicate(char* srcFileName, char* desFileName,char* desDBName); 

int main(int argc,char* argv[]){	
	int* tids=new int[5];
	int parentID=pvm_parent();
	
	if(parentID<0 && parentID!=PvmNoParent){ //error PVM is not running
		cout<<"error, most likely PVM is not runing, press any number to exit...\n";
		cin>>parentID; //just to stay active for the user to see the message
		return -1;
	}
	int c=pvm_spawn("dbCluster",(char**)0,0,"",5,tids);
	

	if(c<1){
		cout<<"couldn't spawn any clusters\n";
		cin>>c;
		delete []tids;
		return -1;
	}

	/*add databases from precreated files:
	  db names are: cidbi.txt (i=1..5)
	*/
	int choice=11; //11 is create data base from file, params are: the dataFile path
	char* fileName=new char[50]; //only name required (files are in the same directory)
	char* dbName=new char[50];
	char* errorCode=new char[100];
	int answer=-1;
	char* source="source.txt";
	char* destPath=new char[200];
	char* temp=new char[200];

	//if we wanted to spawn this program we need argumen for this
	if(parentID==PvmNoParent){
		cout<<"input destination for databases created by the simulation(i.e c:\\):\n";
		cin>>destPath;
	}
	else{
		if(argc!=1)
			return -1;
		else
			strcpy(destPath,argv[0]);

	}

	for(int ci=1 /* cluster number*/; ci<=c; ci++){
		for(int dbi=1; dbi<=3; dbi++){
			sprintf(fileName,"c%ddb%d.txt",ci,dbi);
			sprintf(dbName,"c%ddb%d",ci,dbi);
			strcpy(temp,destPath);
			duplicate(source,strcat(temp,fileName),dbName); //duplicate

			pvm_initsend(PvmDataDefault);
			pvm_pkint(&choice,1,1);
			pvm_pkstr(temp);
			pvm_send(tids[ci-1],1);
			// ckeck for success values
			pvm_recv(tids[ci-1],1);
			pvm_upkint(&answer,1,1);
			if(answer==1){ //error, happens if file is corrupted, access denied, or PVM error
				cout<<"couldn't add c"<<ci<<"db"<<dbi<<" to cluster"<<ci<<" the reason:"<<endl;
				pvm_upkstr(errorCode);
				cout<<errorCode<<endl;
			}

		}
		cout<<"******cluster"<<ci<<" was created successfully******\n";
	}

	if(parentID==PvmNoParent){ //the simulator was runing stand alone
		cout<<"you can exit by pressing any number, but don't\n"
			"until you are done from the clusters test...\n";
		cin>>parentID;
		//exit all clusters normally
		choice=0; //exit
		for(int i=0; i<c; i++){
			pvm_initsend(PvmDataDefault);
			pvm_pkint(&choice,1,1);
			pvm_send(tids[i],1);
		}		
	}
	else{ //the program was generated using spawn, wait for father to send exit signal or kill
		while(true){
			pvm_recv(parentID,1); //block
			pvm_upkint(&choice,1,1);
			if(choice==0) //exit signal
				break;
		}
	}

	delete []fileName;
	delete [] errorCode;
	delete []tids;
	delete []destPath;
	delete []temp;

	return 0;
}

bool duplicate(char* srcFileName, char* desFileName,char* desDBName){
	FILE* src=fopen(srcFileName,"r");		
	if(src==NULL)
		return false; //don't create dest if source is not available

	FILE* des=fopen(desFileName,"w");
	if(des==NULL)
		return false;

	char* line=new char[200];
	line[0]='\0';
	strcat(line,"<DATABASE> ");
	strcat(line,desDBName);
	strcat(line,"\n");
	fputs(line,des); //chage the first line only containg the database name
	fgets(line,200,src); //skip line

	while(!feof(src)){
		fgets(line,200,src);
		fputs(line,des);
	}	

	delete []line;
	fclose(src);
	fclose(des);
	
	return true;

}