#include "voting.h"
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

// Enable in Makefile
#ifdef DEBUG_PRINTS_ENABLED
#define DebugPrint(...) printf(__VA_ARGS__);
#else
#define DebugPrint(...)
#endif

#define PRIMES_SZ 1024
#define DISTRICTS_SZ 56
#define PARTIES_SZ 5

typedef struct District District;
typedef struct Station Station;
typedef struct Voter Voter;
typedef struct Party Party;
typedef struct Candidate Candidate;
typedef struct ElectedCandidate ElectedCandidate;

struct District {
    int did;
    int seats;
    int blanks;
    int invalids;
    int partyVotes[PARTIES_SZ];
};

struct Station {
    int sid;
    int did;
    int registered;
    Voter* voters;
    Station* next;
};
struct Voter {
    int vid;
    bool voted;
    Voter* parent;
    Voter* lc;
    Voter* rc;
};

struct Party {
    int pid;
    int electedCount;
    Candidate* candidates;
};
struct Candidate {
    int cid;
    int did;
    int votes;
    bool isElected;
    Candidate* lc;
    Candidate* rc;
};

struct ElectedCandidate {
    int cid;
    int did;
    int pid;
    ElectedCandidate* next;
};

District Districts[DISTRICTS_SZ];
Station** StationsHT;
Party Parties[PARTIES_SZ];
ElectedCandidate* Parliament;

const int DefaultDid = -1;
const int BlankDid = -1;
const int InvalidDid = -2;

