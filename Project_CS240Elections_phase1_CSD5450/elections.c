#include <stdio.h>
#include <stdlib.h>
#include "elections.h"

//definitions in main.c
extern struct parliament Parliament; 
extern struct district Districts[56];
extern struct party Parties[5];


/*Inserts candidate a after candidate b. if b==NULL it inserts at head--used to insert candidate in new position in D.L list
  ptr-to-ptr parameters so that it directly mutates the lists of caller function
  used in vote() to easily change place of candidate to maintain descending sort
*/
void insertCandidate(struct candidate** head, struct candidate** a, struct candidate**b){
	
	struct candidate* new = malloc(sizeof(struct candidate));
	*new = **a;
	if(*b==NULL){
		new->prev = NULL;
		new->next = *head;
		(*head)->prev = new;
		*head = new;
	}else{
		new->next = (*b)->next;
		new->prev = *b;
		(*b)->next->prev = new;
		(*b)->next = new;
		
	}
}

/*deletes candidate c --used to delete candidate from old position after relocation in candidates D.L list
  ptr-to-ptr parameters so that it directly mutates the lists of caller function
  used in vote() to easily change place of candidate to maintain descending sort
*/
void deleteCandidate(struct candidate** c){
	
	if((*c)->next==NULL){
		(*c)->prev->next = NULL;
	}else{
		(*c)->prev->next = (*c)->next;
		(*c)->next->prev = (*c)->prev;
	}

	free(*c);
}

/*inserts a parliament member at the tail of the list in O(1) time*/
void insertMember(struct candidate* c){
	static struct candidate* lastInserted=NULL; //to allow for O(1) insertion at tail of list
	struct candidate* new = malloc(sizeof(struct candidate));
	*new = *c; //copy candidate to new node
	new->prev = NULL;
	new->next = NULL;
	
	if(Parliament.members==NULL){
		Parliament.members = new;
	}else{
		lastInserted->next = new;
	}
	lastInserted=new;
}

//finds candidate with most votes in an array of 5--used in form_parliament()
int findMax(struct candidate* c[5]){
	struct candidate* max = c[0];
	int pos=0;
	for(int i=1;i<5;i++){
		if(c[i]!=NULL &&c[i]->votes>max->votes){ //if c[i]==NULL, it means that list has been exhausted--ignore
			max = c[i];
			pos=i;
		}
	}

	return pos;
}

//inserts candidate c in elected list of their party, sorted insert
void insertElected(struct candidate* c){
	int pos = partyLookup(c->pid);
	struct candidate* new = malloc(sizeof(struct candidate));
	*new = *c; //copy candidate to new node
	new->prev = NULL;
	if(Parties[pos].elected==NULL){ //if elected list is empty, make candidate head
		new->next=NULL;
		Parties[pos].elected = new;
	}else{
		struct candidate* p = Parties[pos].elected;

		struct candidate* prev;
		while(p!=NULL && p->votes>c->votes){ //find first elected candidate that has less votes 
			prev = p;
			p=p->next;
		}
		//insert before candidate
		if(p==Parties[pos].elected){ 
			new->next = p;
			Parties[pos].elected = new;
		}else{
			new->next = prev->next;
			prev->next = new;
		}
	}
}

/*searches for district with given did in array
 @param did: did of the district
 @returns: number in range [0,55] if found
 @returns: 56 if not found*/
int districtLookup(int did){
	int i=0;
	while(i<56 && Districts[i].did!=did) i++;

	return i; //if not found, i==56 at exit of the loop
}

/*searches for party with given pid in array
 @param pid: pid of the party
 @returns: number in range [0,4] if found
 @returns: 5 if not found*/
int partyLookup(int pid){
	int i=0;
	while(i<5 && Parties[i].pid!=pid) i++;

	return i;
}

/*searches for station with given sid in given stations list
  @param struct station*: head of the stations list 
  @param sid: sid of the station
  @returns: pointer to the station, if found
  @returns: NULL, if not found*/
struct station* stationLookup(struct station* station, int sid){
	struct station* p = station;
	while(p!=NULL && p->sid != sid) p=p->next;

	return p;//if not found, p==NULL at exit of loop
}

