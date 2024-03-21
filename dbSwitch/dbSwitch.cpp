/*  this program runs one cluster on the local machine, it's the normal one.
	it first searched the local pvm daemon for other clusters runing, if it doesnn't
	find a one it runs the cluster.
	only one cluster runs on the same machine in normal cases
	(the simultor program was only meant for testing in one machine)
	this program is stand alone
	this program also provide few additional jobs
*/

#include <pvm3.h>
#include <iostream.h>

int main(int argc, char* argv[]){
	int ntasks=0;
	bool ok=true;;
	int mytid=pvm_mytid();
	int choice,info,tid,cid,i;
	cid=-1;

	struct pvmtaskinfo* tasks;
	int did=pvm_tidtohost(mytid); //local daemon id

	do{
		cout<<"\ninput 0: to exit(don't if a client is using you)\n"
			  "        1: create a create cluster on this machine(if one doesn't exists)\n"
			  "        2: to view all runing tasks in this machine\n"
			  "        3: to view all runing tasks in pvm\n"
			  "        4: to kill a task(use this carefully,i.e if a danglling cluster exists)\n";

		cin>>choice;
		switch(choice){
			case 0:
				if(cid!=-1){ //send exit code to cluster
					pvm_initsend(PvmDataDefault);
					pvm_pkint(&choice,1,1);
					pvm_send(cid,1);
				}
				break;
			case 1: //you must add cast to refresh here
				info=pvm_tasks(did,&ntasks,&tasks);
				if(info<0){
					cout<<"check pvm if running\n";
					break;
				}

				for(i=0; i<ntasks; i++){
					if(strcmp("dbCluster.exe",tasks[i].ti_a_out)==0){ //a cluster already exists abort
						cout<<"a cluster already runing...\n";
						ok=false;
						break;
					}
				}
				if(ok){
					int c=pvm_spawn("dbCluster",(char**)0,0,"",1,&cid);
					if(c!=1){
						cout<<"couldn't switch cluster...\n";
						break;
					}
					cout<<"cluster is running...\n";
				}
				break;
			case 2:
				info=pvm_tasks(did,&ntasks,&tasks);
				if(info<0){
					cout<<"check pvm if running\n";
					break;
				}

				for(i=0; i<ntasks; i++){
					cout<<"****taskName:"<<tasks[i].ti_a_out<<"\ttid:"<<tasks[i].ti_tid<<"\t"
						"parentID:"<<tasks[i].ti_ptid<<endl;
				}
				break;
			case 3:
				info=pvm_tasks(0,&ntasks,&tasks);
				if(info<0){
					cout<<"check pvm if running\n";
					break;
				}

				for(i=0; i<ntasks; i++){
					cout<<"****taskName:"<<tasks[i].ti_a_out<<"\ttid:"<<tasks[i].ti_tid<<"\t"
						"parentID:"<<tasks[i].ti_ptid<<endl;
				}
				break;
			case 4:
				cout<<"input task ID:";
				cin>>tid;
				if(pvm_kill(tid)<0)
					cout<<"couldn't kill task\n";
				else
					cout<<"tasks killed...\n";
				
				break;
			default:
				break;
	
		}

	}while(choice!=0);

	pvm_exit();

	return 0;
}