const int Primes[PRIMES_SZ] = {
    0, 1, 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 307, 311, 313, 317, 331, 337, 347, 349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409, 419, 421, 431, 433, 439, 443, 449, 457, 461, 463, 467, 479, 487, 491, 499, 503, 509, 521, 523, 541, 547, 557, 563, 569, 571, 577, 587, 593, 599, 601, 607, 613, 617, 619, 631, 641, 643, 647, 653, 659, 661, 673, 677, 683, 691, 701, 709, 719, 727, 733, 739, 743, 751, 757, 761, 769, 773, 787, 797, 809, 811, 821, 823, 827, 829, 839, 853, 857, 859, 863, 877, 881, 883, 887, 907, 911, 919, 929, 937, 941, 947, 953, 967, 971, 977, 983, 991, 997, 1009, 1013, 1019, 1021, 1031, 1033, 1039, 1049, 1051, 1061, 1063, 1069, 1087, 1091, 1093, 1097, 1103, 1109, 1117, 1123, 1129, 1151, 1153, 1163, 1171, 1181, 1187, 1193, 1201, 1213, 1217, 1223, 1229, 1231, 1237, 1249, 1259, 1277, 1279, 1283, 1289, 1291, 1297, 1301, 1303, 1307, 1319, 1321, 1327, 1361, 1367, 1373, 1381, 1399, 1409, 1423, 1427, 1429, 1433, 1439, 1447, 1451, 1453, 1459, 1471, 1481, 1483, 1487, 1489, 1493, 1499, 1511, 1523, 1531, 1543, 1549, 1553, 1559, 1567, 1571, 1579, 1583, 1597, 1601, 1607, 1609, 1613, 1619, 1621, 1627, 1637, 1657, 1663, 1667, 1669, 1693, 1697, 1699, 1709, 1721, 1723, 1733, 1741, 1747, 1753, 1759, 1777, 1783, 1787, 1789, 1801, 1811, 1823, 1831, 1847, 1861, 1867, 1871, 1873, 1877, 1879, 1889, 1901, 1907, 1913, 1931, 1933, 1949, 1951, 1973, 1979, 1987, 1993, 1997, 1999, 2003, 2011, 2017, 2027, 2029, 2039, 2053, 2063, 2069, 2081, 2083, 2087, 2089, 2099, 2111, 2113, 2129, 2131, 2137, 2141, 2143, 2153, 2161, 2179, 2203, 2207, 2213, 2221, 2237, 2239, 2243, 2251, 2267, 2269, 2273, 2281, 2287, 2293, 2297, 2309, 2311, 2333, 2339, 2341, 2347, 2351, 2357, 2371, 2377, 2381, 2383, 2389, 2393, 2399, 2411, 2417, 2423, 2437, 2441, 2447, 2459, 2467, 2473, 2477, 2503, 2521, 2531, 2539, 2543, 2549, 2551, 2557, 2579, 2591, 2593, 2609, 2617, 2621, 2633, 2647, 2657, 2659, 2663, 2671, 2677, 2683, 2687, 2689, 2693, 2699, 2707, 2711, 2713, 2719, 2729, 2731, 2741, 2749, 2753, 2767, 2777, 2789, 2791, 2797, 2801, 2803, 2819, 2833, 2837, 2843, 2851, 2857, 2861, 2879, 2887, 2897, 2903, 2909, 2917, 2927, 2939, 2953, 2957, 2963, 2969, 2971, 2999, 3001, 3011, 3019, 3023, 3037, 3041, 3049, 3061, 3067, 3079, 3083, 3089, 3109, 3119, 3121, 3137, 3163, 3167, 3169, 3181, 3187, 3191, 3203, 3209, 3217, 3221, 3229, 3251, 3253, 3257, 3259, 3271, 3299, 3301, 3307, 3313, 3319, 3323, 3329, 3331, 3343, 3347, 3359, 3361, 3371, 3373, 3389, 3391, 3407, 3413, 3433, 3449, 3457, 3461, 3463, 3467, 3469, 3491, 3499, 3511, 3517, 3527, 3529, 3533, 3539, 3541, 3547, 3557, 3559, 3571, 3581, 3583, 3593, 3607, 3613, 3617, 3623, 3631, 3637, 3643, 3659, 3671, 3673, 3677, 3691, 3697, 3701, 3709, 3719, 3727, 3733, 3739, 3761, 3767, 3769, 3779, 3793, 3797, 3803, 3821, 3823, 3833, 3847, 3851, 3853, 3863, 3877, 3881, 3889, 3907, 3911, 3917, 3919, 3923, 3929, 3931, 3943, 3947, 3967, 3989, 4001, 4003, 4007, 4013, 4019, 4021, 4027, 4049, 4051, 4057, 4073, 4079, 4091, 4093, 4099, 4111, 4127, 4129, 4133, 4139, 4153, 4157, 4159, 4177, 4201, 4211, 4217, 4219, 4229, 4231, 4241, 4243, 4253, 4259, 4261, 4271, 4273, 4283, 4289, 4297, 4327, 4337, 4339, 4349, 4357, 4363, 4373, 4391, 4397, 4409, 4421, 4423, 4441, 4447, 4451, 4457, 4463, 4481, 4483, 4493, 4507, 4513, 4517, 4519, 4523, 4547, 4549, 4561, 4567, 4583, 4591, 4597, 4603, 4621, 4637, 4639, 4643, 4649, 4651, 4657, 4663, 4673, 4679, 4691, 4703, 4721, 4723, 4729, 4733, 4751, 4759, 4783, 4787, 4789, 4793, 4799, 4801, 4813, 4817, 4831, 4861, 4871, 4877, 4889, 4903, 4909, 4919, 4931, 4933, 4937, 4943, 4951, 4957, 4967, 4969, 4973, 4987, 4993, 4999, 5003, 5009, 5011, 5021, 5023, 5039, 5051, 5059, 5077, 5081, 5087, 5099, 5101, 5107, 5113, 5119, 5147, 5153, 5167, 5171, 5179, 5189, 5197, 5209, 5227, 5231, 5233, 5237, 5261, 5273, 5279, 5281, 5297, 5303, 5309, 5323, 5333, 5347, 5351, 5381, 5387, 5393, 5399, 5407, 5413, 5417, 5419, 5431, 5437, 5441, 5443, 5449, 5471, 5477, 5479, 5483, 5501, 5503, 5507, 5519, 5521, 5527, 5531, 5557, 5563, 5569, 5573, 5581, 5591, 5623, 5639, 5641, 5647, 5651, 5653, 5657, 5659, 5669, 5683, 5689, 5693, 5701, 5711, 5717, 5737, 5741, 5743, 5749, 5779, 5783, 5791, 5801, 5807, 5813, 5821, 5827, 5839, 5843, 5849, 5851, 5857, 5861, 5867, 5869, 5879, 5881, 5897, 5903, 5923, 5927, 5939, 5953, 5981, 5987, 6007, 6011, 6029, 6037, 6043, 6047, 6053, 6067, 6073, 6079, 6089, 6091, 6101, 6113, 6121, 6131, 6133, 6143, 6151, 6163, 6173, 6197, 6199, 6203, 6211, 6217, 6221, 6229, 6247, 6257, 6263, 6269, 6271, 6277, 6287, 6299, 6301, 6311, 6317, 6323, 6329, 6337, 6343, 6353, 6359, 6361, 6367, 6373, 6379, 6389, 6397, 6421, 6427, 6449, 6451, 6469, 6473, 6481, 6491, 6521, 6529, 6547, 6551, 6553, 6563, 6569, 6571, 6577, 6581, 6599, 6607, 6619, 6637, 6653, 6659, 6661, 6673, 6679, 6689, 6691, 6701, 6703, 6709, 6719, 6733, 6737, 6761, 6763, 6779, 6781, 6791, 6793, 6803, 6823, 6827, 6829, 6833, 6841, 6857, 6863, 6869, 6871, 6883, 6899, 6907, 6911, 6917, 6947, 6949, 6959, 6961, 6967, 6971, 6977, 6983, 6991, 6997, 7001, 7013, 7019, 7027, 7039, 7043, 7057, 7069, 7079, 7103, 7109, 7121, 7127, 7129, 7151, 7159, 7177, 7187, 7193, 7207, 7211, 7213, 7219, 7229, 7237, 7243, 7247, 7253, 7283, 7297, 7307, 7309, 7321, 7331, 7333, 7349, 7351, 7369, 7393, 7411, 7417, 7433, 7451, 7457, 7459, 7477, 7481, 7487, 7489, 7499, 7507, 7517, 7523, 7529, 7537, 7541, 7547, 7549, 7559, 7561, 7573, 7577, 7583, 7589, 7591, 7603, 7607, 7621, 7639, 7643, 7649, 7669, 7673, 7681, 7687, 7691, 7699, 7703, 7717, 7723, 7727, 7741, 7753, 7757, 7759, 7789, 7793, 7817, 7823, 7829, 7841, 7853, 7867, 7873, 7877, 7879, 7883, 7901, 7907, 7919, 7927, 7933, 7937, 7949, 7951, 7963, 7993, 8009, 8011, 8017, 8039, 8053, 8059, 8069, 8081, 8087, 8089, 8093, 8101, 8111, 8117, 8123
};
int MaxStationsCount;
int MaxSid;