/*searches for candidate with given cid in given candidates list
  @param struct candidate*: head of candidates list
  @param cid: cid of the candidate
  @returns: pointer to candidate, if found
  @returns: NULL, if not found
*/
struct candidate* candidateLookup(struct candidate* candidate, int cid){
	struct candidate* p = candidate;
	while(p!=NULL && p->cid != cid) p=p->next;
	return p;
}

/*searches for voter with given vid in given voters list
  @param station: head to the stations list--the station we wanna look at
  @param vid: vid of the voter
  @returns: pointer to voter, if found
  @returns: NULL, if not found
*/
struct voter* voterLookup(struct station* station, int vid){
	struct voter* p = station->voters;
	station->vsentinel->vid = vid;

	while(p->vid!=vid) p=p->next;

	if(p!=station->vsentinel)return p;
	return NULL;
}

struct party findMaxParty(){
	struct party maxParty = Parties[0];
	for(int i=1;i<5;i++){
		if(Parties[i].elected > maxParty.elected){
			maxParty = Parties[i];
		}
	}

	return maxParty;
}

/*initializes all arrays. If field is int, init to -1. If field is pointer, init to NULL*/
void announce_elections(void){
    Parliament.members=NULL;

    for(int i=0;i<56;i++){  
        Districts[i].did=-1;
        Districts[i].seats=-1; 
        Districts[i].allotted=-1; 
        Districts[i].blanks=-1;
        Districts[i].voids=-1;
        Districts[i].stations=NULL;
        Districts[i].candidates=NULL;
    }

    for(int i=0;i<5;i++){  
        Parties[i].pid=-1;
	    Parties[i].nelected=-1;
	    Parties[i].elected=NULL;
    }

	printf("A\nDONE\n");
}

/*Creates a new district and adds it to the next available slot in array. 
  @param did: did of new district
  @param seats: available seats in district	
  @return: 0, on success
  @return: 1, if array is full or other failure
*/
int create_district(int did, int seats){
	static int nextAvailable = 0; //to remember next available slot in Districts[], so that we don't search every time for the next available slot 
	if(nextAvailable==56) return 1; 

	Districts[nextAvailable].did=did;
	Districts[nextAvailable].seats=seats;
	Districts[nextAvailable].allotted=0;
	Districts[nextAvailable].blanks=0;
	Districts[nextAvailable].voids=0;
	nextAvailable++;

	printf("D <%d> <%d>\nDistricts: ", did, seats);
	for(int i=0;i<nextAvailable;i++){
		printf("<%d>",Districts[i].did);
		 
	}
	printf("\nDONE\n");

	return 0;
}

/*Creates a new station in given district and adds it to the head of the corresponding stations list. 
  @param did: did of district
  @param sid: sid of new station
  @return: 0, on success
  @return: 1, if district with given did is not found
*/
int create_station(int sid, int did){
	int i=districtLookup(did);
	if(i==56) return 1; 
	
	struct station* s = malloc(sizeof(struct station)); //create new station
	s->registered=0;
	s->sid=sid;
	s->vsentinel = malloc(sizeof(struct voter)); //voters list initially includes only sentinel node
	s->voters =  s->vsentinel;
	s->next = Districts[i].stations;
	Districts[i].stations = s; //insert at head, for smaller complexity

	printf("S <%d> <%d>\nStations: ", sid, did);
	struct station* p = Districts[i].stations;
	while(p!=NULL){
		printf("<%d>", p->sid);
		p=p->next;
	}
	printf("\nDONE\n");
	return 0;
}

/*Creates a new party and adds it to the next available slot in array. 
  @param pid: pid of new party
  @return: 0, on success
  @return: 1, if array is full or other failure
*/
void create_party(int pid){
	static int nextAvailable=0; //same technique as in create_district()
	if(nextAvailable==5) return; //if Parties[] is full, return error
	Parties[nextAvailable].pid=pid;
	Parties[nextAvailable].nelected=0;
	Parties[nextAvailable].elected=NULL;
	nextAvailable++;

	printf("P <%d>\nParties: ", pid);
	for(int i=0;i<nextAvailable;i++){
		printf("<%d>",Parties[i].pid);
		 
	}
	printf("\nDONE\n");

}

