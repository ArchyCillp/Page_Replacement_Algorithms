/**
 * Page Replacement Algorithms 
 * 
 * 0-FIFO, 1-LRU, 2-MIN, 3-Clock, 4-Second-chance
 * Input format: 
 *  First line for K(cache size), A(algorithm index), N(query number) 
 *  N lines for query page.
 * Ouput hit rate.
 * 
 * Author: Jiaping Cao <11610809@mail.sustech.edu.cn>
 * 
 * If you want to run the not optimized version, please define NOOPT 
 * in compilation. 
 * */
#include <bits/stdc++.h>
#define RELEASE
using namespace std;
const static int MAXN = 1000005;
const static int INF = 0x3f3f3f3f;
static int K, A, N;
static int* page;

struct Node
{
    bool Valid;
    int Page_num; // Only for Cycle_list
    int Next_time; // Only for min
    Node* Next;
    Node* Pre; // Only for List
    Node (int page_num, int next_time = INF, bool valid=false)
        :  Valid(valid), Page_num(page_num), Next_time(next_time), Next(NULL), Pre(NULL) {}
};

struct List
{
    Node* head;
    Node* tail;
    int size;
    List() : head(NULL), tail(NULL), size(0) {};

    void debug_out()
    {
        Node* cur = head;
        fprintf(stderr,"{");
        while(cur)
        {
            fprintf(stderr,"%d,",cur->Page_num);
            cur=cur->Next;
        }
        fprintf(stderr,"}\n");
    }

    Node* find_page_num(int target_page)
    {
        Node* cur = head;
        while (cur != NULL)
        {
            if (cur->Page_num == target_page)
            {
                return cur;
            }
            else
            {
                cur = cur->Next;
            }
        }
        return NULL;
    }

    int pop_head()
    {
        int page_num = -1;
        if (head)
        {
            page_num = head->Page_num;
            head = head->Next;
            if (head)
            {
                head->Pre = NULL;
            }
            else
            {
                tail = NULL;
            }
            --size;
        }
        return page_num;
    }

    int pop_tail()
    {
        int page_num = -1;
        if (tail)
        {
            page_num = tail->Page_num;
            tail = tail->Pre;
            if (tail)
            {
                tail->Next = NULL;
            }
            else
            {
                head = NULL;
            }
            --size;
        }
        return page_num;
    }

    void push_back(Node* page)
    {
        if (!size)
        {
            head = tail = page;
            size = 1;
        }
        else
        {
            page->Pre = tail;
            tail->Next = page;
            tail = page;
            ++size;
        }
    }

    void push_front(Node* page)
    {
        if (!size)
        {
            head = tail = page;
            size = 1;
        }
        else
        {
            page->Next = head;
            head->Pre = page;
            head = page;
            ++size;
        }
    }

    bool erase(Node* target) 
    {
        if (target)
        {
            if (target == head)
            {
                pop_head();
            }
            else if (target == tail)
            {
                pop_tail();
            }
            else
            {
                target->Pre->Next = target->Next;
                target->Next->Pre = target->Pre;
                --size;
            }
            return true;
        }
        else
        {
            return false;
        }
    }

    bool find_and_delete(int page_num)
    {
        Node* target = find_page_num(page_num);
        return erase(target);
    }
};

struct Cycle_list
{
    Node *head;
    int m_size;
    Cycle_list (int size) : m_size(size)
    {
        head = new Node(-1, false);
        Node* tail = head;
        for (int i=1; i<size; i++)
        {
            Node* new_one = new Node(-1, false);
            tail->Next = new_one;
            tail = new_one;
        }
        tail->Next = head;
    }

    bool check_exist(int target_page)
    {
        Node* cur=head;
        for (int cnt=0; cnt<m_size; ++cnt)
        {
            if (cur->Page_num == target_page)
            {
                cur->Valid = true;
                return true;
            }
            cur=cur->Next;
        }
        return false;
    }

    void insert_page(int target_page)
    {
        Node* cur = head;
        while(true)
        {
            if (!cur->Valid)
            {
                cur->Page_num = target_page;
                cur->Valid = true;
                head = cur->Next;
                return;
            }
            else
            {
                cur->Valid = false;
                cur = cur->Next;
            }
        }
    }
};

double fifo_raw()
{
    int hit_num = 0;
    List fifo_list;
    for (int i=0; i<N; i++)
    {
        Node* target = fifo_list.find_page_num(page[i]);
        if (!target)
        {
            Node* target = new Node(page[i]);
            if (fifo_list.size < K)
            {
                fifo_list.push_back(target);
            }
            else
            {
                fifo_list.pop_head();
                fifo_list.push_back(target);
            }
        }
        else
        {
            ++hit_num;
        }
    }
    return 1.0*hit_num/N;
}

double fifo_optimize()
{
    int* my_queue = new int[N];
    unordered_set<int> exist_set;
    int hit_num = 0;
    int cur_size = 0;
    int head = 0;
    int tail = 0;
    for (int i=0; i<N; i++)
    {
        auto it = exist_set.find(page[i]);
        if (it == exist_set.end())
        {
            if (cur_size < K)
            {
                my_queue[tail++] = page[i];
                exist_set.insert(page[i]);
                ++cur_size;
            }
            else
            {
                int head_page = my_queue[head++];
                exist_set.erase(head_page);
                exist_set.insert(page[i]);
                my_queue[tail++] = page[i];
            }
        }
        else
        {
            ++hit_num;
        }
    }
    return 1.0*hit_num/N;
}