int a,b,size,p=0; //hash table size and hash function coefficients



//HELPER FUNCTIONS ---- START

/**
 * Universal hash function of stations
 * @param key: the key to be hashed
 */
int StationHash(int key){  
    
    return ((a*key + b)%p)%size;  
    
}


/**
 * Collection of lookup functions 
 */
int districtLookup(int did){
	int i=0;
	while(i<DISTRICTS_SZ && Districts[i].did!=did) i++;

	return i; //if not found, i==56 at exit of the loop
}

Station* stationLookup(int sid){

    int chain = StationHash(sid);
    Station* s = StationsHT[chain];

    while(s!=NULL && s->sid!=sid) s=s->next;

    return s;//will become NULL if it isn't found
}

Voter* voterLookup(Voter* root, int vid){
    Voter* p = root;

    if(p==NULL) return NULL;

    if(p->vid==vid) return p;
    Voter* left = voterLookup(root->lc, vid);
    if(left) return left;
    Voter* right = voterLookup(root->rc, vid);
    
    return right;

}

Candidate* candidateLookup(Candidate* root, int cid){
    Candidate* p = root;

    if(p==NULL) return NULL;

    if(p->cid==cid) return p;
    Candidate* left = candidateLookup(root->lc, cid);
    if(left) return left;
    Candidate* right = candidateLookup(root->rc, cid);
    
    return right;
}

/**
 * Collection of functions for inorder traversal of the binary trees
 */
void InOrderCandidates(Candidate* root){
    if(root==NULL) return;

    InOrderCandidates(root->lc);
    printf("<%d> ", root->cid);
    InOrderCandidates(root->rc);
}

void InOrderVoters(Voter* root){

    if(root==NULL) return;

    InOrderVoters(root->lc);
    printf("<%d> ", root->vid);
    InOrderVoters(root->rc);
}

int heap_sz, curr_sz=0; //MAX heap size, current heap size