/*Creates a new candidate that belongs to given party and to given district and adds it to head of corresponding candidates list
  @param cid: cid of new candidate
  @param did: did of district candidate belongs to
  @param pid: pid of party candidate belongs to
  @returns: 0, on succes
  @returns: 1, if district or party is not found
*/
int register_candidate(int cid, int did, int pid){
	int i=districtLookup(did);
	if(i==56) return 1; //district not found
	int j=partyLookup(pid);
	if(j==5) return 1; //party not found
	struct candidate* c = malloc(sizeof(struct candidate)); //create new candidate
	c->cid=cid;
	c->pid=pid;
	c->votes=0;
	c->elected=0;
	//since all candidates are created in the beginning, we can insert at head for smaller complexity
	c->next = Districts[i].candidates;
	if(c->next!=NULL) c->next->prev = c; //if is not first candidate
	c->prev = NULL;
	Districts[i].candidates = c;

	printf("C <%d> <%d> <%d>\nCandidates: ", cid, did, pid);
	for(struct candidate*p=Districts[i].candidates;p!=NULL;p=p->next){
		printf("<%d>", p->cid);
	}
	printf("\nDONE\n");

	return 0;
}

/* Creates a voter in given station of given district and adds it to head of corresponding list
	@param vid: vid of new voter
	@param did: did of district voter belongs to
	@param sid: sid of station voter belongs to
	@returns: 0, on success
	@returns: 1, if district or station not found
*/
int register_voter(int vid, int did, int sid){
	int i=districtLookup(did);
	if(i==56) return 1; //if i==56, District with D.did==did was not found
	struct station* s = stationLookup(Districts[i].stations, sid);
	if(s==NULL) return 1; //if s==NULL, Station with S.sid==sid was not found

	struct voter* v = malloc(sizeof(struct voter)); //create new voter
	v->vid=vid;
	v->next=s->voters;
	s->voters=v;

	s->registered++; //increment registered voters in the station
	printf("R <%d> <%d> <%d>\nVoters: ", vid, did, sid);
	for(struct voter*p=s->voters;p!=NULL;p=p->next){
		if(p!=s->vsentinel) printf("<%d>", p->vid); //we don't wanna print sentinel node, as it will be <0>
	}
	printf("\nDONE\n");

	return 0;
}


/* Deletes voter from corresponding list
	@param vid: vid of voter
	@returns: 0, on success
	@returns: 1, if voter not found
*/
int unregister_voter(int vid){
	
	int i=0,found=0;
	struct station* s;
	struct voter* v=NULL;
	struct voter* prev;

	while(i<56 && found==0){  //iterate all districts until found
		s=Districts[i].stations;
		while(s!=NULL && found==0){ //iterate all nonempty stations until found
			v = s->voters;
			s->vsentinel->vid = vid;
			while(v->vid!=vid){ //iterate voter list of current station
				prev = v; //hold previous node-->connect it to next node of deleted node
				v=v->next;
			}

			if(v!=s->vsentinel){ //if voter was found before sentinel, update found and end loop
				found=1;
			}else{
				s=s->next;
			}
		}
		
		if(found==0) i++; //if not found, go to next district
	}

	if(i==56) return 1; //voter not found in any district

	if(v==s->voters) s->voters=s->voters->next; //if voter was found in head of voters list
	else prev->next = v->next; //else just connect previous node to next node of voter
	free(v); //free memory

	return 0;
}

/* Deletes all empty stations from all districts*/
void delete_empty_stations(){
	printf("E\n");
	struct station* s, *prev;

	for(int i=0;i<56;i++){
		s = Districts[i].stations;
		while(s!=NULL){
			if(s->voters!=s->vsentinel){
				prev=s;
				s=s->next;
			}else{
				printf("<%d> <%d>\n", s->sid, Districts[i].did);
				if(s==Districts[i].stations){
					Districts[i].stations = s->next;
				}else{
					prev->next = s->next;
				}
				free(s);
				s = prev->next;
			}
		}
	}

	printf("DONE\n");
}


