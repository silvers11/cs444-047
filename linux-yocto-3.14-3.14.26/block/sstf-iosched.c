  	/*
    * elevator short_search
    */
   #include <linux/blkdev.h>
   #include <linux/elevator.h>
   #include <linux/bio.h>
   #include <linux/module.h>
   #include <linux/slab.h>
   #include <linux/init.h>
  
  struct short_search_data {
          struct list_head queue;t
          int forwards;
          sector_t position;

  };
  
  static void short_search_merged_requests(struct request_queue *q, struct request *rq,
                                   struct request *next)
  {
          list_del_init(&next->queuelist);
  }
  
static int short_search_dispatch(struct request_queue *q, int force){
    struct short_search_data *nd = q->elevator->elevator_data;
    struct request *rq;
  
    rq = list_first_entry_or_null(&nd->queue, struct request, queuelist);
          
    //check if request queue is empty
    if(!list_empty(&nd->queue)){

        struct request *my_request, *next_request, *previous_request;

        //assign values to previous and next requests
        previous_request = list_entry(nd->queue.prev, struct request, queuelist);
        next_request = list_entry(nd->queue.next, struct request, queuelist);

        //check if there is more than one request
        if(previous_request == next_request){
          	my_request = next_request;
        }
        //there is more than one request
        else{

       		//check direction 
        	if(nd->forwards == 1){

          		//if next request is further in same direction
          		if(next_request->__sector > nd->position){

          			//move in that direction
          			my_request = next_request;
          		}
          		//next request must be in opposite direction, so change direction
          		else{
          			nd->forwards = 0;
          			my_request = previous_request;
          		}

       		}
          	//same thing except reversing checks to account for already going backwards
          	else{

          		//check if next request is in same direction
          		if(previous_request->__sector < nd->position){

          			//move in that direction
          			my_request = previous_request;
          		}
          		//else the next request must be in forward direction, change direction
          		else{
          			nd->forwards = 1;
          			my_request = next_request;
          		}
          	}
        }

        //get new position for the disk head
        nd->position = blk_req_pos(my_request) + blk_req_sectors(my_request);

        //remove request to be dispatched from queuelist
        list_del_init(&my_request, queuelist);

        //sort request into dispatch queue
        elv_dispatch_sort(q,my_request);
        printk("request sorted.\n");
        return 1;
    }

return 0;
}
  
  static void short_search_add_request(struct request_queue *q, struct request *rq)
  {
          struct short_search_data *nd = q->elevator->elevator_data;
  
          list_add_tail(&rq->queuelist, &nd->queue);
  }
  
  static struct request *
  short_search_former_request(struct request_queue *q, struct request *rq)
  {
          struct short_search_data *nd = q->elevator->elevator_data;
  
          if (rq->queuelist.prev == &nd->queue)
                  return NULL;
          return list_prev_entry(rq, queuelist);
  }
  
  static struct request *
  short_search_latter_request(struct request_queue *q, struct request *rq)
  {
          struct short_search_data *nd = q->elevator->elevator_data;
  
          if (rq->queuelist.next == &nd->queue)
                  return NULL;
          return list_next_entry(rq, queuelist);
  }
  
  static int short_search_init_queue(struct request_queue *q, struct elevator_type *e)
  {
          struct short_search_data *nd;
          struct elevator_queue *eq;
  
          eq = elevator_alloc(q, e);
          if (!eq)
                  return -ENOMEM;
  
          nd = kmalloc_node(sizeof(*nd), GFP_KERNEL, q->node);
          if (!nd) {
                  kobject_put(&eq->kobj);
                  return -ENOMEM;
          }
          eq->elevator_data = nd;
  
          INIT_LIST_HEAD(&nd->queue);
  
          spin_lock_irq(q->queue_lock);
          q->elevator = eq;
          spin_unlock_irq(q->queue_lock);
          return ;
  }
  
  static void short_search_exit_queue(struct elevator_queue *e)
  {
          struct short_search_data *nd = e->elevator_data;
  
          BUG_ON(!list_empty(&nd->queue));
          kfree(nd);
  }
  
  static struct elevator_type elevator_short_search = {
          .ops = {
                  .elevator_merge_req_fn          = short_search_merged_requests,
                  .elevator_dispatch_fn           = short_search_dispatch,
                  .elevator_add_req_fn            = short_search_add_request,
                  .elevator_former_req_fn         = short_search_former_request,
                 .elevator_latter_req_fn         = short_search_latter_request,
                 .elevator_init_fn               = short_search_init_queue,
                 .elevator_exit_fn               = short_search_exit_queue,
         },
         .elevator_name = "short_search",
         .elevator_owner = THIS_MODULE,
 };
 
 static int __init short_search_init(void)
 {
         return elv_register(&elevator_short_search);
 }
 
 static void __exit short_search_exit(void)
 {
         elv_unregister(&elevator_short_search);
 }
 
 module_init(short_search_init);
 module_exit(short_search_exit);
 
 
MODULE_AUTHOR("Jacob Smith & Steven Silvers");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("SSTF IO scheduler");