/**
 * Inserts new candidate into minheap
 * @param c: candidate to be inserted
 * @param heap: root of heap
 */
void HeapInsert(Candidate* c, Candidate* heap[]){

    if(curr_sz==heap_sz) return;

    int m=curr_sz;

    while(m>0 && c->votes < heap[(m-1)/2]->votes){
        heap[m] = heap[(m-1)/2];
        m = (m-1)/2;
    }

    heap[m] = c;
    curr_sz++;
}

//FREE TREES
void freeTreeVoters(Voter** root){

    if(*root==NULL) return;

    freeTreeVoters(&((*root)->lc));
    freeTreeVoters(&((*root)->rc));
    free(*root);
}

void freeTreeCandidates(Candidate** root){

    if(*root==NULL) return;

    freeTreeCandidates(&((*root)->lc));
    freeTreeCandidates(&((*root)->rc));
    free(*root);
}

/**
 * Initializes the heap with the first partyElected candidates of District
 * @param root: root of candidate tree
 * @param did: did of district
 * @param heap: root of heap
 */
void InitHeap(Candidate* root, int did, Candidate* heap[]){

    if(root==NULL) return;

    if(root->did==did){
        HeapInsert(root,heap);
    }

    InitHeap(root->lc,did, heap);
    InitHeap(root->rc, did, heap);

}

/**
 * Algorithm that deletes the root of the heap
 * @param heap: root of heap
 */
void DeleteMin(Candidate* heap[]){

    if(curr_sz==0) return;

    Candidate* key = heap[curr_sz-1];
    curr_sz--;
    if(curr_sz==1) {
        heap[0] = heap[1];
        return;
    }

    int m = 0;
    int p;

    while((2*m+1 < curr_sz && key->votes > heap[2*m+1]->votes) || 
          (2*m+2 < curr_sz && key->votes > heap[2*m+1]->votes)){

        if(2*m+2 < heap_sz) {
            if(heap[2*m+1]->votes < heap[2*m+2]->votes) p = 2*m+1;
            else p = 2*m+2;
        }else p = curr_sz-1;
        heap[m] = heap[p];
        m=p;
    }

    heap[m] = key;
}

/**
 * Algorithm that traverses the tree and if it finds a candidate of given district with more votes than the root of heap :
 * -Deletes root (i.e current candidate with least votes)
 * -Inserts the new candidate into the heap
 * @param root: root of candidate tree
 * @param did: did of district
 * @param heap: root of heap
 */
void InsertCandidates(Candidate* root, int did, Candidate* heap[]){

    if(root==NULL || heap[0]==NULL) return;

    if(root->did==did && heap[0]->votes < root->votes){
        DeleteMin(heap);
        HeapInsert(root,heap);
    }

    InsertCandidates(root->lc, did, heap);
    InsertCandidates(root->rc, did, heap);
}

/**
 * Algorithm that elects candidates of given party in given district, using minHeap
 * @param pid: Party of candidates to be elected
 * @param did: District they belong to
 * @param partyElected: MAX size of heap, i.e seats allocated to party
 */
void ElectPartyCandidatesInDistrict(int pid, int did, int partyElected){

    heap_sz = partyElected;
    curr_sz = 0;
    Candidate** elected = malloc(heap_sz*sizeof(Candidate*));

    //initialize heap
    InitHeap(Parties[pid].candidates,did, elected);
    //iterate tree and add candidates to be elected
    InsertCandidates(Parties[pid].candidates, did, elected);

    for(int i=0;i<curr_sz;i++){
        elected[i]->isElected = true;
        printf("CID: %d, PID: %d, DID: %d\n",elected[i]->cid, pid, elected[i]->did);
    }

}





/**
 * Other helper functions
*/

/**
 * Helper function - binary searches first District[i] with did=-1 (empty slot)
 * @param low: bottom end of array
 * @param high: high end of array
 * @returns position of first empty slot in Districts array or -1 if it's full
 */
int BSearch(int low, int high){

    if(low>high) return -1; //array is full

    int mid = (low+high)/2;

    if(Districts[mid].did==-1){

        if(mid==0 || Districts[mid-1].did!=-1) return mid; //if Districts[0] empty or we've found an element with did=-1 and prev element did!=-1, it means that all before prev are occupied, return element
        else return(BSearch(low,mid-1)); //else there may be another empty element before it, so search left part

    }else{
        return BSearch(mid+1,high); //if middle element is occupied, all leftmost are, need to look right
    }

}