/*Saves the vote of a registered voter of the given station
	@param vid: voter's vid
	@param sid: station sid the voter belongs to
	@param cid: cid of voted candidate
	@returns: 0, on success
	@returns: 1, if voter was not found
*/
int vote (int vid, int sid, int cid){
	struct station* s=NULL;
	int i=0;
	while(i<56 && s==NULL){
		s = stationLookup(Districts[i].stations,sid);
		if(s==NULL) i++;
	}
	if(s==NULL) return 1; //station doesn't exist, no point going further
	struct voter* v = voterLookup(s, vid);
	if(v==NULL) return 1; //voter doesn't exist

	v->voted = 1;
	if(cid==0) Districts[i].blanks++;
	else if(cid==1) Districts[i].voids++;
	else{
		struct candidate* c = candidateLookup(Districts[i].candidates,cid);
		if(c==NULL) return 1; //candidate was not found
		c->votes++;
		//search for first candidate with more votes
		struct candidate* p = c->prev;
		while(p!=NULL && c->votes>p->votes) {
			p=p->prev;
		}
		//insert candidate before last candidate that had less votes
		insertCandidate(&(Districts[i].candidates),&c,&p);
		//delete candidate from previous position
		deleteCandidate(&c);
		
	}

	printf("V <%d> <%d> <%d>\n", vid, sid, cid);
	printf("District: <%d>\n", Districts[i].did);
	printf("Candidate votes:");
	for(struct candidate* c=Districts[i].candidates;c!=NULL;c=c->next){
		printf("(<%d>,<%d>)", c->cid, c->votes);
		if(c->next!=NULL) printf(",");
		else printf("\n");
	}
	printf("Blanks: <%d>\n", Districts[i].blanks);
	printf("Voids:<%d>\n", Districts[i].voids);
	printf("DONE\n");
	return 0;
}


/*Counts votes of all parties in given district and elects the first electedSeats candidates from each party in district*/
void count_votes(int did){
	int d = districtLookup(did);
	int pos;
	if(d==56) return; //district not found

	int validVotes=0; //counts total valid votes in district
	int partyVotes[5] = {0,0,0,0,0}; //array to store total votes of each party, init to zero
	int electedSeats[5];
	struct candidate* c = Districts[d].candidates;
	while(c!=NULL){
		pos = partyLookup(c->pid); //look for where candidate party is in original parties array, so that it is placed correctly in partyVotes
		partyVotes[pos] += c->votes; //increment total party votes by current candidate's votes
		validVotes += c->votes; //increment total valid votes by current candidate's votes
		c=c->next;
	}

	
	double eklogikoMetro = (double)validVotes/(double)Districts[d].seats;
	for(int i=0;i<5;i++){
		electedSeats[i] = (int) (partyVotes[i]/eklogikoMetro);
	}

	c = Districts[d].candidates;
	while(c!=NULL){
		pos = partyLookup(c->pid); //find part in array--parties are in the same array position in Parties, partyVotes and electedSeats
		if(electedSeats[pos]>0){
			c->elected = 1; //elect
			Parties[pos].nelected++; //increment number of party's elected candidates
			Districts[d].allotted++; //increment alloted seats of district
			electedSeats[pos]--; //decrement elected seats of party

			insertElected(c);
		}
		c=c->next;
	}

	printf("M <%d>\n", did);
	printf("Seats:\n");
	c = Districts[d].candidates;
	while(c!=NULL){
		if(c->elected==1){
			printf("<%d> <%d> <%d>\n", c->cid, c->pid, c->votes);
		}
		c=c->next;
	}
	printf("DONE\n");
}

/*For every district with leftover seats, it first elects the candidates of the party with most votes across all stations, and if 
  party doesn't have enough candidates to fill leftover seats, it elects candidates of the district, starting from highest vote count
*/
void form_government(){
	struct party maxParty = findMaxParty(); //find party with most votes across all districts
	int leftoverSeats;

	printf("G\n");
	printf("Seats:\n");
	for(int i=0;i<56;i++){
		leftoverSeats = Districts[i].seats - Districts[i].allotted;
		struct candidate* c = Districts[i].candidates;
		while(c!=NULL && leftoverSeats>0){//elect all unelected candidates of most voted party
			if(c->pid == maxParty.pid && c->elected==0){
				c->elected = 1;
				leftoverSeats--;
				insertElected(c);
				printf("<%d> <%d> <%d>\n", Districts[i].did, c->cid, c->votes);
			}
			c=c->next;
		}

		c = Districts[i].candidates;
		while(c!=NULL && leftoverSeats>0){ //if district has more leftover seats, elect candidates starting from most voted, until no more leftover seats
			if(c->elected==0){
				c->elected = 1;
				leftoverSeats--;
				insertElected(c);
				printf("<%d> <%d> <%d>\n", Districts[i].did, c->cid, c->votes);
			}
			c=c->next;
		}
	}

	printf("DONE\n");
	return;
	
}

