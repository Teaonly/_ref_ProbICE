#include <iostream>
#include "PthTest.h"

PthTest::PthTest() {
    worker_thread_ = new talk_base::Thread();
    worker_thread_->Start();


    signal_thread_ = new talk_base::Thread();
    signal_thread_->Start();

    signal_thread_->Post(this, 1);
}

PthTest::~PthTest() {

}

void PthTest::OnMessage(talk_base::Message *msg) { 
    switch (msg->message_id) {
        case 1: {
            for(;;) {
                talk_base::MessageData msgdata;
                worker_thread_->Send(this, 2, &msgdata);
                worker_thread_->Post(this, 3);
            }
        } break;
 
        case 2: {
            std::cout << "message 2" << std::endl;
        } break;

        case 3: {
            usleep(1000*20);
            worker_thread_->Clear(this, talk_base::MQID_ANY, NULL);
        } break;

    }
   
}