/**
 * Helper function. Initializes hash table and hash function (global) parameters
 * @param max_cnt: the maximum possible number of elements to be stored
 * @param max_key: the biggest possible key to be stored
 */

void initHash(int max_cnt, int max_key){
    
    int loadfactor = 4; //4 elements on average in each chain of the hash table, for O(1) complexity. 
                        //This ensures that chains have a few elements, but also that the hash table stays relatively small (1/4 of max_cnt) so there are not any unused slots
                        //due to collisions
    
    size = (max_cnt<4) ? 1 : max_cnt / loadfactor ;// solve equation: loadfactor = max_count / ht_size for ht_size to find number of chains needed
                                                   //if max_cnt<4 then size<0 (we don't want that), so create only one chain in the table

    StationsHT = malloc(size*sizeof(Station*)); //allocate stations ht with size number of chains

    for(int i=0;i<size;i++){
        StationsHT[i] = NULL; //init chains as empty
    }

    int i =0;
    while(i<PRIMES_SZ && (p=Primes[i])<=max_key) i++; //choose p for hash function: next prime number after maxSid
    if(i==PRIMES_SZ){
        printf("Max element too big!\n");
        return;
    }

    //init hash function coefficients randomly 
    srand(time(NULL));
    a = rand()%(p-1)+1; //generate random a that belongs to [1,p-1]
    b = rand()%(p-1); //generate random b that belongs to [0,p-1]
}

/**
 * Helper function - initializes districts array as follows:
 * did = -1
 * blanks,invalids = 0
 * seats=0
 * partyVotes[i] = 0 for every 0 <= i < PARTIES_SZ
 */
void initDistricts(){

    for(int i=0;i<DISTRICTS_SZ;i++){
        Districts[i].did = DefaultDid;
        Districts[i].blanks = 0;
        Districts[i].invalids = 0;
        Districts[i].seats = 0;

        for(int j=0;j<PARTIES_SZ;j++) Districts[i].partyVotes[j] = 0;
    }
}

/**
 * Helper function - initializes parties array as follows:
 * pid = index in array
 * candidates list init as empty
 * electedCount = 0
 */
void initParties(){
    for(int i=0;i<PARTIES_SZ;i++){
        Parties[i].pid = i;
        Parties[i].candidates = NULL;
        Parties[i].electedCount = 0;
    }
}

/**
 * Helper function - finds the parent of the (size+1)-th node of the voters complete tree
 * @param root: root of tree
 * @param size: its current size (before insertion)
 * @returns parent of new node
 */
Voter* findParent(Voter* root, int size){
    Voter* parent = root;
    int height = log2(size+1);
    int min = pow(2,height) -1;
    int max = pow(2, height+1) -2;
    int mid;

    while(parent->lc!=NULL && parent->rc!=NULL){
        mid = (min+max)/2;
        if(size>mid){
            parent = parent->rc;
            min = mid+1;
        }else{
            parent = parent->lc;
            max = mid;
        } 
    }
    return parent;
}

void toList(Candidate* root, ElectedCandidate** head, int pid){

    if(root==NULL) return;

    toList(root->lc, head,pid);
    if(root->isElected==1){
        ElectedCandidate* new = malloc(sizeof(ElectedCandidate));
        new->cid = root->cid;
        new->did = root->did;
        new->next = *head;
        new->pid = pid;
        *head = new;
    }
    toList(root->rc,head,pid);
}