/*Forms the parliament members list, in descending order based on votes*/
void form_parliament(){

	struct candidate* c[5] = {Parties[0].elected, Parties[1].elected, Parties[2].elected, Parties[3].elected, Parties[4].elected};

	/*Hold head of all candidate lists in helper array
	  find max voted candidate from current nodes in array
	  move to next node on the list that included that candidate
	  repeat until all lists are iterated
	*/
	while(c[0]!=NULL || c[1]!=NULL || c[2]!=NULL || c[3]!=NULL || c[4]!=NULL){
		int max = findMax(c); //O(1) -- always iterate through input of size 5
		insertMember(c[max]); //O(1) -- has pointer to last inserted node 
		c[max] = c[max]->next;
		//this whole loop is repeated until all n=300 elected members have been iterated, i.e until all lists have been exhausted
		//O(n)
	}

	printf("N\n");
	printf("Members: \n");
	for(struct candidate* c=Parliament.members;c!=NULL;c=c->next){
		printf("<%d> <%d> <%d>\n", c->cid, c->pid, c->votes);
	}
	printf("DONE\n");
	return;
	
}


//prints data of a district
void print_district(int did){

	int pos = districtLookup(did);
	if(pos==56) {
		printf("District not found!\n");
		return;
	} 
	printf("I <%d>\n", did);
	printf("Seats: <%d>\n", Districts[pos].seats);
	printf("Blanks: <%d>\n", Districts[pos].blanks);
	printf("Voids: <%d>\n", Districts[pos].voids);
	printf("Candidates: \n");
	for(struct candidate* c = Districts[pos].candidates;c!=NULL;c=c->next){
		printf("<%d> <%d> <%d> \n", c->cid, c->pid, c->votes);
	}
	printf("Stations: ");
	for(struct station* s = Districts[pos].stations;s!=NULL;s=s->next){
		printf("<%d>",s->sid);
		if(s->next!=NULL) printf(", ");
		else printf("\n");
		
	}
	printf("DONE\n");
	return;
}

//prints data of given station of given district
void print_station(int sid, int did){
	int pos = districtLookup(did);
	if(did==56){
		printf("District not found!\n");
		return;
	}

	struct station* s = stationLookup(Districts[pos].stations,sid);
	if(s==NULL) {
		printf("Station not found!\n");
		return;
	}

	printf("J <%d>\n", sid);
	printf("Registered = <%d>\n", s->registered);
	printf("Voters:\n");
	for(struct voter* v=s->voters;v!=NULL;v=v->next){
		printf("<%d> <%d>\n", v->vid, v->voted);
	}
	printf("DONE\n");
	return;
}

//prints data of a party
void print_party(int pid){
	int pos = partyLookup(pid);
	if(pos==5){
		printf("Party not found!\n");
		return;
	}

	printf("K <%d>\nElected:\n", pid);
	for(struct candidate* c=Parties[pos].elected;c!=NULL;c=c->next){
		printf("<%d> <%d>\n", c->cid, c->votes);
	}
	printf("DONE\n");
	return;
}

//prints data of parliament
void print_parliament(){
	if(Parliament.members==NULL){
		printf("Parliament has no members yet!\n");
		return;
	}

	printf("L\nMembers:\n");
	for(struct candidate* c=Parliament.members;c!=NULL;c=c->next){
		printf("<%d> <%d> <%d>\n", c->cid, c->pid, c->votes);
	}

	printf("DONE\n");
	return;
}

//destroys candidates list of a district
void destroyCandidates(struct candidate** head){

	struct candidate* tmp, *c=*head;

	while(c!=NULL){
		tmp = c->next; //first save next node, else it will be lost after freeing c
		free(c); //free
		c = tmp; //make c point to next node
	}
	
	*head=NULL; //change head of list to NULL, so that main knows it is empty and doesn't try to iterate it => segmentation fault / iteration over garbage
}

//destroys voters list of a station
void destroyVoters(struct voter** head){

	struct voter* tmp, *c=*head;

	while(c!=NULL){
		tmp = c->next;
		free(c);
		c = tmp;
	}

	*head=NULL;
}

//destroys stations list of district
void destroyStations(struct station** head){

	struct station* tmp, *c=*head;

	while(c!=NULL){
		destroyVoters(&(c->voters));
		tmp = c->next;
		free(c);
		c = tmp;
	}

	*head=NULL;
}
