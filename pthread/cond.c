pthread_mutex_t count_lock;
pthread_cond_t count_nonzero;
unsigned count;

decrement_count()
{
    pthread_mutex_lock(&count_lock);
    while (count == 0)
        pthread_cond_wait(&count_nonzero, &count_lock);
    count = count - 1;
    pthread_mutex_unlock(&count_lock);
}

increment_count()
{
    pthread_mutex_lock(&count_lock);
    if (count == 0)
        pthread_cond_signal(&count_nonzero);
    count = count + 1;
    pthread_mutex_unlock(&count_lock);
}


// Another use case

pthread_mutex_t rsrc_lock;
pthread_cond_t rsrc_add;
unsigned int resources;

get_resources(int amount)
{
    pthread_mutex_lock(&rsrc_lock);
    while (resources < amount) {
        pthread_cond_wait(&rsrc_add, &rsrc_lock);
    }
    resources -= amount;
    pthread_mutex_unlock(&rsrc_lock);
}

add_resources(int amount)
{
    pthread_mutex_lock(&rsrc_lock);
    resources += amount;
    pthread_cond_broadcast(&rsrc_add);
    pthread_mutex_unlock(&rsrc_lock);
}


// example where _queue has MAX elememts possible.
// if there is no limit MAX then the enque does not need the while loop
// and deque does not need to signal rsrc_enque
enque(node *n)
{

    pthread_mutex_lock(&rsrc_lock);
    while ( MAX == _queue.size()) {
        pthread_cond_wait(&rsrc_enque, &rsrc_lock);
    }
    _queue.push(n);
    pthread_cond_broadcast(&rsrc_deque);
    pthread_mutex_unlock(&rsrc_lock);

}

//
deque(node *n)
{

    pthread_mutex_lock(&rsrc_lock);
    while ( 0 == _queue.size()) {
        pthread_cond_wait(&rsrc_deque, &rsrc_lock);
    }
    _queue.pop(n);
    pthread_cond_broadcast(&rsrc_enque);
    pthread_mutex_unlock(&rsrc_lock);
}