int findMax(ElectedCandidate* lists[]){

    ElectedCandidate* min;
    int pos;

    int i=0;

    while(i<5 && lists[i]==NULL) i++;
    if(i==5) return -1;

    min = lists[i];
    pos=i;

    for(int j=0;j<5;j++){
        if(lists[j] != NULL && min!= NULL && lists[j]->cid > min->cid){
            min = lists[j];
            pos=j;
        }
    }

    return pos;
}
ElectedCandidate* mergeTrees(Candidate* t1, Candidate* t2, Candidate* t3, Candidate* t4, Candidate* t5){

    //creates lists with tree nodes in descending order
    ElectedCandidate* l1=NULL, *l2=NULL, *l3=NULL, *l4=NULL, *l5=NULL, *l=NULL;
    toList(t1, &l1, 0);
    toList(t2, &l2, 1);
    toList(t3, &l3, 2);
    toList(t4, &l4, 3);
    toList(t5, &l5, 4);

   
    ElectedCandidate* lists[5] = {l1,l2,l3,l4,l5};

    while(lists[0]!=NULL || lists[1]!=NULL || lists[2]!=NULL || lists[3]!=NULL || lists[4]!=NULL){
        //find min and insert at head. List creates is gonna have nodes from max to min
        int min = findMax(lists);
        if(min!=-1){
            ElectedCandidate* new = malloc(sizeof(ElectedCandidate));
            new->cid = lists[min]->cid;
            new->did = lists[min]->did;
            new->next = l;
            new->pid = lists[min]->pid;
            l = new;
            lists[min] = lists[min]->next;
        }
    }

    return l;
}


void printCandidates(Candidate* root){

    if(root==NULL) return;

    printCandidates(root->lc);
    printf("CID: %d, Votes: %d\n", root->cid, root->votes);
    printCandidates(root->rc);
}


void printVoters(Voter* root){

    if(root==NULL) return;

    printVoters(root->lc);
    printf("VID: %d, Voted: %d\n", root->vid, root->voted);
    printVoters(root->rc);
}


//HELPER FUNCTIONS ---- END


/**
 * EVENT FUNCTIONS ---- START
 */
/**
 * Event A - initialize Districts array, stations hash function and table, parties array and parliament list
 * @param parsedMaxSid: maximum possible SID , used in hash function
 * @param parsedMaxStationsCount: maximum possible stations number, used to decide hash table size
 */
void EventAnnounceElections(int parsedMaxStationsCount, int parsedMaxSid) {
    printf("A %d %d\n", parsedMaxStationsCount, parsedMaxSid);
    initDistricts();
    initHash(parsedMaxStationsCount,parsedMaxSid);
    initParties();
    Parliament = NULL;
    printf("DONE\n");
}
/**
  *Event D- creates new district and places it in an array
  *It first binary searches first empty spot on table and then inserts it there --> O(logn)
  *@param did: did of new district
  *@param seats: available seats
*/
void EventCreateDistrict(int did, int seats) {
    printf("D %d %d\n", did, seats);

    int empty = BSearch(0,DISTRICTS_SZ);
    if(empty==-1) {
        printf("Array is full!\n");
        return;
    }

    Districts[empty].did = did;
    Districts[empty].seats = seats;

    printf("Districts:");
    for(int i=0;i<DISTRICTS_SZ && Districts[i].did!=-1;i++){
        printf((i==55)?"<%d>":"<%d>,", Districts[i].did);
    }

    printf("\nDONE\n");

}
/**
 * Event S - creates station and adds it appropriately to hash table (in correct chain, sorted insert)
 * @param sid: SID of new station
 * @param did: DID of district it belongs to
 */
void EventCreateStation(int sid, int did) {
    printf("S %d %d\n", sid, did);

    int d = districtLookup(did);

    if(d==DISTRICTS_SZ){
        printf("District with DID: %d not found!\n",did);
        return;
    }

    //allocate new station
    Station* new = malloc(sizeof(Station));
    new->did = did;
    new->sid = sid;
    new->registered = 0;
    new->voters = NULL;
    new->next = NULL ;

    int hashIndex = StationHash(sid); //find where it must be hashed

    //insert in corresponding chain
    if(StationsHT[hashIndex]==NULL) StationsHT[hashIndex] = new;
    else{
        Station* p = StationsHT[hashIndex];
        Station* prev = NULL;
        while(p!=NULL && p->sid<new->sid){
            prev = p;
            p = p->next;
        }

        if(prev==NULL){
            new->next = StationsHT[hashIndex];
            StationsHT[hashIndex] = new;
        }else{
            new->next = prev->next;
            prev->next = new;
        }
    }
    printf("Stations[%d]: ", StationHash(sid));
    for(Station* p=StationsHT[hashIndex];p!=NULL;p=p->next){
        printf("<%d> ", p->sid);
    }
    printf("\nDONE\n");

}

/**
 * Event R- registers a new voter in given station
 * @param vid: VID of new voter
 * @param sid: station they belong to
 */