double min_optimize()
{
    typedef pair<int,int> PII;
    PII* page_time = new PII[N];
    PII* nexttime_page = new PII[N];
    for (int i=0; i<N; i++)
    {
        page_time[i] = make_pair(page[i], i);
    }
    sort(page_time, page_time+N);
    for (int i=0; i<N; i++)
    {
        if (i < N-1 && page_time[i+1].first == page_time[i].first)
        {
            nexttime_page[page_time[i].second] = make_pair(page_time[i+1].second, page_time[i].first);
        }
        else
        {
            nexttime_page[page_time[i].second] = make_pair(INF, page_time[i].first);
        }
    }

    int sz = 0;
    int hit_cnt = 0;
    set<PII,greater<PII> > check_set;
    for (int i=0; i<N; i++)
    {
        PII nowtime_page = make_pair(i, page[i]);
        auto it = check_set.find(nowtime_page);
        if (it == check_set.end())
        {
            if (sz < K)
            {
                check_set.insert(nexttime_page[i]);
                ++sz;
            }
            else
            {
                check_set.erase(check_set.begin());
                check_set.insert(nexttime_page[i]);
            }
        }
        else
        {
            check_set.erase(it);
            check_set.insert(nexttime_page[i]);
            ++hit_cnt;
        }
    }
    return 1.0*hit_cnt/N;
}

double LRU_raw()
{
    int hit_cnt = 0;
    List lru_list;
    for (int i=0; i<N; i++)
    {
        bool found = lru_list.find_and_delete(page[i]);
        Node* target = new Node(page[i]);
        lru_list.push_front(target);
        if (lru_list.size > K)
        {
            lru_list.pop_tail();
        }
        if (found)
        {
            ++hit_cnt;
        }
    }
    return 1.0*hit_cnt/N;
}

double LRU_optimize()
{
    unordered_map<int, Node*> page_node_table;
    int hit_cnt = 0;
    List lru_list;
    for (int i=0; i<N; i++)
    {
        auto found = page_node_table.find(page[i]);
        Node * new_node = new Node(page[i]);
        if (found == page_node_table.end()) 
        {
            lru_list.push_front(new_node);
            page_node_table.insert({page[i], new_node});
            if (lru_list.size > K) 
            {
                int out_page = lru_list.pop_tail();
                page_node_table.erase(out_page);
            }
        }
        else
        {
            lru_list.erase(found->second);
            page_node_table.erase(found);
            page_node_table.insert({page[i], new_node});
            lru_list.push_front(new_node);
            ++hit_cnt;
        }
    }
    return 1.0*hit_cnt/N;
}

double clock_raw()
{
    Cycle_list clock_list(K);
    int hit_cnt = 0;
    for (int i=0; i<N; i++)
    {
        if (clock_list.check_exist(page[i]))
        {
            ++hit_cnt;
        }
        else
        {
            clock_list.insert_page(page[i]);
        }
    }
    return 1.0*hit_cnt/N;
}

double second_chance_raw()
{
    int K1 = K/2;
    int K2 = K-K1;
    int hit_cnt = 0;
    List fifo_list;
    List lru_list;
    for (int i=0; i<N; i++)
    {
        assert(fifo_list.size <= K1 && lru_list.size <= K2);
        Node* target = fifo_list.find_page_num(page[i]);
        if (target)
        {
            ++hit_cnt;
        }
        else {
            bool found = lru_list.find_and_delete(page[i]);
            Node* new_node = new Node(page[i]);
            if (found)
            {
                fifo_list.push_back(new_node);;
                lru_list.push_front(new Node(fifo_list.pop_head()));
                ++hit_cnt;
            }
            else
            {
                if (fifo_list.size < K1)
                {
                    fifo_list.push_back(new_node);
                }
                else
                {
                    if (lru_list.size >= K2)
                    {
                        lru_list.pop_tail();
                    }
                    fifo_list.push_back(new_node);;
                    lru_list.push_front(new Node(fifo_list.pop_head()));
                }
            }
        }
    }
    return 1.0*hit_cnt/N;
}

int main()
{
    scanf("%d%d%d", &K, &A, &N);
    page = new int[N+5];
    for (int i=0; i<N; i++)
    {
        scanf("%d", &page[i]);
    }
    double ans = 0;
    #ifndef NOOPT
    switch (A)
    {
    case 0:
        ans = fifo_optimize();
        break;
    case 1:
        ans = LRU_optimize();
        break;
    case 2:
        ans = min_optimize();
        break;
    case 3:
        ans = clock_raw();
        break;
    case 4:
        ans = second_chance_raw();
        break;
    default:
        assert(false);
        break;
    }
    #else
    switch (A)
    {
    case 0:
        ans = fifo_raw();
        break;
    case 1:
        ans = LRU_raw();
        break;
    case 2:
        ans = min_optimize();
        break;
    case 3:
        ans = clock_raw();
        break;
    case 4:
        ans = second_chance_raw();
        break;
    default:
        assert(false);
        break;
    }
    #endif // NOOPT
    printf("Hit ratio = %.2f%%\n",ans*100.0);
    return 0;
}
