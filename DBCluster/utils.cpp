#include "utils.h"

void wlog(char* message){
	FILE* log; //log file to write events to
	log=fopen("c:\\dblog.txt","a");
	if(log!=NULL){
		fputs(message,log);
		fclose(log);
	}
}

char* copy(char* str){
	char* dup=new char[strlen(str)+1];
	strcpy(dup,str);
	dup[strlen(str)]='\0';
	return dup;
}

int contains(const char* line, char* word,char* sep)
{
	char* token;
	int index=1;
	//duplicate input line
	int len=strlen(line);
	char* temp=new char[len+1];
	temp[len]='\0';
	strcpy(temp,line);

	token=strtok(temp,sep);
	while(token!=NULL){
		if(strcmp(token,word)==0)
			return index;
		token=strtok(NULL,sep);
		++index;
	}
	return -1;
}

ClusterNode::ClusterNode(){
	dbNames=NULL;
	cid=0;
	next=NULL;
}

ClusterNode::ClusterNode(char* names,int cid,ClusterNode* next){
	this->dbNames=copy(names);
	this->cid=cid;
	this->next=next;
}

ClusterList::ClusterList(){
	head=NULL;
	count=0;
	refresh();
	//find all clusters in the netwrok and get database names from them
}

void ClusterList::insertC(char* name,int id){
	//add to the front
	ClusterNode* newNode=new ClusterNode(copy(name),id,head);
	head=newNode;
	++count;
}

void ClusterList::getDBNames(char* buffer){
	buffer[0]='\0';
	ClusterNode* temp=head;
	
	while(temp!=NULL){
		strcat(buffer,temp->dbNames);
		strcat(buffer,"\n");
	
		temp=temp->next;
	}
	
}

void ClusterList::getDBNames(int cid,char* buffer){
	buffer[0]='\0';
	ClusterNode* temp=head;
	
	while(temp!=NULL){
		if(temp->cid==cid){
			strcat(buffer,temp->dbNames);
			strcat(buffer,"\n");
			return;
		}	
		temp=temp->next;
	}
	
}

int ClusterList::findC(char* dbName){
	ClusterNode* temp=head;
	while(temp!=NULL){
		if(contains(temp->dbNames,dbName," \n\t")>0)
			return temp->cid;

		temp=temp->next;

	}

	return -1;


}

void ClusterList::clear(){
	count=0;
	ClusterNode* temp=head;
	while(head!=NULL){
		head=head->next;
		delete temp;
		temp=head;
	}
	head=NULL;
}

ClusterList::~ClusterList(){
	clear();
}

void ClusterList::refresh(){
	clear();
	int ntasks=0;
	int cid=-1;
	int mytid=pvm_mytid();
	int choice=13; //get database names
	int answer;
	int error=1;
	int success=0;
	char* namesBuff=new char[500];
	char* temp=new char[100];

	struct pvmtaskinfo* tasks;
	
	int info=pvm_tasks(0,&ntasks,&tasks);
	if(info<0)
		return;

	for(int i=0; i<ntasks; i++){
		if(strcmp("dbCluster.exe",tasks[i].ti_a_out)==0){ //a cluster
			cid=tasks[i].ti_tid;
			//get database names of this cluster(cid)
			
			if(cid!=mytid){ //except your cluster
				pvm_initsend(PvmDataDefault);
				pvm_pkint(&choice,1,1);
				pvm_send(cid,1);

				pvm_recv(cid,1); //getnames
				pvm_upkint(&answer,1,1);
				namesBuff[0]='\0';
				if(answer==success)
					pvm_upkstr(namesBuff);
				insertC(namesBuff,cid);
			}
			
		}
	}
	delete []temp;

}

int* ClusterList::gettids(){
	ClusterNode* temp=head;
	int* tids=new int[count];
	int i=0;
	while(temp!=NULL && i<count){
		tids[i]=temp->cid;
		temp=temp->next;
		++i;
	}
	return tids;
	
}

// ckeck if word is inside this line: return it's index(starts from 1) or -1 for none