void EventRegisterVoter(int vid, int sid) {
    printf("R %d %d\n", vid, sid);

    Station* s = stationLookup(sid);

    if(s==NULL){
        printf("Station with SID: %d not found!\n", sid);
        return;
    }

    Voter* new = malloc(sizeof(Voter));
    new->vid = vid;
    new->voted = false;
    new->lc = NULL;
    new->rc = NULL;
    new->parent = NULL;


    //insert new node so that voters tree remains complete
    if(s->registered==0){
        s->voters = new;
    }else{
        Voter* parent = findParent(s->voters, s->registered);
        new->parent = parent;
        if((s->registered+1)%2==0) parent->lc = new;
        else parent->rc = new;
    }
    
    s->registered++;
    printf("Voters[%d]:", sid);
    InOrderVoters(s->voters);
    printf("\nDONE\n");
}

/**
 * Event C - registers a new candidate of given district and given party
 * @param cid: CID of new candidate
 * @param pid: PID of party they belong to
 * @param did: DID of district they belong to
 */
void EventRegisterCandidate(int cid, int pid, int did) {
    printf("C %d %d %d\n", cid, pid, did);

    if(pid>=PARTIES_SZ){
        printf("Out of bounds!\n");
        return;
    }
    if(districtLookup(did)==56){
        printf("District with DID: %d not found!\n", did);
        return;
    }

    Candidate* new = malloc(sizeof(Candidate));
    new->cid = cid;
    new->did = did;
    new->isElected = false;
    new->votes = 0;
    new->lc = NULL;
    new->rc = NULL;
    
    if(Parties[pid].candidates==NULL) Parties[pid].candidates = new;
    else{
        Candidate* root = Parties[pid].candidates;
        Candidate* prev = Parties[pid].candidates;
        while(root!=NULL){
            prev = root;
            if(new->cid > root->cid){
                root = root->rc;
            }else{
                root = root->lc;
            }
        }

        if(new->cid>prev->cid) prev->rc = new;
        else prev->lc = new;
    }

    printf("Candidates[%d]:", pid);
    InOrderCandidates(Parties[pid].candidates);
    printf("\nDONE\n");


}

/**
 * Event V - A voter votes a candidate (not necessarily of his own district)
 * @param vid: Voter's VID
 * @param sid: SID of the station the voter belongs to
 * @param cid: CID of candidate they want to vote
 * @param pid: PID of party candidate belongs to
 */
void EventVote(int vid, int sid, int cid, int pid) {
    printf("V %d %d %d %d\n", vid, sid, cid, pid);

    if(pid>PARTIES_SZ){
        printf("Party not found!\n");
        return;
    }
    Station* s = stationLookup(sid);
    if(s==NULL){
        printf("Station not found!\n");
        return;
    }
    
    int voterDistrict = districtLookup(s->did);
    int candidateDistrict = -1;
    Voter* v = voterLookup(s->voters, vid);
    if(v==NULL){
        printf("Voter not found!\n");
        return;
    }

    v->voted = true;

    if(cid==-1){
        Districts[voterDistrict].blanks++;
    }else if(cid==-2){
        Districts[voterDistrict].invalids++;
    }else if(cid > -1){
        Candidate* c = candidateLookup(Parties[pid].candidates, cid);
        if(c==NULL){
            printf("Candidate not found!\n");
            return;
        }
        candidateDistrict = districtLookup(c->did);
        c->votes++;
        Districts[candidateDistrict].partyVotes[pid]++;
    }else{
        printf("Invalid CID!\n");
        return;
    }

    int d =  (candidateDistrict==-1) ? voterDistrict : candidateDistrict;
    printf("District[%d]:\n",d);
    printf("Blanks: %d\n", Districts[d].blanks);
    printf("Invalids: %d\n", Districts[d].invalids);
    printf("Party Votes: \n");
    for(int i=0;i<PARTIES_SZ;i++){
        printf("pid: <%d>, votes:<%d>\n", i, Districts[d].partyVotes[i]);
    }

    printf("DONE\n");
}

/**
 * Event M - counts votes in a district
 * @param did: District to count votes in
 */
