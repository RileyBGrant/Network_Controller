using namespace std;

//defines structures and a pointer to the structures
typedef struct node node_t; 
typedef struct linkedList linkedList_t;

//holds the information for each node in the list
struct node
{
    void *data;
    node_t *next;
};

//class for the actual list
class linkedList
{
private:
    node_t *head;
    node_t *tail;
    int length;
public:
    linkedList();
    int append(void* data);
    int insert(int index, void *data);
    int remove(int index);
    node_t *getHead();
    node_t *getNext(node_t * currentN);
    node_t *getTail();
    int getLen();
    ~linkedList();
};

