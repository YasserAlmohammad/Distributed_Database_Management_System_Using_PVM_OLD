#include "utils.h"

char* copy(char* str){
	char* dup=new char[strlen(str)+1];
	strcpy(dup,str);
	return dup;
}

DBNode::DBNode(){
	name=NULL;
	dataBase=NULL;
	next=NULL;
}

DBNode::DBNode(char* name,DB* val,DBNode* next){
	this->name=copy(name);
	dataBase=val;
	this->next=next;
}


void DBList::insertDB(char* name,DB* dataBase){
	//add to the front
	DBNode* newNode=new DBNode(copy(name),dataBase,head);
	head=newNode;
}

bool DBList::deleteDB(char* name){
	//first find the database DBNode

	DBNode* temp=head;
	DBNode* prev=NULL;
	while(temp!=NULL){
		if(strcmp(name,temp->name)==0){ //found
			if(prev==NULL){ //the head
				head=head->next;
				delete temp;
				return true;
			}
			else{
				prev->next=temp->next;
				delete temp;
				return true;
			}
		}
		prev=temp;
		temp=temp->next;

	}

	return false;
}

void DBList::getNames(char* buffer){
	buffer[0]='\0';
	DBNode* temp=head;
	
	while(temp!=NULL){
		strcat(buffer,temp->name);
		strcat(buffer,"\n");
	
		temp=temp->next;
	}
	
}

DB* DBList::findDB(char* name){
	DBNode* temp=head;
	while(temp!=NULL){
		if(strcmp(name,temp->name)==0)
			return temp->dataBase;

		temp=temp->next;

	}

	return NULL;


}

void DBList::clear(){
	DBNode* temp=head;
	while(head!=NULL){
		head=head->next;
		delete temp;
		temp=head;
	}
	head=NULL;
}

DBList::~DBList(){
	clear();
}