void EventCountVotes(int did) {
    printf("M %d\nSeats:\n", did);

    int district = districtLookup(did);
    if(district==DISTRICTS_SZ) {
        printf("District not found!\n");
        return;
    }

    double electoralQuota=0;
    int sumVotes = 0;
    int partyElected[PARTIES_SZ] = {0,0,0,0,0};

    if(Districts[district].seats>0){
        for(int i=0;i<PARTIES_SZ;i++){
            sumVotes += Districts[district].partyVotes[i];
        }
        electoralQuota = (double)sumVotes/(double)Districts[district].seats;
    }

    for(int i=0;i<PARTIES_SZ;i++){
        if(electoralQuota==0) partyElected[i]=0;
        else partyElected[i] = Districts[district].partyVotes[i]/electoralQuota;

        Parties[i].electedCount += partyElected[i];
        Districts[district].seats -= partyElected[i];
    }

    for(int i=0;i<PARTIES_SZ;i++){
        ElectPartyCandidatesInDistrict(i,did,partyElected[i]);
    }

    printf("DONE\n");
}


/**
 * Event N - forms parliament list
 */
void EventFormParliament(void) {
    printf("N\n");

    Parliament = mergeTrees(Parties[0].candidates, Parties[1].candidates, Parties[2].candidates, Parties[3].candidates, Parties[4].candidates);
    
    printf("Members: \n");
    EventPrintParliament();
}


/**
 * Event I - prints information of a district
 * @param did: DID of desired district
 */
void EventPrintDistrict(int did) {
    printf("I %d\n", did);
    
    int pos = districtLookup(did);
    
    if(pos==56){
        printf("Not found!\n");
        return;
    }

    printf("Seats: %d\n", Districts[pos].seats);
    printf("Blanks: %d\n", Districts[pos].blanks);
    printf("Invalids: %d\n", Districts[pos].invalids);
    printf("Party votes: \n");

    for(int i=0;i<PARTIES_SZ;i++){
        printf("PID: %d, Votes: %d\n", i, Districts[pos].partyVotes[i]);
    }

    printf("DONE\n");
}

/**
 * Event J - prints information of station 
 * @param sid: SID of desired station
 */
void EventPrintStation(int sid) {
    printf("J %d\n", sid);

    Station* s = stationLookup(sid);
    if(s==NULL){
        printf("Not found!\n");
        return;
    }

    printf("Registered: %d\n", s->registered);
    if(s->voters==NULL) printf("No voters!\n");
    else{
        printf("Voters:\n");
        printVoters(s->voters);
    }
    printf("DONE\n");
}


/**
 * Event K - prints party information
 * @param pid: PID of desired party
 */
void EventPrintParty(int pid) {
    printf("K %d\n", pid);

    if(pid<0 || pid>4){
        printf("Not found!\n");
        return;
    }

    printf("Elected: %d\n", Parties[pid].electedCount);
    printCandidates(Parties[pid].candidates);

    printf("DONE\n");

}

/**
 * Event L - prints parliament members
 */
void EventPrintParliament(void) {
    printf("L\n");

    ElectedCandidate* p = Parliament;

    if(p==NULL){
        printf("Parliament has no members yet!\n");
        return;
    }
    int i =0;
    while(p!=NULL){
        i++;
        printf((p->next==NULL)?("<%d> <%d> <%d>\n") : ("<%d> <%d> <%d>,\n"), p->cid, p->pid, p->did);
        p=p->next;
    }

    printf("DONE\n");
}

void EventBonusUnregisterVoter(int vid, int sid) {
    printf("BU %d %d\n", vid, sid);
    // TODO
}

void EventBonusFreeMemory(void) {
    printf("BF\n");
    //free hash table lists
    for(int i=0;i<size;i++){
        Station* head = StationsHT[i];
        while(head!=NULL){
            Station* tmp = head->next;
            freeTreeVoters(&head->voters); //free voters tree
            head->voters = NULL;
            free(head);
            head = tmp;
        }
        head = NULL;
    }
    //free parties candidate tree
    for(int i=0;i<PARTIES_SZ;i++){
        freeTreeCandidates(&Parties[i].candidates);
        Parties[i].candidates = NULL;
    }

    //free parliament list
    ElectedCandidate* p = Parliament;

    while(p!=NULL){
        ElectedCandidate* tmp = p->next;
        free(p);
        p=tmp;
    }
    Parliament=NULL;
}

//EVENT FUNCTIONS ---- END
